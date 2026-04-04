import json
import os
import warnings
from dataclasses import dataclass
from typing import Dict, List, Tuple

import joblib
import m2cgen as m2c
import numpy as np
import pandas as pd

from sklearn.ensemble import ExtraTreesClassifier, RandomForestClassifier
from sklearn.metrics import (
    accuracy_score,
    average_precision_score,
    balanced_accuracy_score,
    classification_report,
    confusion_matrix,
    f1_score,
    matthews_corrcoef,
    precision_score,
    recall_score,
    roc_auc_score,
)
from sklearn.base import clone
from sklearn.model_selection import GroupKFold, GroupShuffleSplit, StratifiedShuffleSplit

try:
    from sklearn.model_selection import StratifiedGroupKFold
except Exception:
    StratifiedGroupKFold = None


FEATURES = [
    "cycle_nmse",
    "zcv",
    "zc_dwell_ratio",
    "cycle_rms_drop_ratio",
    "peak_fluct_cv",
    "midband_residual_rms",
    "hf_band_energy_ratio",
    "spec_entropy",
    "neg_dip_event_ratio",
    "irms_drop_vs_baseline",
]
TARGET = "label_arc"
GROUP_COL_CANDIDATES = ["session_id", "session", "sid"]


def resolve_n_jobs(default: int = -1) -> int:
    raw = os.environ.get("TINYML_N_JOBS", "").strip()
    if not raw:
        return int(default)
    try:
        value = int(raw)
    except Exception:
        return int(default)
    if value == 0:
        return int(default)
    return int(value)


MODEL_CONFIGS = {
    "rf": {
        "pretty_name": "RandomForest",
        "builder": lambda: RandomForestClassifier(
            random_state=42,
            class_weight="balanced_subsample",
            n_jobs=resolve_n_jobs(-1),
            bootstrap=True,
            oob_score=False,
        ),
        "param_space": {
            "n_estimators": [80, 120, 160, 220, 280, 360],
            "max_depth": [4, 6, 8, 10, 12, 16, 20, None],
            "min_samples_leaf": [1, 2, 3, 4, 6, 8],
            "min_samples_split": [2, 4, 6, 8, 12, 16],
            "max_features": ["sqrt", "log2", 0.35, 0.5, 0.65, 0.8, 1.0],
            "bootstrap": [True, False],
            "class_weight": ["balanced", "balanced_subsample", None],
            "criterion": ["gini", "entropy"],
            "ccp_alpha": [0.0, 0.00001, 0.00005, 0.0001, 0.0002, 0.0005, 0.001],
            "max_leaf_nodes": [None, 32, 48, 64, 96, 128, 160],
            "min_impurity_decrease": [0.0, 0.000001, 0.00001, 0.00005, 0.0001],
        },
    },
    "et": {
        "pretty_name": "ExtraTrees",
        "builder": lambda: ExtraTreesClassifier(
            random_state=42,
            class_weight="balanced",
            n_jobs=resolve_n_jobs(-1),
            bootstrap=False,
        ),
        "param_space": {
            "n_estimators": [80, 120, 160, 220, 280, 360],
            "max_depth": [4, 6, 8, 10, 12, 16, 20, None],
            "min_samples_leaf": [1, 2, 3, 4, 6, 8],
            "min_samples_split": [2, 4, 6, 8, 12, 16],
            "max_features": ["sqrt", "log2", 0.35, 0.5, 0.65, 0.8, 1.0],
            "bootstrap": [False, True],
            "class_weight": ["balanced", None],
            "criterion": ["gini", "entropy"],
            "ccp_alpha": [0.0, 0.00001, 0.00005, 0.0001, 0.0002, 0.0005, 0.001],
            "max_leaf_nodes": [None, 32, 48, 64, 96, 128, 160],
            "min_impurity_decrease": [0.0, 0.000001, 0.00001, 0.00005, 0.0001],
        },
    },
}



def ensure_dir(path: str) -> None:
    folder = os.path.dirname(path)
    if folder:
        os.makedirs(folder, exist_ok=True)


def pick_group_column(df: pd.DataFrame) -> str | None:
    for col in GROUP_COL_CANDIDATES:
        if col in df.columns:
            return col
    return None


def normalize_feature_names(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()
    df.columns = [str(c).strip() for c in df.columns]

    aliases = {
        "i_rms": "i_rms",
        "v_rms": "v_rms",
        "temp_c": "temp_c",
        "label_arc": "label_arc",
        "session_id": "session_id",
        "epoch_ms": "epoch_ms",
        "adc_fs_hz": "adc_fs_hz",
        "feat_valid": "feat_valid",
        "current_valid": "current_valid",
        "rf_train_row": "rf_train_row",
        "sample_weight": "sample_weight",
        "cycle_nmse": "cycle_nmse",
        "zcv": "zcv",
        "zc_dwell_ratio": "zc_dwell_ratio",
        "cycle_rms_drop_ratio": "cycle_rms_drop_ratio",
        "peak_fluct_cv": "peak_fluct_cv",
        "midband_residual_rms": "midband_residual_rms",
        "hf_band_energy_ratio": "hf_band_energy_ratio",
        "spec_entropy": "spec_entropy",
        "neg_dip_event_ratio": "neg_dip_event_ratio",
        "irms_drop_vs_baseline": "irms_drop_vs_baseline",
        "pre_dip_spike_ratio": "irms_drop_vs_baseline",
        "thd_i": "thd_i",
        "pulse_count_per_cycle": "pulse_count_per_cycle",
    }

    rename_map = {
        c: aliases[str(c).strip()]
        for c in df.columns
        if str(c).strip() in aliases
    }
    if rename_map:
        df = df.rename(columns=rename_map)

    if (
        "cycle_rms_drop_ratio" not in df.columns
        and "pulse_count_per_cycle" in df.columns
    ):
        df["cycle_rms_drop_ratio"] = pd.to_numeric(
            df["pulse_count_per_cycle"],
            errors="coerce",
        )

    for missing in ["neg_dip_event_ratio", "irms_drop_vs_baseline", "thd_i"]:
        if missing not in df.columns:
            df[missing] = 0.0

    return df


def clean_df(df: pd.DataFrame, include_invalid: bool = False) -> pd.DataFrame:
    df = normalize_feature_names(df.copy())

    missing = [c for c in FEATURES + [TARGET] if c not in df.columns]
    if missing:
        raise ValueError(f"Missing required columns: {missing}")

    df[TARGET] = pd.to_numeric(df[TARGET], errors="coerce")
    df = df[df[TARGET].isin([0, 1])].copy()

    for c in FEATURES:
        df[c] = pd.to_numeric(df[c], errors="coerce")

    if include_invalid:
        x = df[FEATURES].replace([np.inf, -np.inf], np.nan)
        df = df.loc[x.notna().all(axis=1)].copy()
    else:
        if "rf_train_row" in df.columns:
            df = df[df["rf_train_row"] == 1].copy()
        elif "feat_valid" in df.columns:
            curv = pd.to_numeric(
                df.get("current_valid", 1),
                errors="coerce",
            ).fillna(1)
            df = df[
                (pd.to_numeric(df["feat_valid"], errors="coerce") == 1)
                & (curv == 1)
            ].copy()

        x = df[FEATURES].replace([np.inf, -np.inf], np.nan)
        df = df.loc[x.notna().all(axis=1)].copy()

    clip_hi = {
        "cycle_nmse": 2.0,
        "zcv": 10.0,
        "zc_dwell_ratio": 1.0,
        "cycle_rms_drop_ratio": 1.0,
        "peak_fluct_cv": 2.5,
        "midband_residual_rms": 5.0,
        "hf_band_energy_ratio": 1.0,
        "spec_entropy": 1.0,
        "neg_dip_event_ratio": 1.0,
        "irms_drop_vs_baseline": 2.0,
    }
    for c, hi in clip_hi.items():
        df[c] = df[c].clip(0.0, hi)

    if "sample_weight" in df.columns:
        df["sample_weight"] = pd.to_numeric(
            df["sample_weight"],
            errors="coerce",
        ).fillna(1.0)
        df["sample_weight"] = df["sample_weight"].clip(0.05, 100.0)
    else:
        df["sample_weight"] = 1.0

    df[TARGET] = df[TARGET].astype(int)
    return df


def select_threshold_cost(
    y_true: np.ndarray,
    y_proba: np.ndarray,
    fn_weight: float = 80.0,
    fp_weight: float = 4.0,
    min_recall: float = 0.97,
    min_precision: float = 0.0,
    max_fpr: float = 1.0,
    min_threshold: float = 0.05,
) -> dict:
    start_thr = max(0.01, float(min_threshold))
    thresholds = np.arange(start_thr, 0.96, 0.01)
    best = None

    y_true = np.asarray(y_true).astype(int)
    y_proba = np.asarray(y_proba).astype(float)

    def _pack(thr, tn, fp, fn, tp, selection_reason):
        recall = tp / max(1, tp + fn)
        precision = tp / max(1, tp + fp)
        specificity = tn / max(1, tn + fp)
        fpr = fp / max(1, fp + tn)
        threshold_ok = float(thr) >= float(min_threshold)
        recall_ok = recall >= float(min_recall)
        precision_ok = precision >= float(min_precision)
        fpr_ok = fpr <= float(max_fpr)
        return {
            "thr": float(thr),
            "cost": float(fn_weight * fn + fp_weight * fp),
            "tn": int(tn),
            "fp": int(fp),
            "fn": int(fn),
            "tp": int(tp),
            "recall": float(recall),
            "precision": float(precision),
            "specificity": float(specificity),
            "fpr": float(fpr),
            "threshold_ok": bool(threshold_ok),
            "recall_ok": bool(recall_ok),
            "precision_ok": bool(precision_ok),
            "fpr_ok": bool(fpr_ok),
            "constraints_met": bool(threshold_ok and recall_ok and precision_ok and fpr_ok),
            "selection_reason": selection_reason,
        }

    feasible = []
    fallback = []
    for thr in thresholds:
        y_pred = (y_proba >= thr).astype(int)
        tn, fp, fn, tp = confusion_matrix(
            y_true,
            y_pred,
            labels=[0, 1],
        ).ravel()
        row = _pack(thr, tn, fp, fn, tp, "constraint_satisfying_threshold")
        if row["constraints_met"]:
            feasible.append(row)
        miss_recall = max(0.0, float(min_recall) - row["recall"])
        miss_precision = max(0.0, float(min_precision) - row["precision"])
        over_fpr = max(0.0, row["fpr"] - float(max_fpr))
        penalty = (
            row["cost"]
            + (miss_recall * max(1.0, float(fn_weight)) * 1000.0)
            + (miss_precision * max(1.0, float(fp_weight)) * 1000.0)
            + (over_fpr * max(1.0, float(fp_weight)) * 1000.0)
        )
        row["fallback_penalty"] = float(penalty)
        fallback.append(row)

    if feasible:
        best = sorted(
            feasible,
            key=lambda r: (
                r["cost"],
                r["fn"],
                r["fp"],
                -r["precision"],
                -r["recall"],
                -r["thr"],
            ),
        )[0]
        best["selection_reason"] = "constraint_satisfying_threshold"
        return best

    if fallback:
        best = sorted(
            fallback,
            key=lambda r: (
                r["fallback_penalty"],
                r["fn"],
                r["fp"],
                -r["precision"],
                -r["recall"],
                -r["thr"],
            ),
        )[0]
        best["selection_reason"] = "best_fallback_under_constraints"
        return best

    return {
        "thr": max(0.5, float(min_threshold)),
        "cost": float("inf"),
        "tn": 0,
        "fp": 0,
        "fn": 0,
        "tp": 0,
        "recall": 0.0,
        "precision": 0.0,
        "specificity": 0.0,
        "fpr": 1.0,
        "threshold_ok": True,
        "recall_ok": False,
        "precision_ok": False,
        "fpr_ok": False,
        "constraints_met": False,
        "selection_reason": "no_thresholds_available",
    }


def model_size_estimate(model) -> int:
    estimators = getattr(model, "estimators_", None)
    if estimators is None:
        return 0

    flat = []
    if isinstance(estimators, np.ndarray):
        flat = list(estimators.ravel())
    elif isinstance(estimators, (list, tuple)):
        for item in estimators:
            if isinstance(item, (list, tuple, np.ndarray)):
                flat.extend(list(np.asarray(item, dtype=object).ravel()))
            else:
                flat.append(item)
    else:
        flat = [estimators]

    total = 0
    for est in flat:
        tree = getattr(est, "tree_", None)
        if tree is not None and hasattr(tree, "node_count"):
            total += int(tree.node_count)
    return int(total)




def _safe_float(value, default: float = 0.0) -> float:
    try:
        out = float(value)
        if np.isnan(out) or np.isinf(out):
            return float(default)
        return out
    except Exception:
        return float(default)



def _safe_rate(num: float, den: float) -> float:
    return float(num) / max(1.0, float(den))



def _safe_average_precision(y_true, y_score) -> float:
    y_true = np.asarray(y_true).astype(int)
    y_score = np.asarray(y_score).astype(float)
    if y_true.size == 0 or np.unique(y_true).size < 2:
        return 0.0
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        try:
            out = float(average_precision_score(y_true, y_score))
            if np.isnan(out) or np.isinf(out):
                return 0.0
            return out
        except Exception:
            return 0.0


def _safe_roc_auc(y_true, y_score) -> float:
    y_true = np.asarray(y_true).astype(int)
    y_score = np.asarray(y_score).astype(float)
    if y_true.size == 0 or np.unique(y_true).size < 2:
        return 0.5
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        try:
            out = float(roc_auc_score(y_true, y_score))
            if np.isnan(out) or np.isinf(out):
                return 0.5
            return out
        except Exception:
            return 0.5


def summarize_scores(y_score: np.ndarray) -> dict:
    y_score = np.asarray(y_score).astype(float)
    if y_score.size == 0:
        return {
            "min": 0.0,
            "max": 0.0,
            "mean": 0.0,
            "median": 0.0,
            "std": 0.0,
        }
    return {
        "min": float(np.min(y_score)),
        "max": float(np.max(y_score)),
        "mean": float(np.mean(y_score)),
        "median": float(np.median(y_score)),
        "std": float(np.std(y_score)),
    }




def summarize_label_scores(y_true, y_score) -> dict:
    y_true = np.asarray(y_true).astype(int)
    y_score = np.asarray(y_score).astype(float)
    pos = y_score[y_true == 1]
    neg = y_score[y_true == 0]
    return {
        "mean_score": float(np.mean(y_score)) if y_score.size else 0.0,
        "median_score": float(np.median(y_score)) if y_score.size else 0.0,
        "min_score": float(np.min(y_score)) if y_score.size else 0.0,
        "max_score": float(np.max(y_score)) if y_score.size else 0.0,
        "std_score": float(np.std(y_score)) if y_score.size else 0.0,
        "mean_score_pos": float(np.mean(pos)) if pos.size else 0.0,
        "mean_score_neg": float(np.mean(neg)) if neg.size else 0.0,
    }

def evaluate_binary_scores(y_true, y_score, threshold: float) -> dict:
    y_true = np.asarray(y_true).astype(int)
    y_score = np.asarray(y_score).astype(float)
    y_pred = (y_score >= float(threshold)).astype(int)

    tn, fp, fn, tp = confusion_matrix(
        y_true,
        y_pred,
        labels=[0, 1],
    ).ravel()

    try:
        mcc = float(matthews_corrcoef(y_true, y_pred))
        if np.isnan(mcc) or np.isinf(mcc):
            mcc = 0.0
    except Exception:
        mcc = 0.0

    accuracy = _safe_rate(tp + tn, tp + tn + fp + fn)
    recall = _safe_rate(tp, tp + fn)
    specificity = _safe_rate(tn, tn + fp)
    balanced_accuracy = float((recall + specificity) * 0.5)

    out = {
        "threshold": float(threshold),
        "accuracy": float(accuracy),
        "balanced_accuracy": float(balanced_accuracy),
        "average_precision": _safe_average_precision(y_true, y_score),
        "roc_auc": _safe_roc_auc(y_true, y_score),
        "precision": float(precision_score(y_true, y_pred, zero_division=0)),
        "recall": float(recall),
        "f1": float(f1_score(y_true, y_pred, zero_division=0)),
        "specificity": float(specificity),
        "npv": _safe_rate(tn, tn + fn),
        "fpr": _safe_rate(fp, fp + tn),
        "fnr": _safe_rate(fn, fn + tp),
        "mcc": mcc,
        "confusion_matrix": {
            "tn": int(tn),
            "fp": int(fp),
            "fn": int(fn),
            "tp": int(tp),
        },
        "classification_report": classification_report(
            y_true,
            y_pred,
            digits=4,
            labels=[0, 1],
            output_dict=True,
            zero_division=0,
        ),
        "positive_count": int(np.sum(y_true == 1)),
        "negative_count": int(np.sum(y_true == 0)),
        "score_summary": summarize_scores(y_score),
        "label_score_summary": summarize_label_scores(y_true, y_score),
        "class_balance_ok": bool(np.unique(y_true).size >= 2),
    }
    return out


def _selection_policy_meta(
    winner_mode: str,
    fn_weight: float,
    fp_weight: float,
) -> dict:
    if winner_mode == "legacy_cv_ap":
        return {
            "mode": winner_mode,
            "description": (
                "Legacy rule: higher CV average precision, then fewer validation FP, "
                "then fewer validation FN, then smaller node count."
            ),
            "uses_test_metrics_for_selection": False,
            "tie_break_order": [
                "cv_best_average_precision",
                "validation_fp",
                "validation_fn",
                "estimated_node_count",
            ],
        }

    if winner_mode == "arc_guard":
        return {
            "mode": winner_mode,
            "description": (
                "Arc-first winner rule: prefer models that satisfy threshold constraints, "
                "then minimize validation false negatives, then minimize validation false positives, "
                "then maximize validation recall and precision."
            ),
            "uses_test_metrics_for_selection": False,
            "tie_break_order": [
                "threshold_constraints_met",
                "validation_fn",
                "validation_fp",
                "validation_recall",
                "validation_precision",
                "cv_best_average_precision",
                "estimated_node_count",
            ],
        }

    return {
        "mode": winner_mode,
        "description": (
            "Composite winner score using cross-validation AP plus validation-set "
            "average precision, recall, balanced accuracy, F1, specificity, ROC AUC, "
            "MCC, validation threshold cost penalty, and model size penalty. "
            "Test metrics are intentionally excluded from winner selection."
        ),
        "uses_test_metrics_for_selection": False,
        "weights": {
            "cv_best_average_precision": 0.18,
            "validation_average_precision": 0.20,
            "validation_recall": 0.20,
            "validation_balanced_accuracy": 0.15,
            "validation_f1": 0.10,
            "validation_specificity": 0.06,
            "validation_roc_auc": 0.04,
            "validation_mcc_norm": 0.03,
            "validation_cost_penalty": -0.03,
            "node_penalty": -0.01,
        },
        "cost_context": {
            "fn_weight": float(fn_weight),
            "fp_weight": float(fp_weight),
            "cost_unit": float(max(1.0, fn_weight + (5.0 * fp_weight))),
        },
        "tie_break_order": [
            "winner_score",
            "validation_recall",
            "validation_balanced_accuracy",
            "validation_average_precision",
            "cv_best_average_precision",
            "validation_specificity",
            "estimated_node_count",
        ],
    }



def _build_selection_meta(
    result: dict,
    winner_mode: str,
    fn_weight: float,
    fp_weight: float,
) -> dict:
    val = result.get("validation_metrics", {})

    if winner_mode == "legacy_cv_ap":
        score = _safe_float(result.get("cv_best_average_precision"))
        reason = (
            "Legacy winner rule selected this model by CV average precision first, "
            "with validation FP/FN and node count only as tie-breakers."
        )
        return {
            "winner_score": float(score),
            "winner_score_components": {
                "cv_best_average_precision": float(score),
            },
            "winner_reason": reason,
        }

    cv_ap = _safe_float(result.get("cv_best_average_precision"))
    val_ap = _safe_float(val.get("average_precision"))
    val_recall = _safe_float(val.get("recall"))
    val_precision = _safe_float(val.get("precision"))
    val_bal_acc = _safe_float(val.get("balanced_accuracy"))
    val_f1 = _safe_float(val.get("f1"))
    val_spec = _safe_float(val.get("specificity"))
    val_roc_auc = _safe_float(val.get("roc_auc"))
    val_mcc_norm = (_safe_float(val.get("mcc")) + 1.0) / 2.0
    val_cost = _safe_float(result.get("validation_threshold_result", {}).get("cost"))
    val_fn = _safe_float(result.get("validation_threshold_result", {}).get("fn"), 1e18)
    val_fp = _safe_float(result.get("validation_threshold_result", {}).get("fp"), 1e18)
    node_count = _safe_float(result.get("estimated_node_count"))
    constraints_met = bool(result.get("threshold_constraints_met", False))

    if winner_mode == "arc_guard":
        components = {
            "constraints_met": 4.0 * float(1.0 if constraints_met else 0.0),
            "zero_validation_fn": 3.0 * float(1.0 if val_fn == 0 else 0.0),
            "zero_validation_fp": 1.0 * float(1.0 if val_fp == 0 else 0.0),
            "validation_recall": 2.5 * val_recall,
            "validation_precision": 1.5 * val_precision,
            "validation_specificity": 1.0 * val_spec,
            "cv_best_average_precision": 0.5 * cv_ap,
            "validation_fn_penalty": -0.35 * min(20.0, val_fn),
            "validation_fp_penalty": -0.05 * min(50.0, val_fp),
            "node_penalty": -0.01 * min(1.0, node_count / 20000.0),
        }
        score = float(sum(components.values()))
        reason = (
            "Arc-first winner rule selected this model by prioritizing threshold-constraint "
            "compliance, zero or low validation false negatives, then low validation false positives."
        )
        return {
            "winner_score": score,
            "winner_score_components": components,
            "winner_reason": reason,
        }

    cost_unit = max(1.0, fn_weight + (5.0 * fp_weight))
    val_cost_penalty = float(np.tanh(val_cost / cost_unit))
    node_penalty = min(1.0, node_count / 20000.0)

    components = {
        "cv_best_average_precision": 0.18 * cv_ap,
        "validation_average_precision": 0.20 * val_ap,
        "validation_recall": 0.20 * val_recall,
        "validation_balanced_accuracy": 0.15 * val_bal_acc,
        "validation_f1": 0.10 * val_f1,
        "validation_specificity": 0.06 * val_spec,
        "validation_roc_auc": 0.04 * val_roc_auc,
        "validation_mcc_norm": 0.03 * val_mcc_norm,
        "validation_cost_penalty": -0.03 * val_cost_penalty,
        "node_penalty": -0.01 * node_penalty,
    }
    score = float(sum(components.values()))
    reason = (
        "Composite winner score selected this model using CV AP and validation metrics "
        "together. Test metrics were not used for model selection to avoid test leakage."
    )
    return {
        "winner_score": score,
        "winner_score_components": components,
        "winner_reason": reason,
    }


def postprocess_m2c_header(c_code: str) -> str:
    import re

    c_code = re.sub(
        r'(^|\n)void\s+add_vectors\s*\(\s*double\s*\*\s*v1\s*,\s*double\s*\*\s*v2\s*,\s*int\s+size\s*,\s*double\s*\*\s*result\s*\)\s*\{',
        r'\1static inline void add_vectors(double *v1, double *v2, int size, double *result) {',
        c_code,
        flags=re.MULTILINE,
    )

    c_code = re.sub(
        r'(^|\n)void\s+mul_vector_number\s*\(\s*double\s*\*\s*v1\s*,\s*double\s+num\s*,\s*int\s+size\s*,\s*double\s*\*\s*result\s*\)\s*\{',
        r'\1static inline void mul_vector_number(double *v1, double num, int size, double *result) {',
        c_code,
        flags=re.MULTILINE,
    )

    c_code = re.sub(
        r'(^|\n)void\s+arc_rf_predict\s*\(\s*double\s*\*\s*input\s*,\s*double\s*\*\s*output\s*\)\s*\{',
        r'\1static inline void set_output2(double *dst, double a, double b) {\n'
        r'    dst[0] = a;\n'
        r'    dst[1] = b;\n'
        r'}\n'
        r'static inline void arc_rf_predict(double * input, double * output) {',
        c_code,
        flags=re.MULTILINE,
    )

    memcpy_pat = re.compile(
        r'(?m)^(?P<indent>\s*)memcpy\(\s*(?P<dst>[A-Za-z_]\w*)\s*,\s*\(\s*double\s*\[\s*\]\s*\)\s*\{\s*(?P<a>[^,{}]+?)\s*,\s*(?P<b>[^{}]+?)\s*\}\s*,\s*2\s*\*\s*sizeof\s*\(\s*double\s*\)\s*\)\s*;\s*$'
    )

    def repl(match):
        return "%sset_output2(%s, %s, %s);" % (
            match.group("indent"),
            match.group("dst").strip(),
            match.group("a").strip(),
            match.group("b").strip(),
        )

    c_code = memcpy_pat.sub(repl, c_code)
    return c_code


def load_clean_dataset(csv_path: str, include_invalid: bool = False):
    if not os.path.isfile(csv_path):
        raise ValueError(f"Merged dataset not found: {csv_path}")

    raw_df = pd.read_csv(csv_path)
    raw_df = normalize_feature_names(raw_df)
    df = clean_df(raw_df, include_invalid=include_invalid)

    if df.empty or df[TARGET].nunique() < 2:
        raise ValueError("Training requires both classes in cleaned_data.csv.")

    group_col = pick_group_column(df)
    if group_col is None:
        df["_group"] = np.arange(len(df)) // 200
        group_col = "_group"

    groups = df[group_col].astype(str)
    X = df[FEATURES].astype(float)
    y = df[TARGET].astype(int)
    w = df["sample_weight"].astype(float).to_numpy()
    return df, X, y, groups, w


def _group_summary_frame(groups, y) -> pd.DataFrame:
    frame = pd.DataFrame({
        "group": pd.Series(groups).astype(str).to_numpy(),
        "label": pd.Series(y).astype(int).to_numpy(),
    })
    return frame.groupby("group", sort=False).agg(
        group_label=("label", "max"),
        row_count=("label", "size"),
        positive_rows=("label", "sum"),
    ).reset_index()


def _split_has_both_classes(summary: pd.DataFrame, selected_groups: set[str]) -> bool:
    if summary.empty or not selected_groups:
        return False
    labels = summary.loc[summary["group"].isin(selected_groups), "group_label"].astype(int)
    return labels.nunique() >= 2


def _class_balanced_group_kfold_splits(X, y, groups, n_splits: int):
    if StratifiedGroupKFold is not None:
        cv = StratifiedGroupKFold(n_splits=n_splits, shuffle=True, random_state=42)
        return list(cv.split(X, y, groups=groups))

    summary = _group_summary_frame(groups, y)
    pos = summary[summary["group_label"] == 1].sample(frac=1.0, random_state=42).reset_index(drop=True)
    neg = summary[summary["group_label"] == 0].sample(frac=1.0, random_state=43).reset_index(drop=True)
    folds = [{"groups": [], "rows": 0, "pos": 0, "neg": 0} for _ in range(n_splits)]

    def _place(group_row, is_pos: bool):
        target = min(
            range(n_splits),
            key=lambda i: (
                folds[i]["pos"] if is_pos else folds[i]["neg"],
                folds[i]["rows"],
                len(folds[i]["groups"]),
            ),
        )
        folds[target]["groups"].append(str(group_row["group"]))
        folds[target]["rows"] += int(group_row["row_count"])
        folds[target]["pos"] += int(is_pos)
        folds[target]["neg"] += int(not is_pos)

    for _, row in pos.iterrows():
        _place(row, True)
    for _, row in neg.iterrows():
        _place(row, False)

    all_groups = pd.Series(groups).astype(str).to_numpy()
    all_idx = np.arange(len(all_groups))
    out = []
    for fold in folds:
        val_groups = set(fold["groups"])
        val_mask = np.isin(all_groups, list(val_groups))
        val_idx = all_idx[val_mask]
        fit_idx = all_idx[~val_mask]
        if len(val_idx) and len(fit_idx):
            out.append((fit_idx, val_idx))
    return out


def _stratified_group_shuffle_indices(groups, y, test_size: float, random_state: int):
    summary = _group_summary_frame(groups, y)
    if len(summary) < 2:
        raise ValueError("Need at least 2 groups for grouped splitting.")

    pos_groups = int((summary["group_label"] == 1).sum())
    neg_groups = int((summary["group_label"] == 0).sum())
    y_arr = np.asarray(y).astype(int)
    groups_arr = pd.Series(groups).astype(str).to_numpy()
    all_idx = np.arange(len(groups_arr))

    if pos_groups >= 2 and neg_groups >= 2:
        splitter = StratifiedShuffleSplit(
            n_splits=24,
            test_size=test_size,
            random_state=random_state,
        )
        group_names = summary["group"].astype(str).to_numpy()
        group_labels = summary["group_label"].astype(int).to_numpy()
        for train_gi, test_gi in splitter.split(group_names, group_labels):
            train_groups = set(group_names[train_gi].tolist())
            test_groups = set(group_names[test_gi].tolist())
            if _split_has_both_classes(summary, train_groups) and _split_has_both_classes(summary, test_groups):
                train_mask = np.isin(groups_arr, list(train_groups))
                test_mask = np.isin(groups_arr, list(test_groups))
                return all_idx[train_mask], all_idx[test_mask]

    gss = GroupShuffleSplit(n_splits=24, test_size=test_size, random_state=random_state)
    best = None
    best_penalty = None
    for train_idx, test_idx in gss.split(X=np.zeros(len(groups_arr)), y=y_arr, groups=groups_arr):
        penalty = abs(np.unique(y_arr[train_idx]).size - 2) + abs(np.unique(y_arr[test_idx]).size - 2)
        if best is None or penalty < best_penalty:
            best = (train_idx, test_idx)
            best_penalty = penalty
            if penalty == 0:
                break
    if best is None:
        raise ValueError("Failed to build grouped train/test split.")
    return best


def make_group_splits(X, y, groups, test_size=0.20):
    train_full_idx, test_idx = _stratified_group_shuffle_indices(
        groups=groups,
        y=y,
        test_size=test_size,
        random_state=42,
    )

    X_train_full = X.iloc[train_full_idx]
    y_train_full = y.iloc[train_full_idx]
    groups_train_full = groups.iloc[train_full_idx]

    X_test = X.iloc[test_idx]
    y_test = y.iloc[test_idx]

    train_idx, val_idx = _stratified_group_shuffle_indices(
        groups=groups_train_full,
        y=y_train_full,
        test_size=0.20,
        random_state=123,
    )

    train_summary = _group_summary_frame(groups_train_full, y_train_full)
    pos_group_count = int((train_summary["group_label"] == 1).sum())
    neg_group_count = int((train_summary["group_label"] == 0).sum())
    recommended_cv_splits = int(min(5, len(train_summary), pos_group_count, neg_group_count)) if pos_group_count > 0 and neg_group_count > 0 else 0

    return {
        "train_full_idx": train_full_idx,
        "test_idx": test_idx,
        "train_idx": train_idx,
        "val_idx": val_idx,
        "X_train_full": X_train_full,
        "y_train_full": y_train_full,
        "groups_train_full": groups_train_full,
        "X_test": X_test,
        "y_test": y_test,
        "cv_group_summary": {
            "train_group_count": int(len(train_summary)),
            "positive_group_count": pos_group_count,
            "negative_group_count": neg_group_count,
            "recommended_cv_splits": int(recommended_cv_splits),
            "stratified_group_cv": bool(StratifiedGroupKFold is not None and recommended_cv_splits >= 2),
        },
    }


def estimate_search_plan(n_iter: int) -> dict:
    base = max(24, int(n_iter))
    refine = max(18, int(round(base * 0.90)))
    finalists = min(8, max(3, int(round(np.sqrt(base)))))
    robustness_seeds = [42, 314, 2024, 7331]
    if base >= 96:
        robustness_seeds.append(9001)
    total_steps = base + refine + (finalists * len(robustness_seeds)) + 1
    return {
        "broad_candidates": int(base),
        "refine_candidates": int(refine),
        "robust_finalists": int(finalists),
        "robustness_seeds": robustness_seeds,
        "progress_steps": int(total_steps),
    }


def _coerce_jsonable(value):
    if isinstance(value, (np.integer,)):
        return int(value)
    if isinstance(value, (np.floating,)):
        return float(value)
    return value


def _param_signature(params: dict) -> tuple:
    return tuple(sorted((str(k), repr(_coerce_jsonable(v))) for k, v in params.items()))


def _sample_param_set(space: dict, rng: np.random.Generator) -> dict:
    params = {}
    for name, values in space.items():
        vals = list(values)
        params[name] = vals[int(rng.integers(0, len(vals)))]
    return params


def _dedupe_candidates(candidates: list[dict]) -> list[dict]:
    seen = set()
    out = []
    for params in candidates:
        sig = _param_signature(params)
        if sig in seen:
            continue
        seen.add(sig)
        out.append(params)
    return out


def _build_broad_candidates(space: dict, count: int, seed: int = 42) -> list[dict]:
    rng = np.random.default_rng(seed)
    candidates = []
    attempts = 0
    max_attempts = max(200, count * 50)
    while len(candidates) < count and attempts < max_attempts:
        attempts += 1
        candidates.append(_sample_param_set(space, rng))
        candidates = _dedupe_candidates(candidates)
    return candidates[:count]


def _local_candidate_values(values: list, base_value):
    vals = list(values)
    if len(vals) <= 1:
        return vals
    if base_value in vals:
        i = vals.index(base_value)
        lo = max(0, i - 2)
        hi = min(len(vals), i + 3)
        window = vals[lo:hi]
        if base_value not in window:
            window.insert(0, base_value)
        return window
    return vals


def _build_refine_candidates(space: dict, seeds: list[dict], count: int, seed: int = 123) -> list[dict]:
    rng = np.random.default_rng(seed)
    if not seeds:
        return _build_broad_candidates(space, count=count, seed=seed)

    candidates = []
    attempts = 0
    max_attempts = max(400, count * 60)
    while len(candidates) < count and attempts < max_attempts:
        attempts += 1
        seed_params = seeds[int(rng.integers(0, len(seeds)))]
        params = {}
        for name, values in space.items():
            values = list(values)
            base_value = seed_params.get(name, values[0])
            if rng.random() < 0.68:
                pool = _local_candidate_values(values, base_value)
            else:
                pool = values
            params[name] = pool[int(rng.integers(0, len(pool)))]
        candidates.append(params)
        candidates = _dedupe_candidates(candidates)
    return candidates[:count]


def _fit_estimator(builder, params: dict, random_state_override: int | None = None):
    est = clone(builder())
    est.set_params(**params)
    if random_state_override is not None:
        try:
            est.set_params(random_state=int(random_state_override))
        except Exception:
            pass
    return est


def _cv_candidate_metrics(
    builder,
    params: dict,
    X_train,
    y_train,
    groups_train,
    w_train,
    n_splits: int,
    random_state_override: int | None = None,
    fn_weight: float = 80.0,
    fp_weight: float = 4.0,
    min_recall: float = 0.97,
    min_precision: float = 0.0,
    max_fpr: float = 1.0,
    min_threshold: float = 0.05,
) -> dict:
    fold_splits = _class_balanced_group_kfold_splits(
        X_train,
        y_train,
        groups_train,
        n_splits=n_splits,
    )
    ap_scores = []
    roc_scores = []
    bal_scores = []
    recall_scores = []
    precision_scores = []
    f1_scores = []
    specificity_scores = []
    fpr_scores = []
    threshold_scores = []
    cost_scores = []
    constraint_scores = []
    missing_class_folds = 0

    y_train_np = np.asarray(y_train).astype(int)
    w_train_np = np.asarray(w_train).astype(float)

    for fold_idx, (fit_idx, val_idx) in enumerate(fold_splits, start=1):
        est = _fit_estimator(builder, params, random_state_override=random_state_override)
        try:
            est.fit(
                X_train.iloc[fit_idx],
                y_train.iloc[fit_idx],
                sample_weight=w_train_np[fit_idx],
            )
            y_val_fold = y_train_np[val_idx]
            proba = est.predict_proba(X_train.iloc[val_idx])[:, 1]
            threshold_result = select_threshold_cost(
                y_val_fold,
                proba,
                fn_weight=fn_weight,
                fp_weight=fp_weight,
                min_recall=min_recall,
                min_precision=min_precision,
                max_fpr=max_fpr,
                min_threshold=min_threshold,
            )
            metrics = evaluate_binary_scores(y_val_fold, proba, threshold_result["thr"])
            if np.unique(y_val_fold).size < 2:
                missing_class_folds += 1
        except Exception as exc:
            return {
                "params": dict(params),
                "folds": int(fold_idx - 1),
                "cv_average_precision": 0.0,
                "cv_average_precision_std": 1.0,
                "cv_roc_auc": 0.5,
                "cv_balanced_accuracy": 0.0,
                "cv_recall": 0.0,
                "cv_precision": 0.0,
                "cv_specificity": 0.0,
                "cv_fpr": 1.0,
                "cv_threshold_mean": float(min_threshold),
                "cv_threshold_cost": float("inf"),
                "cv_constraints_met_rate": 0.0,
                "cv_f1": 0.0,
                "missing_class_folds": int(missing_class_folds),
                "missing_class_rate": 1.0,
                "search_score": -1.0,
                "cv_error": str(exc),
            }

        ap_scores.append(_safe_float(metrics["average_precision"]))
        roc_scores.append(_safe_float(metrics["roc_auc"], 0.5))
        bal_scores.append(_safe_float(metrics["balanced_accuracy"]))
        recall_scores.append(_safe_float(metrics["recall"]))
        precision_scores.append(_safe_float(metrics["precision"]))
        f1_scores.append(_safe_float(metrics["f1"]))
        specificity_scores.append(_safe_float(metrics["specificity"]))
        fpr_scores.append(_safe_float(metrics["fpr"]))
        threshold_scores.append(_safe_float(threshold_result.get("thr"), min_threshold))
        cost_scores.append(_safe_float(threshold_result.get("cost"), 1e18))
        constraint_scores.append(1.0 if threshold_result.get("constraints_met", False) else 0.0)

    ap_mean = float(np.mean(ap_scores)) if ap_scores else 0.0
    ap_std = float(np.std(ap_scores)) if ap_scores else 0.0
    roc_mean = float(np.mean(roc_scores)) if roc_scores else 0.5
    bal_mean = float(np.mean(bal_scores)) if bal_scores else 0.0
    recall_mean = float(np.mean(recall_scores)) if recall_scores else 0.0
    precision_mean = float(np.mean(precision_scores)) if precision_scores else 0.0
    f1_mean = float(np.mean(f1_scores)) if f1_scores else 0.0
    specificity_mean = float(np.mean(specificity_scores)) if specificity_scores else 0.0
    fpr_mean = float(np.mean(fpr_scores)) if fpr_scores else 1.0
    threshold_mean = float(np.mean(threshold_scores)) if threshold_scores else float(min_threshold)
    raw_cost_mean = float(np.mean(cost_scores)) if cost_scores else 1e18
    constraints_met_rate = float(np.mean(constraint_scores)) if constraint_scores else 0.0
    missing_class_rate = float(missing_class_folds / max(1, len(ap_scores)))

    cost_unit = max(1.0, float(fn_weight) + (5.0 * float(fp_weight)))
    cost_penalty = float(np.tanh(raw_cost_mean / cost_unit)) if np.isfinite(raw_cost_mean) else 1.0
    fpr_penalty = max(0.0, fpr_mean - float(max_fpr))
    recall_shortfall = max(0.0, float(min_recall) - recall_mean)
    precision_shortfall = max(0.0, float(min_precision) - precision_mean)

    search_score = (
        (0.22 * ap_mean)
        + (0.22 * recall_mean)
        + (0.16 * precision_mean)
        + (0.14 * bal_mean)
        + (0.08 * specificity_mean)
        + (0.06 * f1_mean)
        + (0.05 * roc_mean)
        + (0.15 * constraints_met_rate)
        - (0.04 * ap_std)
        - (0.04 * cost_penalty)
        - (0.10 * fpr_penalty)
        - (0.12 * recall_shortfall)
        - (0.08 * precision_shortfall)
        - (0.10 * missing_class_rate)
    )

    return {
        "params": dict(params),
        "folds": int(len(ap_scores)),
        "cv_average_precision": float(ap_mean),
        "cv_average_precision_std": float(ap_std),
        "cv_roc_auc": float(roc_mean),
        "cv_balanced_accuracy": float(bal_mean),
        "cv_recall": float(recall_mean),
        "cv_precision": float(precision_mean),
        "cv_specificity": float(specificity_mean),
        "cv_fpr": float(fpr_mean),
        "cv_threshold_mean": float(threshold_mean),
        "cv_threshold_cost": float(raw_cost_mean),
        "cv_constraints_met_rate": float(constraints_met_rate),
        "cv_f1": float(f1_mean),
        "missing_class_folds": int(missing_class_folds),
        "missing_class_rate": float(missing_class_rate),
        "search_score": float(search_score),
    }


def _report_search_progress(progress_callback, current: int, total: int, payload: dict) -> None:
    if progress_callback is None:
        return
    progress_callback(int(current), int(total), payload)




def train_one_model(
    model_key: str,
    X_train,
    y_train,
    groups_train,
    w_train,
    X_val,
    y_val,
    X_test,
    y_test,
    fn_weight: float,
    fp_weight: float,
    min_recall: float,
    n_iter: int,
    min_precision: float = 0.0,
    max_fpr: float = 1.0,
    min_threshold: float = 0.05,
    progress_callback=None,
) -> dict:
    if model_key not in MODEL_CONFIGS:
        raise ValueError(f"Unsupported model: {model_key}")

    cfg = MODEL_CONFIGS[model_key]
    pretty = cfg["pretty_name"]
    builder = cfg["builder"]
    param_space = cfg["param_space"]

    group_summary = _group_summary_frame(groups_train, y_train)
    pos_group_count = int((group_summary["group_label"] == 1).sum())
    neg_group_count = int((group_summary["group_label"] == 0).sum())
    n_splits = int(min(5, len(group_summary), pos_group_count, neg_group_count)) if pos_group_count > 0 and neg_group_count > 0 else 0
    if n_splits < 2:
        raise ValueError(
            "Need at least 2 positive sessions and 2 normal sessions for class-balanced grouped CV. "
            f"Found {pos_group_count} positive groups and {neg_group_count} normal groups."
        )

    plan = estimate_search_plan(n_iter)
    total_steps = plan["progress_steps"]
    broad_candidates = _build_broad_candidates(
        param_space,
        count=plan["broad_candidates"],
        seed=42,
    )
    search_rows = []
    progress_step = 0

    _report_search_progress(
        progress_callback,
        progress_step,
        total_steps,
        {
            "model_key": model_key,
            "model_name": pretty,
            "stage": "search_init",
            "message": f"Preparing {plan['broad_candidates']} broad candidates",
        },
    )

    for idx, params in enumerate(broad_candidates, start=1):
        row = _cv_candidate_metrics(
            builder=builder,
            params=params,
            X_train=X_train,
            y_train=y_train,
            groups_train=groups_train,
            w_train=w_train,
            n_splits=n_splits,
            fn_weight=fn_weight,
            fp_weight=fp_weight,
            min_recall=min_recall,
            min_precision=min_precision,
            max_fpr=max_fpr,
            min_threshold=min_threshold,
        )
        row["search_stage"] = "broad"
        row["candidate_index"] = int(idx)
        search_rows.append(row)
        progress_step += 1
        _report_search_progress(
            progress_callback,
            progress_step,
            total_steps,
            {
                "model_key": model_key,
                "model_name": pretty,
                "stage": "broad_search",
                "message": (
                    f"Broad {idx}/{plan['broad_candidates']} | "
                    f"AP={row['cv_average_precision']:.4f} | Recall={row.get('cv_recall', 0.0):.4f} | "
                    f"Score={row['search_score']:.4f} | Constraints={row.get('cv_constraints_met_rate', 0.0):.2f}"
                ),
                "cv_average_precision": row["cv_average_precision"],
                "search_score": row["search_score"],
                "missing_class_folds": int(row.get("missing_class_folds", 0)),
                "cv_constraints_met_rate": float(row.get("cv_constraints_met_rate", 0.0)),
            },
        )

    prelim = sorted(
        search_rows,
        key=lambda r: (
            -_safe_float(r.get("search_score")),
            -_safe_float(r.get("cv_average_precision")),
            _safe_float(r.get("cv_average_precision_std"), 1e18),
        ),
    )
    seed_params = [r["params"] for r in prelim[: max(3, plan["robust_finalists"])]]
    refine_candidates = _build_refine_candidates(
        param_space,
        seeds=seed_params,
        count=plan["refine_candidates"],
        seed=123,
    )

    used_signatures = {_param_signature(r["params"]) for r in search_rows}
    kept_refine = []
    for params in refine_candidates:
        sig = _param_signature(params)
        if sig in used_signatures:
            continue
        used_signatures.add(sig)
        kept_refine.append(params)

    if len(kept_refine) < plan["refine_candidates"]:
        extras = _build_broad_candidates(
            param_space,
            count=plan["refine_candidates"] * 2,
            seed=777,
        )
        for params in extras:
            sig = _param_signature(params)
            if sig in used_signatures:
                continue
            used_signatures.add(sig)
            kept_refine.append(params)
            if len(kept_refine) >= plan["refine_candidates"]:
                break

    kept_refine = kept_refine[: plan["refine_candidates"]]

    for idx, params in enumerate(kept_refine, start=1):
        row = _cv_candidate_metrics(
            builder=builder,
            params=params,
            X_train=X_train,
            y_train=y_train,
            groups_train=groups_train,
            w_train=w_train,
            n_splits=n_splits,
            fn_weight=fn_weight,
            fp_weight=fp_weight,
            min_recall=min_recall,
            min_precision=min_precision,
            max_fpr=max_fpr,
            min_threshold=min_threshold,
        )
        row["search_stage"] = "refine"
        row["candidate_index"] = int(idx)
        search_rows.append(row)
        progress_step += 1
        _report_search_progress(
            progress_callback,
            progress_step,
            total_steps,
            {
                "model_key": model_key,
                "model_name": pretty,
                "stage": "refine_search",
                "message": (
                    f"Refine {idx}/{len(kept_refine)} | "
                    f"AP={row['cv_average_precision']:.4f} | Recall={row.get('cv_recall', 0.0):.4f} | "
                    f"Score={row['search_score']:.4f} | Constraints={row.get('cv_constraints_met_rate', 0.0):.2f}"
                ),
                "cv_average_precision": row["cv_average_precision"],
                "search_score": row["search_score"],
                "missing_class_folds": int(row.get("missing_class_folds", 0)),
                "cv_constraints_met_rate": float(row.get("cv_constraints_met_rate", 0.0)),
            },
        )

    ranked_search = sorted(
        search_rows,
        key=lambda r: (
            -_safe_float(r.get("search_score")),
            -_safe_float(r.get("cv_average_precision")),
            _safe_float(r.get("cv_average_precision_std"), 1e18),
        ),
    )

    finalists = ranked_search[: plan["robust_finalists"]]
    robust_rows = []
    for finalist_idx, row in enumerate(finalists, start=1):
        seed_rows = []
        for seed in plan["robustness_seeds"]:
            robust = _cv_candidate_metrics(
                builder=builder,
                params=row["params"],
                X_train=X_train,
                y_train=y_train,
                groups_train=groups_train,
                w_train=w_train,
                n_splits=n_splits,
                random_state_override=seed,
                fn_weight=fn_weight,
                fp_weight=fp_weight,
                min_recall=min_recall,
                min_precision=min_precision,
                max_fpr=max_fpr,
                min_threshold=min_threshold,
            )
            seed_rows.append(robust)
            progress_step += 1
            _report_search_progress(
                progress_callback,
                progress_step,
                total_steps,
                {
                    "model_key": model_key,
                    "model_name": pretty,
                    "stage": "robustness_check",
                    "message": (
                        f"Stability {finalist_idx}/{len(finalists)} | seed={seed} | "
                        f"AP={robust['cv_average_precision']:.4f} | Recall={robust.get('cv_recall', 0.0):.4f} | "
                        f"Score={robust['search_score']:.4f}"
                    ),
                    "cv_average_precision": robust["cv_average_precision"],
                    "search_score": robust["search_score"],
                    "missing_class_folds": int(robust.get("missing_class_folds", 0)),
                },
            )

        robust_score = float(np.mean([r["search_score"] for r in seed_rows])) if seed_rows else -1.0
        robust_ap = float(np.mean([r["cv_average_precision"] for r in seed_rows])) if seed_rows else 0.0
        robust_ap_std = float(np.std([r["cv_average_precision"] for r in seed_rows])) if seed_rows else 1.0
        robust_rows.append({
            "params": dict(row["params"]),
            "robust_search_score": robust_score,
            "robust_cv_average_precision": robust_ap,
            "robust_cv_average_precision_std": robust_ap_std,
            "robust_cv_recall": float(np.mean([r.get("cv_recall", 0.0) for r in seed_rows])) if seed_rows else 0.0,
            "robust_cv_precision": float(np.mean([r.get("cv_precision", 0.0) for r in seed_rows])) if seed_rows else 0.0,
            "robust_cv_constraints_met_rate": float(np.mean([r.get("cv_constraints_met_rate", 0.0) for r in seed_rows])) if seed_rows else 0.0,
            "base_search_score": row["search_score"],
            "base_cv_average_precision": row["cv_average_precision"],
            "base_cv_recall": row.get("cv_recall", 0.0),
            "base_cv_precision": row.get("cv_precision", 0.0),
            "base_cv_constraints_met_rate": row.get("cv_constraints_met_rate", 0.0),
            "seed_breakdown": seed_rows,
        })

    robust_ranked = sorted(
        robust_rows,
        key=lambda r: (
            -_safe_float(r.get("robust_search_score")),
            -_safe_float(r.get("robust_cv_average_precision")),
            _safe_float(r.get("robust_cv_average_precision_std"), 1e18),
        ),
    )

    if robust_ranked:
        final_choice = robust_ranked[0]
        best_params = dict(final_choice["params"])
        search_meta = {
            "strategy": "broad_random_then_local_refine_then_seed_robustness",
            "plan": plan,
            "best_search_score": float(final_choice["robust_search_score"]),
            "best_cv_average_precision": float(final_choice["robust_cv_average_precision"]),
            "best_cv_average_precision_std": float(final_choice["robust_cv_average_precision_std"]),
            "top_candidates": robust_ranked,
            "evaluated_candidates": ranked_search,
        }
    else:
        best_params = dict(ranked_search[0]["params"])
        search_meta = {
            "strategy": "broad_random_then_local_refine",
            "plan": plan,
            "best_search_score": float(ranked_search[0]["search_score"]),
            "best_cv_average_precision": float(ranked_search[0]["cv_average_precision"]),
            "best_cv_average_precision_std": float(ranked_search[0]["cv_average_precision_std"]),
            "top_candidates": ranked_search[: plan["robust_finalists"]],
            "evaluated_candidates": ranked_search,
        }

    progress_step = total_steps - 1
    _report_search_progress(
        progress_callback,
        progress_step,
        total_steps,
        {
            "model_key": model_key,
            "model_name": pretty,
            "stage": "final_fit",
            "message": "Fitting final model and validating threshold",
        },
    )

    best = _fit_estimator(builder, best_params, random_state_override=42)
    best.fit(X_train, y_train, sample_weight=np.asarray(w_train).astype(float))
    val_proba = best.predict_proba(X_val)[:, 1]
    best_thr = select_threshold_cost(
        y_val.to_numpy(),
        val_proba,
        fn_weight=fn_weight,
        fp_weight=fp_weight,
        min_recall=min_recall,
        min_precision=min_precision,
        max_fpr=max_fpr,
        min_threshold=min_threshold,
    )
    thr = float(best_thr["thr"])

    val_metrics = evaluate_binary_scores(y_val.to_numpy(), val_proba, thr)
    y_score = best.predict_proba(X_test)[:, 1]
    test_metrics = evaluate_binary_scores(y_test, y_score, thr)

    fi = pd.Series(
        best.feature_importances_,
        index=FEATURES,
    ).sort_values(ascending=False)

    progress_step = total_steps
    _report_search_progress(
        progress_callback,
        progress_step,
        total_steps,
        {
            "model_key": model_key,
            "model_name": pretty,
            "stage": "done",
            "message": f"Done | CV AP={search_meta['best_cv_average_precision']:.4f} | Test AP={test_metrics['average_precision']:.4f}",
            "cv_average_precision": search_meta["best_cv_average_precision"],
            "test_average_precision": test_metrics["average_precision"],
        },
    )

    result = {
        "model_key": model_key,
        "model_name": pretty,
        "best_params": best_params,
        "cv_best_average_precision": float(search_meta["best_cv_average_precision"]),
        "cv_best_average_precision_std": float(search_meta.get("best_cv_average_precision_std", 0.0)),
        "cv_search_best_score": float(search_meta["best_search_score"]),
        "cv_best_recall": float(search_meta.get("top_candidates", [{}])[0].get("robust_cv_recall", search_meta.get("top_candidates", [{}])[0].get("cv_recall", 0.0)) if search_meta.get("top_candidates") else 0.0),
        "search_summary": search_meta,
        "estimated_node_count": model_size_estimate(best),
        "threshold": thr,
        "threshold_source": best_thr.get("selection_reason", "validation_cost"),
        "threshold_constraints_met": bool(best_thr.get("constraints_met", False)),
        "validation_threshold_result": best_thr,
        "validation_precision": float(val_metrics.get("precision", 0.0)),
        "holdout_validation_precision": float(val_metrics.get("precision", 0.0)),
        "holdout_validation_recall": float(val_metrics.get("recall", 0.0)),
        "validation_metrics": val_metrics,
        "validation_accuracy": val_metrics["accuracy"],
        "validation_average_precision": val_metrics["average_precision"],
        "validation_roc_auc": val_metrics["roc_auc"],
        "validation_precision": val_metrics["precision"],
        "validation_recall": val_metrics["recall"],
        "validation_f1": val_metrics["f1"],
        "validation_balanced_accuracy": val_metrics["balanced_accuracy"],
        "validation_specificity": val_metrics["specificity"],
        "validation_npv": val_metrics["npv"],
        "validation_fpr": val_metrics["fpr"],
        "validation_fnr": val_metrics["fnr"],
        "validation_mcc": val_metrics["mcc"],
        "validation_confusion_matrix": val_metrics["confusion_matrix"],
        "validation_classification_report": val_metrics["classification_report"],
        "test_metrics": test_metrics,
        "test_accuracy": test_metrics["accuracy"],
        "test_average_precision": test_metrics["average_precision"],
        "test_roc_auc": test_metrics["roc_auc"],
        "test_precision": test_metrics["precision"],
        "test_recall": test_metrics["recall"],
        "test_f1": test_metrics["f1"],
        "test_balanced_accuracy": test_metrics["balanced_accuracy"],
        "test_specificity": test_metrics["specificity"],
        "test_npv": test_metrics["npv"],
        "test_fpr": test_metrics["fpr"],
        "test_fnr": test_metrics["fnr"],
        "test_mcc": test_metrics["mcc"],
        "test_confusion_matrix": test_metrics["confusion_matrix"],
        "test_classification_report": test_metrics["classification_report"],
        "split_sizes": {
            "train_rows": int(len(X_train)),
            "validation_rows": int(len(X_val)),
            "val_rows": int(len(X_val)),
            "test_rows": int(len(X_test)),
            "train_groups": int(groups_train.nunique()),
            "cv_splits": int(n_splits),
            "positive_groups": int(pos_group_count),
            "negative_groups": int(neg_group_count),
            "train_positive": int(np.sum(np.asarray(y_train) == 1)),
            "validation_positive": int(np.sum(np.asarray(y_val) == 1)),
            "test_positive": int(np.sum(np.asarray(y_test) == 1)),
        },
        "feature_importances": {
            str(k): float(v) for k, v in fi.items()
        },
        "validation_score_summary": val_metrics["label_score_summary"],
        "test_score_summary": test_metrics["label_score_summary"],
        "score_summary": {
            "validation": val_metrics["score_summary"],
            "test": test_metrics["score_summary"],
        },
        "estimator": best,
    }
    return result


def strip_estimator(result: dict) -> dict:
    out = dict(result)
    out.pop("estimator", None)
    return out


def pick_winner(
    results: List[dict],
    winner_mode: str = "safety_composite",
    fn_weight: float = 80.0,
    fp_weight: float = 4.0,
):
    if not results:
        raise ValueError("No benchmark results provided.")

    policy = _selection_policy_meta(winner_mode, fn_weight, fp_weight)
    annotated = []
    for result in results:
        enriched = dict(result)
        enriched.update(
            _build_selection_meta(
                enriched,
                winner_mode=winner_mode,
                fn_weight=fn_weight,
                fp_weight=fp_weight,
            )
        )
        enriched["winner_mode"] = winner_mode
        annotated.append(enriched)

    if winner_mode == "legacy_cv_ap":
        ordered = sorted(
            annotated,
            key=lambda r: (
                -_safe_float(r.get("cv_best_average_precision")),
                _safe_float(r.get("validation_threshold_result", {}).get("fp"), 1e18),
                _safe_float(r.get("validation_threshold_result", {}).get("fn"), 1e18),
                _safe_float(r.get("estimated_node_count"), 1e18),
            ),
        )
    elif winner_mode == "arc_guard":
        ordered = sorted(
            annotated,
            key=lambda r: (
                -float(1.0 if r.get("threshold_constraints_met", False) else 0.0),
                _safe_float(r.get("validation_threshold_result", {}).get("fn"), 1e18),
                _safe_float(r.get("validation_threshold_result", {}).get("fp"), 1e18),
                -_safe_float(r.get("holdout_validation_recall", r.get("validation_recall"))),
                -_safe_float(r.get("holdout_validation_precision", r.get("validation_precision"))),
                -_safe_float(r.get("validation_specificity")),
                -_safe_float(r.get("cv_best_average_precision")),
                _safe_float(r.get("estimated_node_count"), 1e18),
            ),
        )
    else:
        ordered = sorted(
            annotated,
            key=lambda r: (
                -_safe_float(r.get("winner_score")),
                -_safe_float(r.get("validation_recall")),
                -_safe_float(r.get("validation_balanced_accuracy")),
                -_safe_float(r.get("validation_average_precision")),
                -_safe_float(r.get("cv_best_average_precision")),
                -_safe_float(r.get("validation_specificity")),
                _safe_float(r.get("estimated_node_count"), 1e18),
            ),
        )

    ranked = []
    for rank, result in enumerate(ordered, start=1):
        enriched = dict(result)
        enriched["winner_rank"] = int(rank)
        ranked.append(enriched)

    return ranked[0], ranked, policy


def export_header(
    model,
    threshold: float,
    out_header: str,
    model_name: str,
) -> None:
    ensure_dir(out_header)
    c_code = m2c.export_to_c(model, function_name="arc_rf_predict")
    c_code = postprocess_m2c_header(c_code)

    with open(out_header, "w", encoding="utf-8") as f:
        f.write("#pragma once\n")
        f.write(
            f"// Auto-generated by m2cgen from scikit-learn {model_name}\n"
        )
        f.write("#define ARC_MODEL_FEATURE_VERSION 2\n")
        f.write(f"#define ARC_THRESHOLD {threshold:.4f}\n\n")
        f.write("// Input Feature Order:\n")
        for i, name in enumerate(FEATURES):
            f.write(f"// [{i}] {name}\n")
        f.write("\n#include <string.h>\n")
        f.write(c_code)


def save_model_bundle(
    result: dict,
    out_header: str,
    out_joblib: str,
    out_report: str,
    settings: dict,
) -> None:
    ensure_dir(out_joblib)
    ensure_dir(out_report)

    joblib.dump(result["estimator"], out_joblib)
    export_header(
        model=result["estimator"],
        threshold=float(result["threshold"]),
        out_header=out_header,
        model_name=result["model_name"],
    )

    report = strip_estimator(result)
    report["settings"] = settings
    with open(out_report, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)


def save_duel_bundle(
    winner: dict,
    results: List[dict],
    out_header: str,
    out_joblib: str,
    out_report: str,
    settings: dict,
    winner_policy: dict | None = None,
) -> None:
    ensure_dir(out_joblib)
    ensure_dir(out_report)

    joblib.dump(winner["estimator"], out_joblib)
    export_header(
        model=winner["estimator"],
        threshold=float(winner["threshold"]),
        out_header=out_header,
        model_name=winner["model_name"],
    )

    report = {
        "winner": strip_estimator(winner),
        "all_results": [strip_estimator(r) for r in results],
        "winner_policy": winner_policy or {},
        "settings": settings,
    }
    with open(out_report, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)
