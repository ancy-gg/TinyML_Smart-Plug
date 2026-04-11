import argparse
from concurrent.futures import ThreadPoolExecutor
import itertools
import json
import os
import math
import time
from pathlib import Path

import numpy as np
import pandas as pd
from sklearn.base import clone
from sklearn.model_selection import StratifiedShuffleSplit

from tinyml_common import (
    ARC_CONTEXT_FEATURES,
    ARC_SWEEP_FEATURES,
    CONTEXT_SWEEP_FEATURES,
    DEVICE_FAMILY_CLASSES,
    MODEL_CONFIGS,
    ensure_dir,
    evaluate_binary_scores,
    load_clean_dataset,
    make_group_splits,
    model_size_estimate,
    select_threshold_cost,
    train_one_model,
    estimate_search_plan,
)
from train_context import build_context_frame, prototype_predict_proba, weighted_standardize

SCRIPT_DIR = Path(__file__).resolve().parent
TINYML_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TINYML_DIR.parent

ARC_CSV_PATH = str(PROJECT_ROOT / "tinyml" / "data" / "arc_training.csv")
CONTEXT_CSV_PATH = str(PROJECT_ROOT / "tinyml" / "data" / "load_context.csv")
ARC_OUT_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLFeatureSubsetSweep_report.json")
ARC_OUT_CSV = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLFeatureSubsetSweep_results.csv")
CONTEXT_OUT_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLContextFeatureSubsetSweep_report.json")
CONTEXT_OUT_CSV = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLContextFeatureSubsetSweep_results.csv")


def emit_progress(current: int, total: int, payload: dict) -> None:
    payload = dict(payload or {})
    payload["current"] = int(current)
    payload["total"] = int(total)
    print("[[PROGRESS]] " + json.dumps(payload, sort_keys=True), flush=True)



def _count_combinations(pool_size: int, count_min: int, count_max: int, limit: int = 0) -> int:
    if pool_size <= 0:
        return 0
    lo = max(1, int(count_min))
    hi = min(int(pool_size), int(count_max))
    if hi < lo:
        hi = lo
    total = 0
    for k in range(lo, hi + 1):
        total += math.comb(int(pool_size), int(k))
        if limit and limit > 0 and total >= int(limit):
            return int(limit)
    return int(total)


def _make_progress_tracker(args) -> dict:
    arc_task_enabled = args.task in ("auto", "arc")
    context_task_enabled = args.task in ("auto", "context")

    arc_pool = [str(x).strip() for x in (args.features or ARC_SWEEP_FEATURES) if str(x).strip() in ARC_SWEEP_FEATURES]
    ctx_pool = [str(x).strip() for x in (args.context_features or CONTEXT_SWEEP_FEATURES) if str(x).strip() in CONTEXT_SWEEP_FEATURES]

    arc_feature_min = max(1, int(args.feature_count_min))
    arc_feature_max = min(len(arc_pool), int(args.feature_count_max or len(arc_pool))) if arc_pool else 0
    if arc_feature_max < arc_feature_min and arc_pool:
        arc_feature_max = arc_feature_min

    ctx_feature_min = max(1, int(args.context_feature_count_min or args.feature_count_min))
    ctx_feature_max = min(len(ctx_pool), int(args.context_feature_count_max or len(ctx_pool))) if ctx_pool else 0
    if ctx_feature_max < ctx_feature_min and ctx_pool:
        ctx_feature_max = ctx_feature_min

    arc_combo_count = _count_combinations(len(arc_pool), arc_feature_min, arc_feature_max, int(args.max_combinations)) if arc_task_enabled else 0
    ctx_combo_count = _count_combinations(len(ctx_pool), ctx_feature_min, ctx_feature_max, int(args.context_max_combinations)) if context_task_enabled else 0

    arc_shard_size = max(1, int(getattr(args, "arc_shard_size", 128)))
    arc_keep_per_shard = max(1, int(getattr(args, "arc_keep_per_shard", 1)))
    arc_shortlist_size = max(1, int(getattr(args, "arc_shortlist_size", 18)))
    arc_finalist_count = max(1, int(getattr(args, "arc_finalist_count", 6)))
    arc_shortlist_n_iter = max(1, int(getattr(args, "arc_shortlist_n_iter", max(1, int(round(int(args.n_iter) * 0.25))))))
    arc_shortlist_plan = estimate_search_plan(int(arc_shortlist_n_iter))
    arc_final_plan = estimate_search_plan(int(args.n_iter))
    arc_shard_count = int(math.ceil(arc_combo_count / arc_shard_size)) if arc_combo_count > 0 else 0
    arc_prescreen_survivors = min(int(arc_combo_count), max(1, arc_shard_count * arc_keep_per_shard)) if arc_task_enabled else 0
    arc_shortlist_count = min(int(arc_prescreen_survivors), int(arc_shortlist_size)) if arc_task_enabled else 0
    arc_final_count = min(int(arc_shortlist_count), int(arc_finalist_count)) if arc_task_enabled else 0
    arc_prescreen_steps = int(arc_combo_count)
    arc_shortlist_steps = int(arc_shortlist_count) * int(arc_shortlist_plan.get("progress_steps", 1))
    arc_final_steps = int(arc_final_count) * ((2 * int(arc_final_plan.get("progress_steps", 1))) + 1)
    context_repeat_steps = max(1, int(args.context_repeats))
    context_steps_per_combo = context_repeat_steps + 2 if context_task_enabled else 0

    total_steps = 1  # global kickoff
    if arc_task_enabled:
        total_steps += 5  # arc load + ready + staged transitions/finalization
        total_steps += int(arc_prescreen_steps + arc_shortlist_steps + arc_final_steps)
    if context_task_enabled:
        total_steps += 2  # context load + context ready
        total_steps += int(ctx_combo_count) * int(context_steps_per_combo)
    total_steps += 1  # final save/finalize

    return {
        "current": 0,
        "total": max(1, int(total_steps)),
        "arc_task_enabled": bool(arc_task_enabled),
        "context_task_enabled": bool(context_task_enabled),
        "arc_feature_pool": list(arc_pool),
        "context_feature_pool": list(ctx_pool),
        "arc_combo_count": int(arc_combo_count),
        "context_combo_count": int(ctx_combo_count),
        "arc_shard_count": int(arc_shard_count),
        "arc_prescreen_survivor_estimate": int(arc_prescreen_survivors),
        "arc_shortlist_count_estimate": int(arc_shortlist_count),
        "arc_final_count_estimate": int(arc_final_count),
        "arc_shortlist_steps": int(arc_shortlist_plan.get("progress_steps", 1)),
        "arc_final_model_steps": int(arc_final_plan.get("progress_steps", 1)),
        "context_repeat_steps": int(context_repeat_steps),
        "context_steps_per_combo": int(context_steps_per_combo),
        "arc_shortlist_plan": arc_shortlist_plan,
        "arc_final_plan": arc_final_plan,
    }


def _progress_advance(tracker: dict | None, steps: int = 1, payload: dict | None = None) -> None:
    if tracker is None:
        return
    tracker["current"] = min(int(tracker.get("total", 1)), int(tracker.get("current", 0)) + max(0, int(steps)))
    emit_progress(tracker["current"], tracker["total"], payload or {})


def _progress_set(tracker: dict | None, current: int, payload: dict | None = None) -> None:
    if tracker is None:
        return
    tracker["current"] = max(int(tracker.get("current", 0)), min(int(tracker.get("total", 1)), int(current)))
    emit_progress(tracker["current"], tracker["total"], payload or {})


def _progress_child_callback(tracker: dict | None, base_current: int, local_offset: int, local_total_target: int, task: str, combo_index: int, combo_total: int, feature_combo: str):
    local_total_target = max(1, int(local_total_target))

    def _callback(current: int, total: int, payload: dict):
        if tracker is None:
            return
        try:
            child_current = int(current)
        except Exception:
            child_current = 0
        try:
            child_total = max(1, int(total))
        except Exception:
            child_total = local_total_target
        normalized = int(round((max(0, min(child_current, child_total)) / child_total) * local_total_target))
        global_current = int(base_current) + int(local_offset) + int(normalized)
        out = dict(payload or {})
        out.setdefault("task", task)
        out.setdefault("combo_index", int(combo_index))
        out.setdefault("combo_total", int(combo_total))
        out.setdefault("feature_combo", feature_combo)
        _progress_set(tracker, global_current, out)

    return _callback


# ----------------------------
# Arc sweep helpers
# ----------------------------

def _row_int(row: dict, *keys: str, default: int = 0) -> int:
    for key in keys:
        if key in row and row.get(key) is not None:
            try:
                return int(row.get(key))
            except Exception:
                pass
    return int(default)


def _row_float(row: dict, *keys: str, default: float = 0.0) -> float:
    for key in keys:
        if key in row and row.get(key) is not None:
            try:
                return float(row.get(key))
            except Exception:
                pass
    return float(default)


def _row_bool(row: dict, *keys: str, default: bool = False) -> bool:
    for key in keys:
        if key in row and row.get(key) is not None:
            try:
                return bool(row.get(key))
            except Exception:
                pass
    return bool(default)


def _validation_accuracy(tn: int, fp: int, fn: int, tp: int) -> float:
    total = max(1, int(tn) + int(fp) + int(fn) + int(tp))
    return float((int(tn) + int(tp)) / total)


def _sort_model_row_fn_first(row: dict):
    return (
        _row_int(row, "validation_fn", default=10**9),
        _row_int(row, "validation_fp", default=10**9),
        0 if _row_int(row, "validation_fn", default=10**9) == 0 else 1,
        -_row_float(row, "validation_recall", default=0.0),
        -_row_float(row, "validation_precision", default=0.0),
        _row_float(row, "validation_fpr", default=1.0),
        -_row_float(row, "test_accuracy", default=0.0),
        -_row_float(row, "test_average_precision", default=0.0),
        _row_int(row, "feature_count", default=10**9),
        str(row.get("feature_combo", "")),
    )


def _sort_combined_row_fn_first(row: dict):
    return (
        _row_int(row, "combined_validation_fn", default=10**9),
        _row_int(row, "combined_validation_fp", default=10**9),
        0 if _row_int(row, "combined_validation_fn", default=10**9) == 0 else 1,
        -_row_float(row, "combined_validation_recall_mean", default=0.0),
        -_row_float(row, "combined_validation_precision_mean", default=0.0),
        _row_float(row, "combined_validation_fpr_mean", default=1.0),
        _row_int(row, "combined_test_fn", default=10**9),
        _row_int(row, "combined_test_fp", default=10**9),
        -_row_float(row, "combined_test_accuracy_mean", default=0.0),
        -_row_float(row, "combined_test_average_precision_mean", default=0.0),
        _row_int(row, "feature_count", default=10**9),
        str(row.get("feature_combo", "")),
    )


def _sort_model_row_validation_cost(row: dict):
    return (
        0 if _row_bool(row, "threshold_constraints_met", default=False) else 1,
        _row_float(row, "validation_cost", default=float("inf")),
        _row_int(row, "validation_fp", default=10**9),
        _row_int(row, "validation_fn", default=10**9),
        -_row_float(row, "validation_balanced_accuracy", default=0.0),
        -_row_float(row, "validation_accuracy", default=0.0),
        -_row_float(row, "test_balanced_accuracy", default=0.0),
        -_row_float(row, "test_accuracy", default=0.0),
        _row_int(row, "feature_count", default=10**9),
        str(row.get("feature_combo", "")),
    )


def _sort_combined_row_validation_cost(row: dict):
    return (
        -_row_int(row, "combined_validation_constraints_met_count", default=0),
        _row_float(row, "combined_validation_cost", default=float("inf")),
        _row_int(row, "combined_validation_fp", default=10**9),
        _row_int(row, "combined_validation_fn", default=10**9),
        -_row_float(row, "combined_validation_balanced_accuracy_mean", default=0.0),
        -_row_float(row, "combined_validation_accuracy_mean", default=0.0),
        -_row_float(row, "combined_test_balanced_accuracy_mean", default=0.0),
        -_row_float(row, "combined_test_accuracy_mean", default=0.0),
        _row_int(row, "feature_count", default=10**9),
        str(row.get("feature_combo", "")),
    )


def _sort_model_row_accuracy(row: dict):
    return (
        -_row_float(row, "validation_balanced_accuracy", default=0.0),
        -_row_float(row, "validation_accuracy", default=0.0),
        -_row_float(row, "test_accuracy", default=0.0),
        -_row_float(row, "test_balanced_accuracy", default=0.0),
        _row_float(row, "validation_cost", default=float("inf")),
        _row_int(row, "validation_fn", default=10**9),
        _row_int(row, "validation_fp", default=10**9),
        _row_int(row, "feature_count", default=10**9),
        str(row.get("feature_combo", "")),
    )


def _sort_combined_row_accuracy(row: dict):
    return (
        -_row_float(row, "combined_validation_balanced_accuracy_mean", default=0.0),
        -_row_float(row, "combined_validation_accuracy_mean", default=0.0),
        -_row_float(row, "combined_test_accuracy_mean", default=0.0),
        -_row_float(row, "combined_test_balanced_accuracy_mean", default=0.0),
        _row_float(row, "combined_validation_cost", default=float("inf")),
        _row_int(row, "combined_validation_fn", default=10**9),
        _row_int(row, "combined_validation_fp", default=10**9),
        _row_int(row, "feature_count", default=10**9),
        str(row.get("feature_combo", "")),
    )


def _sort_model_row_practical(row: dict):
    return (
        0 if _row_bool(row, "threshold_constraints_met", default=False) else 1,
        -_row_float(row, "validation_balanced_accuracy", default=0.0),
        -_row_float(row, "validation_accuracy", default=0.0),
        _row_float(row, "validation_fpr", default=1.0),
        _row_int(row, "validation_fp", default=10**9),
        _row_int(row, "validation_fn", default=10**9),
        -_row_float(row, "validation_precision", default=0.0),
        -_row_float(row, "validation_recall", default=0.0),
        -_row_float(row, "test_balanced_accuracy", default=0.0),
        -_row_float(row, "test_accuracy", default=0.0),
        _row_int(row, "feature_count", default=10**9),
        str(row.get("feature_combo", "")),
    )


def _sort_combined_row_practical(row: dict):
    return (
        -_row_int(row, "combined_validation_constraints_met_count", default=0),
        -_row_float(row, "combined_validation_balanced_accuracy_mean", default=0.0),
        -_row_float(row, "combined_validation_accuracy_mean", default=0.0),
        _row_float(row, "combined_validation_fpr_mean", default=1.0),
        _row_int(row, "combined_validation_fp", default=10**9),
        _row_int(row, "combined_validation_fn", default=10**9),
        -_row_float(row, "combined_validation_precision_mean", default=0.0),
        -_row_float(row, "combined_validation_recall_mean", default=0.0),
        -_row_float(row, "combined_test_balanced_accuracy_mean", default=0.0),
        -_row_float(row, "combined_test_accuracy_mean", default=0.0),
        _row_int(row, "feature_count", default=10**9),
        str(row.get("feature_combo", "")),
    )


# Default ranking now follows the practical validation tradeoff:
# prefer low FPR/FP and strong balanced accuracy without sacrificing recall.
def _sort_model_row(row: dict):
    return _sort_model_row_practical(row)


def _sort_combined_row(row: dict):
    return _sort_combined_row_practical(row)


def _arc_row_int(row: dict, *keys: str, default: int = 0) -> int:
    return _row_int(row, *keys, default=default)


def _arc_row_float(row: dict, *keys: str, default: float = 0.0) -> float:
    return _row_float(row, *keys, default=default)


def _sort_arc_candidate_row(row: dict):
    val_constraints = _arc_row_int(
        row,
        "combined_validation_constraints_met_count",
        default=1 if _row_bool(row, "threshold_constraints_met", default=False) else 0,
    )
    return (
        -val_constraints,
        -_arc_row_float(row, "validation_balanced_accuracy", "combined_validation_balanced_accuracy_mean", default=0.0),
        -_arc_row_float(row, "validation_accuracy", "combined_validation_accuracy_mean", default=0.0),
        _arc_row_float(row, "validation_fpr", "combined_validation_fpr_mean", default=1.0),
        _arc_row_int(row, "validation_fp", "combined_validation_fp", default=10**9),
        _arc_row_int(row, "validation_fn", "combined_validation_fn", default=10**9),
        -_arc_row_float(row, "validation_precision", "combined_validation_precision_mean", default=0.0),
        -_arc_row_float(row, "validation_recall", "combined_validation_recall_mean", default=0.0),
        -_arc_row_float(row, "test_balanced_accuracy", "combined_test_balanced_accuracy_mean", default=0.0),
        -_arc_row_float(row, "test_accuracy", "combined_test_accuracy_mean", default=0.0),
        _arc_row_int(row, "feature_count", default=10**9),
        str(row.get("feature_combo", "")),
    )


def _arc_feature_combinations(feature_pool: list[str], feature_count_min: int, feature_count_max: int, max_combinations: int) -> list[list[str]]:
    combos = []
    combo_counter = 0
    for feature_count in range(feature_count_min, feature_count_max + 1):
        for combo in itertools.combinations(feature_pool, feature_count):
            combo_counter += 1
            if max_combinations and max_combinations > 0 and combo_counter > int(max_combinations):
                return combos
            combos.append(list(combo))
    return combos


def _chunk_arc_combinations(combos: list[list[str]], shard_size: int) -> list[list[list[str]]]:
    shard_size = max(1, int(shard_size))
    return [combos[i:i + shard_size] for i in range(0, len(combos), shard_size)]


def _select_diverse_arc_rows(rows: list[dict], keep_total: int, per_feature_count: int = 1) -> list[dict]:
    keep_total = max(0, int(keep_total))
    if keep_total <= 0 or not rows:
        return []

    ordered = sorted(rows, key=_sort_arc_candidate_row)
    selected = []
    seen = set()
    per_count = {}
    limit_per_count = max(0, int(per_feature_count))

    if limit_per_count > 0:
        for row in ordered:
            combo_key = str(row.get("feature_combo", ""))
            feature_count = int(row.get("feature_count", 0))
            if combo_key in seen or per_count.get(feature_count, 0) >= limit_per_count:
                continue
            selected.append(row)
            seen.add(combo_key)
            per_count[feature_count] = per_count.get(feature_count, 0) + 1
            if len(selected) >= keep_total:
                return selected

    for row in ordered:
        combo_key = str(row.get("feature_combo", ""))
        if combo_key in seen:
            continue
        selected.append(row)
        seen.add(combo_key)
        if len(selected) >= keep_total:
            break
    return selected


def _arc_budget_state(started_at: float, budget_seconds: float) -> tuple[bool, float, float | None]:
    elapsed = max(0.0, time.monotonic() - float(started_at))
    if float(budget_seconds) <= 0.0:
        return False, elapsed, None
    remaining = max(0.0, float(budget_seconds) - elapsed)
    return remaining <= 0.0, elapsed, remaining


def _failed_arc_model_result(model_key: str, model_name: str, y_val, y_test, min_threshold: float, error_text: str, best_params: dict | None = None, strategy: str = "failed_arc_screen") -> dict:
    y_val_np = np.asarray(y_val).astype(int)
    y_test_np = np.asarray(y_test).astype(int)
    val_pos = int(np.sum(y_val_np == 1))
    val_neg = int(np.sum(y_val_np == 0))
    test_pos = int(np.sum(y_test_np == 1))
    test_neg = int(np.sum(y_test_np == 0))
    threshold_result = {
        "thr": float(min_threshold),
        "selection_reason": "error_fallback",
        "constraints_met": False,
        "cost": float((val_pos * 1_000_000) + val_neg),
        "tn": 0,
        "fp": int(val_neg),
        "fn": int(val_pos),
        "tp": 0,
    }
    return {
        "model_key": model_key,
        "model_name": model_name,
        "best_params": dict(best_params or {}),
        "search_summary": {
            "strategy": strategy,
            "error": str(error_text),
        },
        "estimated_node_count": 0,
        "threshold": float(min_threshold),
        "threshold_source": "error_fallback",
        "threshold_constraints_met": False,
        "validation_threshold_result": threshold_result,
        "validation_precision": 0.0,
        "holdout_validation_precision": 0.0,
        "holdout_validation_recall": 0.0,
        "validation_accuracy": 0.0,
        "validation_average_precision": 0.0,
        "validation_roc_auc": 0.5,
        "validation_recall": 0.0,
        "validation_f1": 0.0,
        "validation_balanced_accuracy": 0.0,
        "validation_specificity": 0.0,
        "validation_npv": 0.0,
        "validation_fpr": 1.0,
        "validation_fnr": 1.0,
        "validation_mcc": 0.0,
        "validation_confusion_matrix": {
            "tn": 0,
            "fp": int(val_neg),
            "fn": int(val_pos),
            "tp": 0,
        },
        "test_accuracy": 0.0,
        "test_average_precision": 0.0,
        "test_roc_auc": 0.5,
        "test_precision": 0.0,
        "test_recall": 0.0,
        "test_f1": 0.0,
        "test_balanced_accuracy": 0.0,
        "test_specificity": 0.0,
        "test_npv": 0.0,
        "test_fpr": 1.0,
        "test_fnr": 1.0,
        "test_mcc": 0.0,
        "test_confusion_matrix": {
            "tn": 0,
            "fp": int(test_neg),
            "fn": int(test_pos),
            "tp": 0,
        },
    }


def _safe_train_arc_model(
    model_key: str,
    X_train,
    y_train,
    groups_train,
    w_train,
    X_val,
    y_val,
    X_test,
    y_test,
    n_iter: int,
    args,
    progress_callback=None,
) -> dict:
    model_name = "RandomForest" if model_key == "rf" else "ExtraTrees"
    try:
        return train_one_model(
            model_key=model_key,
            X_train=X_train,
            y_train=y_train,
            groups_train=groups_train,
            w_train=w_train,
            X_val=X_val,
            y_val=y_val,
            X_test=X_test,
            y_test=y_test,
            fn_weight=args.fn_weight,
            fp_weight=args.fp_weight,
            min_recall=args.min_recall,
            n_iter=n_iter,
            min_precision=args.min_precision,
            max_fpr=args.max_fpr,
            min_threshold=args.min_threshold,
            progress_callback=progress_callback,
        )
    except Exception as exc:
        return _failed_arc_model_result(
            model_key=model_key,
            model_name=model_name,
            y_val=y_val,
            y_test=y_test,
            min_threshold=args.min_threshold,
            error_text=str(exc),
            strategy=f"{model_key}_arc_subset_failure",
        )


def _run_arc_et_prescreen(
    X_train,
    y_train,
    w_train,
    X_val,
    y_val,
    X_test,
    y_test,
    args,
) -> dict:
    params = {
        "n_estimators": max(48, int(getattr(args, "arc_prescreen_trees", 64))),
        "max_depth": max(4, int(getattr(args, "arc_prescreen_max_depth", 8))),
        "min_samples_leaf": 2,
        "min_samples_split": 4,
        "max_features": "sqrt",
        "bootstrap": False,
        "class_weight": "balanced",
        "criterion": "gini",
        "ccp_alpha": 0.0,
        "max_leaf_nodes": 64,
        "min_impurity_decrease": 0.0,
    }

    try:
        estimator = clone(MODEL_CONFIGS["et"]["builder"]())
        estimator.set_params(**params)
        estimator.fit(X_train, y_train, sample_weight=np.asarray(w_train).astype(float))
        y_val_np = np.asarray(y_val).astype(int)
        y_test_np = np.asarray(y_test).astype(int)
        val_proba = estimator.predict_proba(X_val)[:, 1]
        best_thr = select_threshold_cost(
            y_val_np,
            val_proba,
            fn_weight=args.fn_weight,
            fp_weight=args.fp_weight,
            min_recall=args.min_recall,
            min_precision=args.min_precision,
            max_fpr=args.max_fpr,
            min_threshold=args.min_threshold,
        )
        threshold = float(best_thr["thr"])
        val_metrics = evaluate_binary_scores(y_val_np, val_proba, threshold)
        test_proba = estimator.predict_proba(X_test)[:, 1]
        test_metrics = evaluate_binary_scores(y_test_np, test_proba, threshold)
        return {
            "model_key": "et",
            "model_name": "ExtraTrees",
            "best_params": dict(params),
            "search_summary": {
                "strategy": "single_pass_fast_et_prescreen",
                "plan": {
                    "broad_candidates": 1,
                    "refine_candidates": 0,
                    "robust_finalists": 0,
                    "robustness_seeds": [],
                    "progress_steps": 1,
                },
            },
            "estimated_node_count": int(model_size_estimate(estimator)),
            "threshold": threshold,
            "threshold_source": best_thr.get("selection_reason", "validation_cost"),
            "threshold_constraints_met": bool(best_thr.get("constraints_met", False)),
            "validation_threshold_result": best_thr,
            "validation_precision": float(val_metrics.get("precision", 0.0)),
            "holdout_validation_precision": float(val_metrics.get("precision", 0.0)),
            "holdout_validation_recall": float(val_metrics.get("recall", 0.0)),
            "validation_accuracy": float(val_metrics.get("accuracy", 0.0)),
            "validation_average_precision": float(val_metrics.get("average_precision", 0.0)),
            "validation_roc_auc": float(val_metrics.get("roc_auc", 0.5)),
            "validation_recall": float(val_metrics.get("recall", 0.0)),
            "validation_f1": float(val_metrics.get("f1", 0.0)),
            "validation_balanced_accuracy": float(val_metrics.get("balanced_accuracy", 0.0)),
            "validation_specificity": float(val_metrics.get("specificity", 0.0)),
            "validation_npv": float(val_metrics.get("npv", 0.0)),
            "validation_fpr": float(val_metrics.get("fpr", 1.0)),
            "validation_fnr": float(val_metrics.get("fnr", 1.0)),
            "validation_mcc": float(val_metrics.get("mcc", 0.0)),
            "validation_confusion_matrix": val_metrics.get("confusion_matrix", {}),
            "test_accuracy": float(test_metrics.get("accuracy", 0.0)),
            "test_average_precision": float(test_metrics.get("average_precision", 0.0)),
            "test_roc_auc": float(test_metrics.get("roc_auc", 0.5)),
            "test_precision": float(test_metrics.get("precision", 0.0)),
            "test_recall": float(test_metrics.get("recall", 0.0)),
            "test_f1": float(test_metrics.get("f1", 0.0)),
            "test_balanced_accuracy": float(test_metrics.get("balanced_accuracy", 0.0)),
            "test_specificity": float(test_metrics.get("specificity", 0.0)),
            "test_npv": float(test_metrics.get("npv", 0.0)),
            "test_fpr": float(test_metrics.get("fpr", 1.0)),
            "test_fnr": float(test_metrics.get("fnr", 1.0)),
            "test_mcc": float(test_metrics.get("mcc", 0.0)),
            "test_confusion_matrix": test_metrics.get("confusion_matrix", {}),
        }
    except Exception as exc:
        return _failed_arc_model_result(
            model_key="et",
            model_name="ExtraTrees",
            y_val=y_val,
            y_test=y_test,
            min_threshold=args.min_threshold,
            error_text=str(exc),
            best_params=params,
            strategy="single_pass_fast_et_prescreen",
        )


def _build_arc_model_row_optimized(stage: str, combo: list[str], selected_cols: list[str], result: dict, extra: dict | None = None) -> dict:
    row = {
        "task": "arc",
        "stage": str(stage),
        "model_key": str(result.get("model_key", "")),
        "feature_count": int(len(combo)),
        "feature_combo": "|".join(combo),
        "features": list(combo),
        "full_feature_names": list(selected_cols),
        "validation_accuracy": float(result.get("validation_accuracy", 0.0)),
        "validation_fn": int(result.get("validation_threshold_result", {}).get("fn", 0)),
        "validation_fp": int(result.get("validation_threshold_result", {}).get("fp", 0)),
        "validation_recall": float(result.get("validation_recall", 0.0)),
        "validation_precision": float(result.get("validation_precision", 0.0)),
        "validation_fpr": float(result.get("validation_fpr", 1.0)),
        "validation_average_precision": float(result.get("validation_average_precision", 0.0)),
        "validation_balanced_accuracy": float(result.get("validation_balanced_accuracy", 0.0)),
        "validation_cost": float((result.get("validation_threshold_result", {}) or {}).get("cost", 0.0)),
        "test_fn": int(result.get("test_confusion_matrix", {}).get("fn", 0)),
        "test_fp": int(result.get("test_confusion_matrix", {}).get("fp", 0)),
        "test_recall": float(result.get("test_recall", 0.0)),
        "test_precision": float(result.get("test_precision", 0.0)),
        "test_fpr": float(result.get("test_fpr", 1.0)),
        "test_average_precision": float(result.get("test_average_precision", 0.0)),
        "test_accuracy": float(result.get("test_accuracy", 0.0)),
        "test_balanced_accuracy": float(result.get("test_balanced_accuracy", 0.0)),
        "estimated_node_count": int(result.get("estimated_node_count", 0)),
        "threshold": float(result.get("threshold", 0.5)),
        "threshold_source": str(result.get("threshold_source", "")),
        "threshold_constraints_met": bool(result.get("threshold_constraints_met", False)),
        "search_strategy": str((result.get("search_summary", {}) or {}).get("strategy", "")),
    }
    if extra:
        row.update(extra)
    return row


def _build_arc_combined_row_optimized(stage: str, combo: list[str], selected_cols: list[str], rf_result: dict, et_result: dict, extra: dict | None = None) -> dict:
    rf_row = _build_arc_model_row_optimized(stage, combo, selected_cols, rf_result)
    et_row = _build_arc_model_row_optimized(stage, combo, selected_cols, et_result)
    combined = {
        "task": "arc",
        "stage": str(stage),
        "feature_count": int(len(combo)),
        "feature_combo": "|".join(combo),
        "features": list(combo),
        "full_feature_names": list(selected_cols),
        "combined_validation_cost": float(rf_row["validation_cost"] + et_row["validation_cost"]),
        "combined_validation_constraints_met_count": int(int(bool(rf_row["threshold_constraints_met"])) + int(bool(et_row["threshold_constraints_met"]))),
        "combined_validation_constraints_all_met": bool(rf_row["threshold_constraints_met"] and et_row["threshold_constraints_met"]),
        "combined_validation_fn": int(rf_row["validation_fn"] + et_row["validation_fn"]),
        "combined_validation_fp": int(rf_row["validation_fp"] + et_row["validation_fp"]),
        "combined_test_fn": int(rf_row["test_fn"] + et_row["test_fn"]),
        "combined_test_fp": int(rf_row["test_fp"] + et_row["test_fp"]),
        "combined_validation_accuracy_mean": float((rf_row["validation_accuracy"] + et_row["validation_accuracy"]) / 2.0),
        "combined_validation_balanced_accuracy_mean": float((rf_row["validation_balanced_accuracy"] + et_row["validation_balanced_accuracy"]) / 2.0),
        "combined_validation_recall_mean": float((rf_row["validation_recall"] + et_row["validation_recall"]) / 2.0),
        "combined_validation_precision_mean": float((rf_row["validation_precision"] + et_row["validation_precision"]) / 2.0),
        "combined_validation_fpr_mean": float((rf_row["validation_fpr"] + et_row["validation_fpr"]) / 2.0),
        "combined_test_recall_mean": float((rf_row["test_recall"] + et_row["test_recall"]) / 2.0),
        "combined_test_precision_mean": float((rf_row["test_precision"] + et_row["test_precision"]) / 2.0),
        "combined_test_accuracy_mean": float((rf_row["test_accuracy"] + et_row["test_accuracy"]) / 2.0),
        "combined_test_balanced_accuracy_mean": float((rf_row["test_balanced_accuracy"] + et_row["test_balanced_accuracy"]) / 2.0),
        "combined_test_average_precision_mean": float((rf_row["test_average_precision"] + et_row["test_average_precision"]) / 2.0),
        "combined_estimated_node_count": int(rf_row["estimated_node_count"] + et_row["estimated_node_count"]),
    }
    combined.update(_build_arc_model_summary("rf", combo, rf_result))
    combined.update(_build_arc_model_summary("et", combo, et_result))
    if extra:
        combined.update(extra)
    return combined


def _build_arc_model_summary(prefix: str, combo: list[str], result: dict) -> dict:
    cm = result.get("test_confusion_matrix", {}) or {}
    val_cm = result.get("validation_threshold_result", {}) or {}
    return {
        f"{prefix}_model_name": result.get("model_name"),
        f"{prefix}_threshold": float(result.get("threshold", 0.5)),
        f"{prefix}_threshold_source": str(result.get("threshold_source", "")),
        f"{prefix}_threshold_constraints_met": bool(result.get("threshold_constraints_met", False)),
        f"{prefix}_estimated_node_count": int(result.get("estimated_node_count", 0)),
        f"{prefix}_validation_cost": float(val_cm.get("cost", 0.0)),
        f"{prefix}_validation_tn": int(val_cm.get("tn", 0)),
        f"{prefix}_validation_fp": int(val_cm.get("fp", 0)),
        f"{prefix}_validation_fn": int(val_cm.get("fn", 0)),
        f"{prefix}_validation_tp": int(val_cm.get("tp", 0)),
        f"{prefix}_validation_recall": float(result.get("validation_recall", 0.0)),
        f"{prefix}_validation_precision": float(result.get("validation_precision", 0.0)),
        f"{prefix}_validation_specificity": float(result.get("validation_specificity", 0.0)),
        f"{prefix}_validation_fpr": float(result.get("validation_fpr", 1.0)),
        f"{prefix}_validation_average_precision": float(result.get("validation_average_precision", 0.0)),
        f"{prefix}_validation_balanced_accuracy": float(result.get("validation_balanced_accuracy", 0.0)),
        f"{prefix}_test_tn": int(cm.get("tn", 0)),
        f"{prefix}_test_fp": int(cm.get("fp", 0)),
        f"{prefix}_test_fn": int(cm.get("fn", 0)),
        f"{prefix}_test_tp": int(cm.get("tp", 0)),
        f"{prefix}_test_recall": float(result.get("test_recall", 0.0)),
        f"{prefix}_test_precision": float(result.get("test_precision", 0.0)),
        f"{prefix}_test_specificity": float(result.get("test_specificity", 0.0)),
        f"{prefix}_test_fpr": float(result.get("test_fpr", 1.0)),
        f"{prefix}_test_average_precision": float(result.get("test_average_precision", 0.0)),
        f"{prefix}_test_balanced_accuracy": float(result.get("test_balanced_accuracy", 0.0)),
        f"{prefix}_test_accuracy": float(result.get("test_accuracy", 0.0)),
    }


def run_arc_dual_sweep(args, tracker: dict | None = None) -> tuple[dict, pd.DataFrame]:
    feature_pool = [str(x).strip() for x in (args.features or ARC_SWEEP_FEATURES) if str(x).strip() in ARC_SWEEP_FEATURES]
    if tracker is not None:
        tracker["arc_feature_pool"] = list(feature_pool)
    feature_count_min = max(1, int(args.feature_count_min))
    feature_count_max = min(len(feature_pool), int(args.feature_count_max or len(feature_pool)))
    if feature_count_max < feature_count_min:
        feature_count_max = feature_count_min

    if tracker is not None:
        _progress_advance(tracker, 1, {
            "task": "arc",
            "stage": "load_dataset",
            "message": "Loading arc dataset and building grouped splits",
            "feature_pool_size": len(feature_pool),
        })

    full_feature_names = list(feature_pool) + list(ARC_CONTEXT_FEATURES)
    df_meta, X_all, y, groups, w = load_clean_dataset(
        args.arc_csv,
        include_invalid=args.include_invalid,
        feature_names=full_feature_names,
    )
    splits = make_group_splits(X_all, y, groups)
    train_idx = splits["train_idx"]
    val_idx = splits["val_idx"]
    train_full_idx = splits["train_full_idx"]
    X_train_full = splits["X_train_full"]
    y_train_full = splits["y_train_full"]
    X_test_full = splits["X_test"]
    y_test = splits["y_test"]

    X_train_base = X_train_full.iloc[train_idx]
    y_train_base = y_train_full.iloc[train_idx]
    groups_train_base = splits["groups_train_full"].iloc[train_idx]
    w_train_base = np.asarray(w[train_full_idx][train_idx]).astype(float)
    X_val_base = X_train_full.iloc[val_idx]
    y_val_base = y_train_full.iloc[val_idx]

    combos = _arc_feature_combinations(
        feature_pool=feature_pool,
        feature_count_min=feature_count_min,
        feature_count_max=feature_count_max,
        max_combinations=int(args.max_combinations),
    )
    total = int(len(combos))
    budget_minutes = float(getattr(args, "arc_time_budget_minutes", 0.0))
    budget_seconds = (budget_minutes * 60.0) if budget_minutes > 0 else 0.0
    shard_size = max(1, int(getattr(args, "arc_shard_size", 128)))
    keep_per_shard = max(1, int(getattr(args, "arc_keep_per_shard", 1)))
    shortlist_size = max(1, int(getattr(args, "arc_shortlist_size", 18)))
    finalist_count = max(1, int(getattr(args, "arc_finalist_count", 6)))
    arc_shortlist_n_iter = max(1, int(getattr(args, "arc_shortlist_n_iter", max(1, int(round(int(args.n_iter) * 0.25))))))
    shards = _chunk_arc_combinations(combos, shard_size)
    shard_count = len(shards)
    use_combined_rankings = False

    print("Arc subset sweep task: staged dual-model")
    print("Arc feature pool:", feature_pool)
    print("Arc feature pool size:", len(feature_pool))
    print("Arc total combinations:", total)
    print("Arc strategy: ET prescreen -> ET shortlist -> RF+ET finalists (parallel)")
    print("Arc sweep n_iter per final model:", args.n_iter)
    print("Arc shortlist n_iter:", arc_shortlist_n_iter)
    print("Arc time budget minutes:", budget_minutes)
    print("CV split summary:", splits.get("cv_group_summary", {}))

    if tracker is not None:
        tracker["arc_combo_count"] = int(total)
        _progress_advance(tracker, 1, {
            "task": "arc",
            "stage": "ready",
            "message": (
                f"Arc staged sweep ready: {total} combos | {shard_count} ET prescreen shards | "
                f"{tracker.get('arc_final_count_estimate', 0)} finalist duels"
            ),
            "feature_pool_size": len(feature_pool),
            "combo_total": int(total),
            "shard_total": int(shard_count),
        })

    rows = []
    prescreen_rows = []
    prescreen_survivors = []
    shortlist_rows = []
    rf_rows = []
    et_rows = []
    final_rows = []
    budget_hit = False
    budget_stop_stage = ""
    arc_started_at = time.monotonic()
    combo_counter = 0

    for shard_index, shard_combos in enumerate(shards, start=1):
        deadline_hit, elapsed, remaining = _arc_budget_state(arc_started_at, budget_seconds)
        if deadline_hit:
            budget_hit = True
            budget_stop_stage = "prescreen_et"
            break
        shard_rows = []
        for combo in shard_combos:
            deadline_hit, elapsed, remaining = _arc_budget_state(arc_started_at, budget_seconds)
            if deadline_hit:
                budget_hit = True
                budget_stop_stage = "prescreen_et"
                break
            combo_counter += 1
            selected_cols = list(combo) + list(ARC_CONTEXT_FEATURES)
            combo_key = "|".join(combo)
            result = _run_arc_et_prescreen(
                X_train=X_train_base[selected_cols],
                y_train=y_train_base,
                w_train=w_train_base,
                X_val=X_val_base[selected_cols],
                y_val=y_val_base,
                X_test=X_test_full[selected_cols],
                y_test=y_test,
                args=args,
            )
            row = _build_arc_model_row_optimized(
                stage="prescreen_et",
                combo=list(combo),
                selected_cols=selected_cols,
                result=result,
                extra={
                    "combo_index": int(combo_counter),
                    "combo_total": int(total),
                    "shard_index": int(shard_index),
                    "shard_total": int(shard_count),
                    "elapsed_seconds": float(elapsed),
                    "remaining_budget_seconds": None if remaining is None else float(remaining),
                },
            )
            prescreen_rows.append(row)
            rows.append(row)
            shard_rows.append(row)
            message = (
                f"Arc prescreen shard {shard_index}/{max(1, shard_count)} | combo {combo_counter}/{max(1, total)} | "
                f"Val BalAcc={row.get('validation_balanced_accuracy', 0.0):.4f} FPR={row.get('validation_fpr', 1.0):.4f} "
                f"FN={row['validation_fn']} FP={row['validation_fp']}"
            )
            if tracker is not None:
                _progress_advance(tracker, 1, {
                    "task": "arc",
                    "stage": "prescreen_combo",
                    "message": message,
                    "combo_index": int(combo_counter),
                    "combo_total": int(total),
                    "feature_combo": combo_key,
                    "feature_count": int(len(combo)),
                    "shard_index": int(shard_index),
                    "shard_total": int(shard_count),
                })
            else:
                emit_progress(combo_counter, max(1, total), {
                    "task": "arc",
                    "stage": "prescreen_combo",
                    "message": message,
                    "feature_combo": combo_key,
                    "feature_count": int(len(combo)),
                })
        kept = _select_diverse_arc_rows(shard_rows, keep_total=keep_per_shard, per_feature_count=1)
        prescreen_survivors.extend(kept)
        if budget_hit:
            break

    shortlist_seed_rows = _select_diverse_arc_rows(
        prescreen_survivors,
        keep_total=shortlist_size,
        per_feature_count=1,
    )

    if tracker is not None:
        _progress_advance(tracker, 1, {
            "task": "arc",
            "stage": "shortlist_ready",
            "message": (
                f"Arc ET shortlist: {len(shortlist_seed_rows)} combos chosen from {len(prescreen_rows)} prescreened"
                + (" (budget stop)" if budget_hit else "")
            ),
            "combo_total": int(len(shortlist_seed_rows)),
        })

    shortlist_plan_steps = int((tracker or {}).get("arc_shortlist_steps", estimate_search_plan(int(arc_shortlist_n_iter)).get("progress_steps", 1)))
    for shortlist_index, seed_row in enumerate(shortlist_seed_rows, start=1):
        deadline_hit, elapsed, remaining = _arc_budget_state(arc_started_at, budget_seconds)
        if deadline_hit:
            budget_hit = True
            budget_stop_stage = "shortlist_et"
            break
        combo = list(seed_row.get("features", []))
        selected_cols = combo + list(ARC_CONTEXT_FEATURES)
        combo_key = "|".join(combo)
        combo_base_current = int(tracker.get("current", 0)) if tracker is not None else 0
        result = _safe_train_arc_model(
            model_key="et",
            X_train=X_train_base[selected_cols],
            y_train=y_train_base,
            groups_train=groups_train_base,
            w_train=w_train_base,
            X_val=X_val_base[selected_cols],
            y_val=y_val_base,
            X_test=X_test_full[selected_cols],
            y_test=y_test,
            n_iter=arc_shortlist_n_iter,
            args=args,
            progress_callback=_progress_child_callback(
                tracker,
                combo_base_current,
                0,
                shortlist_plan_steps,
                "arc",
                shortlist_index,
                len(shortlist_seed_rows),
                combo_key,
            ),
        )
        row = _build_arc_model_row_optimized(
            stage="shortlist_et",
            combo=combo,
            selected_cols=selected_cols,
            result=result,
            extra={
                "shortlist_index": int(shortlist_index),
                "shortlist_total": int(len(shortlist_seed_rows)),
                "seed_stage": "prescreen_et",
                "elapsed_seconds": float(elapsed),
                "remaining_budget_seconds": None if remaining is None else float(remaining),
            },
        )
        shortlist_rows.append(row)
        rows.append(row)
        if tracker is not None:
            _progress_set(tracker, combo_base_current + shortlist_plan_steps, {
                "task": "arc",
                "stage": "shortlist_done",
                "message": (
                    f"Arc shortlist {shortlist_index}/{len(shortlist_seed_rows)} done | "
                    f"Val BalAcc={row.get('validation_balanced_accuracy', 0.0):.4f} FPR={row.get('validation_fpr', 1.0):.4f} "
                    f"FN={row['validation_fn']} FP={row['validation_fp']}"
                ),
                "combo_index": int(shortlist_index),
                "combo_total": int(len(shortlist_seed_rows)),
                "feature_combo": combo_key,
                "feature_count": int(len(combo)),
            })

    finalist_seed_rows = _select_diverse_arc_rows(
        shortlist_rows if shortlist_rows else prescreen_rows,
        keep_total=finalist_count,
        per_feature_count=1,
    )

    if tracker is not None:
        _progress_advance(tracker, 1, {
            "task": "arc",
            "stage": "finalists_ready",
            "message": (
                f"Arc full-depth finalists: {len(finalist_seed_rows)} combos"
                + (" from shortlist" if shortlist_rows else " from prescreen")
            ),
            "combo_total": int(len(finalist_seed_rows)),
        })

    final_model_steps = int((tracker or {}).get("arc_final_model_steps", estimate_search_plan(int(args.n_iter)).get("progress_steps", 1)))
    for finalist_index, seed_row in enumerate(finalist_seed_rows, start=1):
        deadline_hit, elapsed, remaining = _arc_budget_state(arc_started_at, budget_seconds)
        if deadline_hit:
            budget_hit = True
            budget_stop_stage = "final_duel"
            break
        combo = list(seed_row.get("features", []))
        selected_cols = combo + list(ARC_CONTEXT_FEATURES)
        combo_key = "|".join(combo)
        print(f"\n=== Arc finalist {finalist_index}/{len(finalist_seed_rows)}: {combo_key} ===")
        combo_base_current = int(tracker.get("current", 0)) if tracker is not None else 0
        rf_callback = _progress_child_callback(tracker, combo_base_current, 0, final_model_steps, "arc", finalist_index, len(finalist_seed_rows), combo_key)
        et_callback = _progress_child_callback(tracker, combo_base_current, final_model_steps, final_model_steps, "arc", finalist_index, len(finalist_seed_rows), combo_key)
        with ThreadPoolExecutor(max_workers=2) as executor:
            rf_future = executor.submit(
                _safe_train_arc_model,
                "rf",
                X_train_base[selected_cols],
                y_train_base,
                groups_train_base,
                w_train_base,
                X_val=X_val_base[selected_cols],
                y_val=y_val_base,
                X_test=X_test_full[selected_cols],
                y_test=y_test,
                n_iter=int(args.n_iter),
                args=args,
                progress_callback=rf_callback,
            )
            et_future = executor.submit(
                _safe_train_arc_model,
                "et",
                X_train_base[selected_cols],
                y_train_base,
                groups_train_base,
                w_train_base,
                X_val=X_val_base[selected_cols],
                y_val=y_val_base,
                X_test=X_test_full[selected_cols],
                y_test=y_test,
                n_iter=int(args.n_iter),
                args=args,
                progress_callback=et_callback,
            )
            rf_result = rf_future.result()
            et_result = et_future.result()
        rf_row = _build_arc_model_row_optimized("final_rf", combo, selected_cols, rf_result, {
            "finalist_index": int(finalist_index),
            "finalist_total": int(len(finalist_seed_rows)),
            "elapsed_seconds": float(elapsed),
            "remaining_budget_seconds": None if remaining is None else float(remaining),
        })
        et_row = _build_arc_model_row_optimized("final_et", combo, selected_cols, et_result, {
            "finalist_index": int(finalist_index),
            "finalist_total": int(len(finalist_seed_rows)),
            "elapsed_seconds": float(elapsed),
            "remaining_budget_seconds": None if remaining is None else float(remaining),
        })
        combined = _build_arc_combined_row_optimized(
            stage="final_duel",
            combo=combo,
            selected_cols=selected_cols,
            rf_result=rf_result,
            et_result=et_result,
            extra={
                "finalist_index": int(finalist_index),
                "finalist_total": int(len(finalist_seed_rows)),
                "elapsed_seconds": float(elapsed),
                "remaining_budget_seconds": None if remaining is None else float(remaining),
            },
        )
        rf_rows.append(rf_row)
        et_rows.append(et_row)
        final_rows.append(combined)
        rows.extend([rf_row, et_row, combined])
        use_combined_rankings = True
        if tracker is not None:
            _progress_set(tracker, combo_base_current + (2 * final_model_steps) + 1, {
                "task": "arc",
                "stage": "final_combo_done",
                "message": (
                    f"Arc finalist {finalist_index}/{len(finalist_seed_rows)} done | "
                    f"Val BalAcc={combined.get('combined_validation_balanced_accuracy_mean', 0.0):.4f} "
                    f"FPR={combined.get('combined_validation_fpr_mean', 1.0):.4f} "
                    f"FN(sum)={combined['combined_validation_fn']} FP(sum)={combined['combined_validation_fp']}"
                ),
                "combo_index": int(finalist_index),
                "combo_total": int(len(finalist_seed_rows)),
                "feature_combo": combo_key,
                "feature_count": int(len(combo)),
            })

    results_df = pd.DataFrame(rows)

    deepest_rows = final_rows if final_rows else (shortlist_rows if shortlist_rows else prescreen_rows)
    et_best_source = et_rows if et_rows else (shortlist_rows if shortlist_rows else prescreen_rows)

    ranking_policies = {
        "practical": {
            "label": "Practical accuracy",
            "combined_sort": _sort_combined_row_practical,
            "model_sort": _sort_model_row_practical,
        },
        "validation_cost": {
            "label": "Balanced validation cost",
            "combined_sort": _sort_combined_row_validation_cost,
            "model_sort": _sort_model_row_validation_cost,
        },
        "fn_first": {
            "label": "FN-first",
            "combined_sort": _sort_combined_row_fn_first,
            "model_sort": _sort_model_row_fn_first,
        },
        "accuracy": {
            "label": "Accuracy-first",
            "combined_sort": _sort_combined_row_accuracy,
            "model_sort": _sort_model_row_accuracy,
        },
    }

    ranking_sections = {}
    for rank_key, cfg in ranking_policies.items():
        candidate_sort = cfg["combined_sort"] if use_combined_rankings else cfg["model_sort"]
        combined_best = sorted(deepest_rows, key=candidate_sort)[0] if deepest_rows else {}
        rf_best = sorted(rf_rows, key=cfg["model_sort"])[0] if rf_rows else {}
        et_best = sorted(et_best_source, key=cfg["model_sort"])[0] if et_best_source else {}
        best_by_feature_count = []
        for feature_count in range(feature_count_min, feature_count_max + 1):
            bucket_rows = [r for r in deepest_rows if int(r.get("feature_count", 0)) == int(feature_count)]
            if not bucket_rows:
                continue
            bucket_rf = [r for r in rf_rows if int(r.get("feature_count", 0)) == int(feature_count)]
            bucket_et = [r for r in et_best_source if int(r.get("feature_count", 0)) == int(feature_count)]
            best_by_feature_count.append({
                "feature_count": int(feature_count),
                "best_combined": sorted(bucket_rows, key=candidate_sort)[0],
                "best_rf": sorted(bucket_rf, key=cfg["model_sort"])[0] if bucket_rf else {},
                "best_et": sorted(bucket_et, key=cfg["model_sort"])[0] if bucket_et else {},
            })
        ranking_sections[rank_key] = {
            "label": cfg["label"],
            "overall_best_combined": combined_best,
            "overall_best_rf": rf_best,
            "overall_best_et": et_best,
            "best_by_feature_count": best_by_feature_count,
        }

    practical_section = ranking_sections.get("practical", {})
    combined_best = practical_section.get("overall_best_combined", {}) or {}
    rf_best = practical_section.get("overall_best_rf", {}) or {}
    et_best = practical_section.get("overall_best_et", {}) or {}
    best_by_feature_count = practical_section.get("best_by_feature_count", []) or []

    report = {
        "task": "arc_dual_model_staged",
        "models": ["rf", "et"],
        "feature_pool_role": "arc_base_features",
        "feature_pool": feature_pool,
        "feature_pool_size": len(feature_pool),
        "context_inputs_fixed": list(ARC_CONTEXT_FEATURES),
        "context_input_count": len(ARC_CONTEXT_FEATURES),
        "strategy": "fast_et_prescreen_then_et_shortlist_then_parallel_rf_et_finalists",
        "parallel_final_models": True,
        "total_combinations": int(total),
        "screened_combinations": int(len(prescreen_rows)),
        "prescreen_survivor_count": int(len(prescreen_survivors)),
        "shortlist_seed_count": int(len(shortlist_seed_rows)),
        "shortlist_evaluated_count": int(len(shortlist_rows)),
        "finalist_seed_count": int(len(finalist_seed_rows)),
        "finalist_evaluated_count": int(len(final_rows)),
        "budget_minutes": float(budget_minutes),
        "budget_hit": bool(budget_hit),
        "budget_stop_stage": str(budget_stop_stage),
        "all_results_csv": args.out_csv,
        "ranking_policy_default": "practical",
        "ranking_sections": ranking_sections,
        "recommended_arc_base_features_global": list(combined_best.get("features", [])),
        "recommended_features_global": list(combined_best.get("features", [])),
        "recommended_by_model": {
            "rf": list(rf_best.get("features", [])),
            "et": list(et_best.get("features", [])),
        },
        "overall_best_combined_tradeoff": combined_best,
        "overall_best_practical": ranking_sections.get("practical", {}).get("overall_best_combined", {}) or {},
        "overall_best_validation_cost": ranking_sections.get("validation_cost", {}).get("overall_best_combined", {}) or {},
        "overall_best_fn_first": ranking_sections.get("fn_first", {}).get("overall_best_combined", {}) or {},
        "overall_best_accuracy": ranking_sections.get("accuracy", {}).get("overall_best_combined", {}) or {},
        "overall_best_rf": rf_best,
        "overall_best_et": et_best,
        "best_by_feature_count": best_by_feature_count,
        "best_by_feature_count_practical": ranking_sections.get("practical", {}).get("best_by_feature_count", []) or [],
        "best_by_feature_count_validation_cost": ranking_sections.get("validation_cost", {}).get("best_by_feature_count", []) or [],
        "best_by_feature_count_fn_first": ranking_sections.get("fn_first", {}).get("best_by_feature_count", []) or [],
        "best_by_feature_count_accuracy": ranking_sections.get("accuracy", {}).get("best_by_feature_count", []) or [],
        "stage_summaries": [
            {
                "stage": "prescreen_et",
                "description": "Single-pass fast ET prescreen across all requested combos",
                "model": "et",
                "scanned_combinations": int(len(prescreen_rows)),
                "shard_size": int(shard_size),
                "shard_count": int(shard_count),
                "keep_per_shard": int(keep_per_shard),
                "selected_count": int(len(shortlist_seed_rows)),
            },
            {
                "stage": "shortlist_et",
                "description": "Deeper ET rerank of prescreen survivors",
                "model": "et",
                "n_iter": int(arc_shortlist_n_iter),
                "candidate_count": int(len(shortlist_seed_rows)),
                "evaluated_count": int(len(shortlist_rows)),
            },
            {
                "stage": "final_duel",
                "description": "Full-depth RF + ET evaluation on the strongest finalists",
                "models": ["rf", "et"],
                "parallel_models": True,
                "n_iter": int(args.n_iter),
                "candidate_count": int(len(finalist_seed_rows)),
                "evaluated_count": int(len(final_rows)),
            },
        ],
        "split_summary": splits.get("cv_group_summary", {}),
        "settings": {
            "arc_csv": args.arc_csv,
            "include_invalid": args.include_invalid,
            "fn_weight": args.fn_weight,
            "fp_weight": args.fp_weight,
            "min_recall": args.min_recall,
            "min_precision": args.min_precision,
            "max_fpr": args.max_fpr,
            "min_threshold": args.min_threshold,
            "n_iter": args.n_iter,
            "n_jobs": getattr(args, "n_jobs", 1),
            "feature_count_min": feature_count_min,
            "feature_count_max": feature_count_max,
            "max_combinations": int(args.max_combinations),
            "arc_time_budget_minutes": float(budget_minutes),
            "arc_shard_size": int(shard_size),
            "arc_keep_per_shard": int(keep_per_shard),
            "arc_shortlist_size": int(shortlist_size),
            "arc_finalist_count": int(finalist_count),
            "arc_prescreen_trees": int(getattr(args, "arc_prescreen_trees", 64)),
            "arc_prescreen_max_depth": int(getattr(args, "arc_prescreen_max_depth", 8)),
            "arc_shortlist_n_iter": int(arc_shortlist_n_iter),
        },
    }
    if tracker is not None:
        _progress_advance(tracker, 1, {
            "task": "arc",
            "stage": "arc_done",
            "message": (
                f"Arc staged sweep complete | screened {len(prescreen_rows)} combos | "
                f"shortlisted {len(shortlist_rows)} | finalists {len(final_rows)}"
            ),
            "combo_total": int(total),
        })
    return report, results_df


# ----------------------------
# Context sweep helpers
# ----------------------------

def _sort_context_row(row: dict):
    return (
        -float(row.get("mean_accuracy", 0.0)),
        -float(row.get("mean_balanced_accuracy", 0.0)),
        float(row.get("mean_unknown_rate", 1.0)),
        -float(row.get("mean_confidence", 0.0)),
        int(row.get("feature_count", 10**9)),
        str(row.get("feature_combo", "")),
    )


def run_context_sweep(args, tracker: dict | None = None) -> tuple[dict, pd.DataFrame]:
    feature_pool = [str(x).strip() for x in (args.context_features or CONTEXT_SWEEP_FEATURES) if str(x).strip() in CONTEXT_SWEEP_FEATURES]
    if tracker is not None:
        tracker["context_feature_pool"] = list(feature_pool)
    feature_count_min = max(1, int(args.context_feature_count_min or args.feature_count_min))
    feature_count_max = min(len(feature_pool), int(args.context_feature_count_max or len(feature_pool)))
    if feature_count_max < feature_count_min:
        feature_count_max = feature_count_min

    if tracker is not None:
        _progress_advance(tracker, 1, {
            "task": "context",
            "stage": "load_dataset",
            "message": "Loading context dataset and building grouped context frames",
            "feature_pool_size": len(feature_pool),
        })

    raw_df = pd.read_csv(args.context_csv)
    total = sum(1 for k in range(feature_count_min, feature_count_max + 1) for _ in itertools.combinations(feature_pool, k))
    if args.context_max_combinations and args.context_max_combinations > 0:
        total = min(total, int(args.context_max_combinations))

    print("Context subset sweep task: context-prototype")
    print("Context feature pool:", feature_pool)
    print("Context total combinations:", total)
    print("Context repeats per combination:", args.context_repeats)

    if tracker is not None:
        tracker["context_combo_count"] = int(total)
        _progress_advance(tracker, 1, {
            "task": "context",
            "stage": "ready",
            "message": f"Context sweep ready: {total} combos, {max(1, int(args.context_repeats))} repeats/combo",
            "feature_pool_size": len(feature_pool),
            "combo_total": int(total),
        })

    rows = []
    combo_counter = 0
    stop = False

    for feature_count in range(feature_count_min, feature_count_max + 1):
        for combo in itertools.combinations(feature_pool, feature_count):
            combo_counter += 1
            if args.context_max_combinations and args.context_max_combinations > 0 and combo_counter > int(args.context_max_combinations):
                stop = True
                break
            combo = list(combo)
            combo_key = "|".join(combo)
            if tracker is not None:
                _progress_advance(tracker, 1, {
                    "task": "context",
                    "stage": "combo_setup",
                    "message": f"Context combo {combo_counter}/{total} setup",
                    "combo_index": int(combo_counter),
                    "combo_total": int(total),
                    "feature_combo": combo_key,
                    "feature_count": int(feature_count),
                })
            ctx_df = build_context_frame(raw_df, combo)
            class_counts = ctx_df["device_family"].value_counts().to_dict()
            valid_classes = [fam for fam in DEVICE_FAMILY_CLASSES if class_counts.get(fam, 0) > 0]
            y = ctx_df["device_family_code"].astype(int).to_numpy()
            X = ctx_df[combo].astype(float).to_numpy()
            weights = pd.to_numeric(ctx_df.get("sample_weight", pd.Series(1.0, index=ctx_df.index)), errors="coerce").fillna(1.0).to_numpy(dtype=float)
            n_rows = len(ctx_df)

            accuracy_vals = []
            balacc_vals = []
            unknown_vals = []
            conf_vals = []
            active_counts = []
            valid_repeats = 0

            repeat_budget = max(1, int(args.context_repeats))
            if len(valid_classes) >= 2 and n_rows >= len(valid_classes) * 2:
                for rep in range(repeat_budget):
                    if tracker is not None:
                        _progress_advance(tracker, 1, {
                            "task": "context",
                            "stage": "repeat",
                            "message": f"Context combo {combo_counter}/{total} repeat {rep + 1}/{repeat_budget}",
                            "combo_index": int(combo_counter),
                            "combo_total": int(total),
                            "feature_combo": combo_key,
                            "feature_count": int(feature_count),
                        })
                    random_state = 42 + rep
                    try:
                        splitter = StratifiedShuffleSplit(n_splits=1, test_size=max(len(valid_classes), int(round(n_rows * 0.25))), random_state=random_state)
                        train_idx, test_idx = next(splitter.split(X, y))
                    except Exception:
                        continue
                    X_train, X_test = X[train_idx], X[test_idx]
                    y_train, y_test = y[train_idx], y[test_idx]
                    w_train = weights[train_idx]
                    X_train_std, X_test_std, means, stds = weighted_standardize(X_train, X_test, w_train)
                    centroids = np.zeros((len(DEVICE_FAMILY_CLASSES), X_train_std.shape[1]), dtype=float)
                    active_mask = []
                    for fam_idx, _ in enumerate(DEVICE_FAMILY_CLASSES):
                        mask = y_train == fam_idx
                        active_mask.append(1 if np.any(mask) else 0)
                        if np.any(mask):
                            fam_w = np.clip(w_train[mask], 1e-9, None)
                            centroids[fam_idx] = np.average(X_train_std[mask], axis=0, weights=fam_w)
                    proba = prototype_predict_proba(X_test_std, centroids, active_class_mask=active_mask)
                    pred = np.argmax(proba, axis=1)
                    conf = np.max(proba, axis=1)
                    pred_unknown = np.where(conf >= float(args.unknown_confidence), pred, -1)
                    accuracy_vals.append(float(np.mean(pred == y_test)) if len(y_test) else 0.0)
                    bal_scores = []
                    for fam_idx in np.unique(y_test):
                        fam_mask = y_test == fam_idx
                        bal_scores.append(float(np.mean(pred[fam_mask] == fam_idx)) if fam_mask.any() else 0.0)
                    balacc_vals.append(float(np.mean(bal_scores)) if bal_scores else 0.0)
                    unknown_vals.append(float(np.mean(pred_unknown == -1)) if len(pred_unknown) else 0.0)
                    conf_vals.append(float(np.mean(conf)) if len(conf) else 0.0)
                    active_counts.append(int(sum(active_mask)))
                    valid_repeats += 1
            else:
                if tracker is not None:
                    _progress_advance(tracker, repeat_budget, {
                        "task": "context",
                        "stage": "repeat_skip",
                        "message": f"Context combo {combo_counter}/{total} skipped repeats (insufficient known-family coverage)",
                        "combo_index": int(combo_counter),
                        "combo_total": int(total),
                        "feature_combo": combo_key,
                        "feature_count": int(feature_count),
                    })

            row = {
                "task": "context",
                "feature_count": int(feature_count),
                "feature_combo": combo_key,
                "features": combo,
                "row_count": int(n_rows),
                "valid_class_count": int(len(valid_classes)),
                "missing_classes": [fam for fam in DEVICE_FAMILY_CLASSES if fam not in valid_classes],
                "repeats": int(valid_repeats),
                "mean_accuracy": float(np.mean(accuracy_vals)) if accuracy_vals else 0.0,
                "mean_balanced_accuracy": float(np.mean(balacc_vals)) if balacc_vals else 0.0,
                "mean_unknown_rate": float(np.mean(unknown_vals)) if unknown_vals else 1.0,
                "mean_confidence": float(np.mean(conf_vals)) if conf_vals else 0.0,
                "mean_active_class_count": float(np.mean(active_counts)) if active_counts else 0.0,
            }
            rows.append(row)
            if tracker is not None:
                _progress_advance(tracker, 1, {
                    "task": "context",
                    "stage": "combo_done",
                    "message": f"Context combo {combo_counter}/{total} done | acc={row['mean_accuracy']:.4f} bal={row['mean_balanced_accuracy']:.4f}",
                    "combo_index": int(combo_counter),
                    "combo_total": int(total),
                    "feature_combo": combo_key,
                    "feature_count": int(feature_count),
                })
            else:
                emit_progress(combo_counter, total, {
                    "task": "context",
                    "stage": "combo_done",
                    "message": f"Context combo {combo_counter}/{total} done | acc={row['mean_accuracy']:.4f} bal={row['mean_balanced_accuracy']:.4f}",
                    "feature_combo": combo_key,
                    "feature_count": int(feature_count),
                })
        if stop:
            break

    results_df = pd.DataFrame(rows)
    best_row = sorted(rows, key=_sort_context_row)[0] if rows else {}
    best_by_feature_count = []
    for feature_count in range(feature_count_min, feature_count_max + 1):
        bucket = [r for r in rows if int(r.get("feature_count", 0)) == int(feature_count)]
        if bucket:
            best_by_feature_count.append({
                "feature_count": int(feature_count),
                "best_overall": sorted(bucket, key=_sort_context_row)[0],
            })

    report = {
        "task": "context",
        "model": "context_prototype",
        "feature_pool_role": "context_features",
        "feature_pool": feature_pool,
        "feature_pool_size": len(feature_pool),
        "total_combinations": int(total),
        "all_results_csv": args.context_out_csv,
        "recommended_context_features": list(best_row.get("features", [])),
        "recommended_features": list(best_row.get("features", [])),
        "overall_best_tradeoff": best_row,
        "best_by_feature_count": best_by_feature_count,
        "settings": {
            "context_csv": args.context_csv,
            "unknown_confidence": args.unknown_confidence,
            "context_repeats": int(args.context_repeats),
            "feature_count_min": feature_count_min,
            "feature_count_max": feature_count_max,
            "max_combinations": int(args.context_max_combinations),
        },
    }
    return report, results_df


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--task", default="auto", choices=["auto", "arc", "context"])
    ap.add_argument("--arc_csv", default=ARC_CSV_PATH)
    ap.add_argument("--context_csv", default=CONTEXT_CSV_PATH)
    ap.add_argument("--out_report", default=ARC_OUT_REPORT)
    ap.add_argument("--out_csv", default=ARC_OUT_CSV)
    ap.add_argument("--context_out_report", default=CONTEXT_OUT_REPORT)
    ap.add_argument("--context_out_csv", default=CONTEXT_OUT_CSV)
    ap.add_argument("--include_invalid", action="store_true")
    ap.add_argument("--fn_weight", type=float, default=80.0)
    ap.add_argument("--fp_weight", type=float, default=4.0)
    ap.add_argument("--min_recall", type=float, default=0.97)
    ap.add_argument("--min_precision", type=float, default=0.90)
    ap.add_argument("--max_fpr", type=float, default=0.03)
    ap.add_argument("--min_threshold", type=float, default=0.08)
    ap.add_argument("--n_iter", type=int, default=24)
    ap.add_argument("--n_jobs", type=int, default=1)
    ap.add_argument("--feature_count_min", type=int, default=1)
    ap.add_argument("--feature_count_max", type=int, default=len(ARC_SWEEP_FEATURES))
    ap.add_argument("--max_combinations", type=int, default=0)
    ap.add_argument("--features", nargs="*", default=None)
    ap.add_argument("--arc_time_budget_minutes", type=float, default=60.0)
    ap.add_argument("--arc_shard_size", type=int, default=128)
    ap.add_argument("--arc_keep_per_shard", type=int, default=1)
    ap.add_argument("--arc_shortlist_size", type=int, default=18)
    ap.add_argument("--arc_finalist_count", type=int, default=6)
    ap.add_argument("--arc_prescreen_trees", type=int, default=64)
    ap.add_argument("--arc_prescreen_max_depth", type=int, default=8)
    ap.add_argument("--arc_shortlist_n_iter", type=int, default=6)
    ap.add_argument("--context_feature_count_min", type=int, default=1)
    ap.add_argument("--context_feature_count_max", type=int, default=len(CONTEXT_SWEEP_FEATURES))
    ap.add_argument("--context_max_combinations", type=int, default=0)
    ap.add_argument("--context_features", nargs="*", default=None)
    ap.add_argument("--context_repeats", type=int, default=24)
    ap.add_argument("--unknown_confidence", type=float, default=0.45)
    args = ap.parse_args()

    if args.n_jobs != 0:
        os.environ["TINYML_N_JOBS"] = str(args.n_jobs)

    tracker = _make_progress_tracker(args)
    _progress_advance(tracker, 1, {
        "task": args.task,
        "stage": "plan",
        "message": (
            f"Planning subset sweep | arc combos={tracker.get('arc_combo_count', 0)} | "
            f"context combos={tracker.get('context_combo_count', 0)} | "
            f"total progress steps={tracker.get('total', 0)}"
        ),
        "arc_combo_count": tracker.get("arc_combo_count", 0),
        "context_combo_count": tracker.get("context_combo_count", 0),
        "total_progress_steps": tracker.get("total", 0),
    })

    if args.task in ("auto", "arc"):
        arc_report, arc_df = run_arc_dual_sweep(args, tracker=tracker)
        ensure_dir(args.out_csv)
        arc_df.to_csv(args.out_csv, index=False)
        ensure_dir(args.out_report)
        Path(args.out_report).write_text(json.dumps(arc_report, indent=2), encoding="utf-8")
        print("Saved arc subset CSV:", args.out_csv)
        print("Saved arc subset report:", args.out_report)

    if args.task in ("auto", "context"):
        context_report, context_df = run_context_sweep(args, tracker=tracker)
        ensure_dir(args.context_out_csv)
        context_df.to_csv(args.context_out_csv, index=False)
        ensure_dir(args.context_out_report)
        Path(args.context_out_report).write_text(json.dumps(context_report, indent=2), encoding="utf-8")
        print("Saved context subset CSV:", args.context_out_csv)
        print("Saved context subset report:", args.context_out_report)

    _progress_advance(tracker, 1, {
        "task": args.task,
        "stage": "workflow_done",
        "message": "Subset sweep reports saved",
        "all_models_done": True,
    })


if __name__ == "__main__":
    main()
