import argparse
import json
import os
import subprocess
import sys
import threading
from pathlib import Path

import joblib

from tinyml_common import (
    ARC_BASE_FEATURES_RANKED,
    ARC_CONTEXT_FEATURES,
    ARC_FEATURES,
    ARC_SWEEP_FEATURES,
    ensure_dir,
    estimate_search_plan,
    pick_winner,
    save_duel_bundle,
    strip_estimator,
    validate_feature_subset,
)
try:
    from generation_paths import (
        apply_generation_defaults,
        archive_generation_exports,
        cli_flag_present,
        ensure_generation_dirs,
        next_generation_tag,
        normalize_generation_tag,
        resolve_generation_paths,
        update_generation_manifest,
        validate_generation_workspace,
    )
except Exception:
    from trainer.generation_paths import (
        apply_generation_defaults,
        archive_generation_exports,
        cli_flag_present,
        ensure_generation_dirs,
        next_generation_tag,
        normalize_generation_tag,
        resolve_generation_paths,
        update_generation_manifest,
        validate_generation_workspace,
    )

SCRIPT_DIR = Path(__file__).resolve().parent
TINYML_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TINYML_DIR.parent

CSV_PATH = str(PROJECT_ROOT / "tinyml" / "data" / "arc_training.csv")
OUT_HEADER = str(PROJECT_ROOT / "tinyml" / "model" / "TinyMLTreeEnsemble.h")
OUT_JOBLIB = str(PROJECT_ROOT / "tinyml" / "trainer" / "lib" / "TinyMLTreeEnsemble.joblib")
OUT_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "benchmark_report.json")
RF_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLTreeEnsemble_RF_report.json")
ET_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLTreeEnsemble_ET_report.json")
RF_JOBLIB = str(PROJECT_ROOT / "tinyml" / "trainer" / "lib" / "TinyMLTreeEnsemble_RF.joblib")
ET_JOBLIB = str(PROJECT_ROOT / "tinyml" / "trainer" / "lib" / "TinyMLTreeEnsemble_ET.joblib")
RF_HEADER = str(PROJECT_ROOT / "tinyml" / "model" / "TinyMLTreeEnsemble_RF.h")
ET_HEADER = str(PROJECT_ROOT / "tinyml" / "model" / "TinyMLTreeEnsemble_ET.h")
DEFAULT_FEATURE_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLFeatureSubsetSweep_report.json")

CHILD_MODEL_CONFIG = {
    "rf": {
        "script": "train_rf.py",
        "report": RF_REPORT,
        "joblib": RF_JOBLIB,
        "header": RF_HEADER,
    },
    "et": {
        "script": "train_et.py",
        "report": ET_REPORT,
        "joblib": ET_JOBLIB,
        "header": ET_HEADER,
    },
}


def emit_progress(current: int, total: int, payload: dict) -> None:
    payload = dict(payload or {})
    payload["current"] = int(current)
    payload["total"] = int(total)
    print("[[PROGRESS]] " + json.dumps(payload, sort_keys=True), flush=True)


def _shared_arc_training_args(args) -> list[str]:
    out = [
        "--arc_tolerance_mode", str(args.arc_tolerance_mode),
        "--pre_arc_window", str(int(args.pre_arc_window)),
        "--post_arc_window", str(int(args.post_arc_window)),
        "--soft_neighbor_weight", str(float(args.soft_neighbor_weight)),
        "--expanded_neighbor_weight", str(float(args.expanded_neighbor_weight)),
        "--hard_negative_ring", str(int(args.hard_negative_ring)),
        "--final_negative_ratio", str(float(args.final_negative_ratio)),
        "--positive_oversample", str(float(args.positive_oversample)),
        "--mismatch_fp_boost", str(float(getattr(args, "mismatch_fp_boost", 1.0))),
        "--mismatch_fn_boost", str(float(getattr(args, "mismatch_fn_boost", 1.0))),
        "--mismatch_focus_ratio", str(float(getattr(args, "mismatch_focus_ratio", 0.0))),
    ]
    if bool(getattr(args, "mismatch_verified_only", False)):
        out.append("--mismatch_verified_only")
    return out


def _prefer_rf_deployment_winner(ranked_results: list[dict], winner_policy: dict) -> tuple[dict | None, dict]:
    rf = next((r for r in ranked_results if str(r.get("model_key", "")).lower() == "rf"), None)
    et = next((r for r in ranked_results if str(r.get("model_key", "")).lower() == "et"), None)
    if rf is None:
        return et, winner_policy
    if et is None:
        return rf, winner_policy

    rf_event_fn = int((rf.get("validation_event_level_metrics") or {}).get("missed_event_count", 10**9))
    et_event_fn = int((et.get("validation_event_level_metrics") or {}).get("missed_event_count", 10**9))
    rf_fn = int((rf.get("validation_threshold_result") or {}).get("fn", 10**9))
    et_fn = int((et.get("validation_threshold_result") or {}).get("fn", 10**9))
    rf_fp = int((rf.get("validation_threshold_result") or {}).get("fp", 10**9))
    et_fp = int((et.get("validation_threshold_result") or {}).get("fp", 10**9))
    rf_false_alarms = float((rf.get("validation_event_level_metrics") or {}).get("false_alarms_per_session", 10**9))
    et_false_alarms = float((et.get("validation_event_level_metrics") or {}).get("false_alarms_per_session", 10**9))
    rf_bal = float(rf.get("validation_balanced_accuracy", 0.0) or 0.0)
    et_bal = float(et.get("validation_balanced_accuracy", 0.0) or 0.0)

    et_clearly_better = (
        (et_event_fn + 1 < rf_event_fn)
        or (
            et_event_fn < rf_event_fn
            and et_fp <= (rf_fp + 3)
            and et_false_alarms <= (rf_false_alarms + 0.08)
        )
        or (
            et_event_fn == rf_event_fn
            and et_fn + 2 < rf_fn
            and et_fp <= (rf_fp + 2)
        )
        or (
            et_event_fn == rf_event_fn
            and et_fn == rf_fn
            and et_fp + 8 < rf_fp
            and et_false_alarms + 0.10 < rf_false_alarms
            and et_bal >= (rf_bal + 0.015)
        )
    )
    if et_clearly_better:
        return et, winner_policy

    policy = dict(winner_policy or {})
    policy["rf_deployment_preference_applied"] = True
    policy["rf_deployment_preference_reason"] = (
        "Random Forest kept as deployment winner unless Extra Trees shows a clearly better "
        "validation event-miss / false-positive tradeoff."
    )
    return rf, policy


def _load_feature_list_from_report(report_path: str | None, model_key: str) -> list[str] | None:
    path = Path(report_path or "")
    if not path.is_file():
        return None
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return None
    recommended_by_model = payload.get("recommended_by_model") or {}
    candidates = []
    if isinstance(recommended_by_model, dict):
        candidates = recommended_by_model.get(model_key) or []
    if not candidates:
        candidates = payload.get("recommended_arc_base_features_global") or []
    if not candidates:
        candidates = payload.get("recommended_features_global") or []
    if not candidates:
        best = payload.get("overall_best_combined_tradeoff") or {}
        candidates = best.get("features") or []
    out = [str(x).strip() for x in candidates if str(x).strip() in ARC_SWEEP_FEATURES]
    return out or None


def resolve_arc_features_by_model(args, model_key: str) -> tuple[list[str], list[str]]:
    explicit_attr = f"{model_key}_features"
    report_attr = f"{model_key}_feature_report"
    common_explicit = [str(x).strip() for x in (getattr(args, "features", None) or []) if str(x).strip() in ARC_SWEEP_FEATURES]
    explicit = [str(x).strip() for x in getattr(args, explicit_attr, []) if str(x).strip() in ARC_SWEEP_FEATURES]
    if explicit:
        base = validate_feature_subset(explicit, allowed_features=ARC_SWEEP_FEATURES, role=f"{model_key} arc training")
    elif common_explicit:
        base = validate_feature_subset(common_explicit, allowed_features=ARC_SWEEP_FEATURES, role=f"{model_key} arc training")
    else:
        report_path = getattr(args, report_attr, None) or args.feature_report
        allow_legacy_default_report = not bool(getattr(args, "_selected_generation", None) or getattr(args, "generation", None))
        default_report_features = _load_feature_list_from_report(DEFAULT_FEATURE_REPORT, model_key) if allow_legacy_default_report else None
        base = (
            _load_feature_list_from_report(report_path, model_key)
            or default_report_features
            or list(ARC_SWEEP_FEATURES)
        )
    return list(base), list(base) + list(ARC_CONTEXT_FEATURES)


def _per_model_n_jobs(model_count: int, override: int) -> int:
    if override is not None and int(override) != 0:
        return int(override)
    cpu_total = os.cpu_count() or 1
    if model_count <= 1:
        return -1
    return max(1, cpu_total // model_count)


def _build_child_command(model_key: str, args, n_jobs: int) -> list[str]:
    cfg = CHILD_MODEL_CONFIG[model_key]
    selected_base_features, selected_feature_names = resolve_arc_features_by_model(args, model_key)
    print(f"[{model_key}] Selected arc base features: {selected_base_features}")
    cmd = [
        sys.executable,
        str(SCRIPT_DIR / cfg["script"]),
        "--csv",
        args.csv,
        "--out_header",
        cfg["header"],
        "--out_joblib",
        cfg["joblib"],
        "--out_report",
        args.rf_out_report if model_key == "rf" else args.et_out_report,
        "--fn_weight",
        str(args.fn_weight),
        "--fp_weight",
        str(args.fp_weight),
        "--min_recall",
        str(args.min_recall),
        "--min_precision",
        str(args.min_precision),
        "--max_fpr",
        str(args.max_fpr),
        "--min_threshold",
        str(args.min_threshold),
        "--n_iter",
        str(args.n_iter),
        "--n_jobs",
        str(n_jobs),
    ]
    if getattr(args, "generation", None):
        cmd.extend(["--generation", str(args.generation)])
    if args.include_invalid:
        cmd.append("--include_invalid")
    if getattr(args, f"{model_key}_feature_report", None):
        cmd.extend(["--feature_report", str(getattr(args, f"{model_key}_feature_report"))])
    elif args.feature_report:
        cmd.extend(["--feature_report", str(args.feature_report)])
    if selected_base_features:
        cmd.extend(["--features", *selected_base_features])
    cmd.extend(_shared_arc_training_args(args))
    return cmd


def _rehydrate_result(report_path: str, joblib_path: str) -> dict:
    with open(report_path, "r", encoding="utf-8") as f:
        payload = json.load(f)
    payload["estimator"] = joblib.load(joblib_path)
    return payload


def _relay_progress(model_key: str, payload: dict, tracker: dict, model_count: int) -> None:
    state = tracker.setdefault(model_key, {"current": 0, "total": 1})
    try:
        state["current"] = int(payload.get("current", state["current"]))
        state["total"] = max(1, int(payload.get("total", state["total"])))
    except Exception:
        pass

    global_current = int(sum(v.get("current", 0) for v in tracker.values()))
    global_total = int(sum(max(1, v.get("total", 1)) for v in tracker.values()))
    relay = dict(payload or {})
    relay["current"] = global_current
    relay["total"] = global_total
    relay["model_key"] = model_key
    relay["model_count"] = int(model_count)
    relay["model_local_current"] = int(state.get("current", 0))
    relay["model_local_total"] = int(state.get("total", 1))
    emit_progress(global_current, global_total, relay)


def _stream_child(model_key: str, proc: subprocess.Popen, tracker: dict, model_count: int) -> None:
    prefix = "[[PROGRESS]] "
    assert proc.stdout is not None
    for line in proc.stdout:
        if line.startswith(prefix):
            try:
                payload = json.loads(line[len(prefix):].strip())
            except Exception:
                print(f"[{model_key}] {line}", end="", flush=True)
                continue
            _relay_progress(model_key, payload, tracker, model_count)
        else:
            print(f"[{model_key}] {line}", end="", flush=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default=CSV_PATH)
    ap.add_argument("--out_header", default=OUT_HEADER)
    ap.add_argument("--out_joblib", default=OUT_JOBLIB)
    ap.add_argument("--out_report", default=OUT_REPORT)
    ap.add_argument("--rf_out_report", default=RF_REPORT)
    ap.add_argument("--et_out_report", default=ET_REPORT)
    ap.add_argument("--generation", default=None)
    ap.add_argument("--include_invalid", action="store_true")
    ap.add_argument("--fn_weight", type=float, default=80.0)
    ap.add_argument("--fp_weight", type=float, default=4.0)
    ap.add_argument("--min_recall", type=float, default=0.97)
    ap.add_argument("--min_precision", type=float, default=0.90)
    ap.add_argument("--max_fpr", type=float, default=0.03)
    ap.add_argument("--min_threshold", type=float, default=0.08)
    ap.add_argument("--n_iter", type=int, default=60)
    ap.add_argument("--winner_mode", default="safety_composite", choices=["arc_guard", "safety_composite", "legacy_cv_ap"])
    ap.add_argument("--models", nargs="+", default=["rf", "et"], choices=["rf", "et"])
    ap.add_argument("--model_n_jobs", type=int, default=0)
    ap.add_argument("--serial_models", action="store_true")
    ap.add_argument("--feature_report", default=None)
    ap.add_argument("--features", nargs="*", default=None)
    ap.add_argument("--rf_feature_report", default=None)
    ap.add_argument("--et_feature_report", default=None)
    ap.add_argument("--rf_features", nargs="*", default=None)
    ap.add_argument("--et_features", nargs="*", default=None)
    ap.add_argument("--arc_tolerance_mode", default="soft_positive", choices=["none", "expanded_positive", "soft_positive"])
    ap.add_argument("--pre_arc_window", type=int, default=1)
    ap.add_argument("--post_arc_window", type=int, default=3)
    ap.add_argument("--soft_neighbor_weight", type=float, default=0.30)
    ap.add_argument("--expanded_neighbor_weight", type=float, default=0.70)
    ap.add_argument("--hard_negative_ring", type=int, default=2)
    ap.add_argument("--final_negative_ratio", type=float, default=0.0)
    ap.add_argument("--positive_oversample", type=float, default=1.0)
    ap.add_argument("--mismatch_fp_boost", type=float, default=1.0)
    ap.add_argument("--mismatch_fn_boost", type=float, default=1.0)
    ap.add_argument("--mismatch_focus_ratio", type=float, default=0.0)
    ap.add_argument("--mismatch_verified_only", action="store_true")
    args = ap.parse_args()

    argv = sys.argv[1:]
    generation_paths = None
    selected_generation = None
    if args.generation:
        selected_generation = normalize_generation_tag(args.generation)
        generation_paths = resolve_generation_paths(PROJECT_ROOT, selected_generation)
        ensure_generation_dirs(generation_paths)
        apply_generation_defaults(
            args,
            argv,
            {
                "csv": ("--csv", generation_paths.arc_csv),
                "out_report": ("--out_report", generation_paths.duel_report),
                "rf_out_report": ("--rf_out_report", generation_paths.rf_report),
                "et_out_report": ("--et_out_report", generation_paths.et_report),
                "feature_report": ("--feature_report", generation_paths.arc_subset_report),
            },
        )
        if not cli_flag_present("--csv", argv):
            errors = validate_generation_workspace(generation_paths, require_arc_csv=True, require_nonempty_arc_csv=True)
            if errors:
                raise ValueError(" ; ".join(errors))

    plan = estimate_search_plan(args.n_iter)
    print("Search plan per model:", plan)

    selected_models = list(dict.fromkeys(args.models))
    per_model_jobs = _per_model_n_jobs(len(selected_models), args.model_n_jobs)
    print("Running models:", selected_models)
    print("Per-model n_jobs:", per_model_jobs)

    tracker = {
        model_key: {"current": 0, "total": max(1, int(plan["progress_steps"]))}
        for model_key in selected_models
    }

    procs = {}
    threads = []
    if args.serial_models:
        for model_key in selected_models:
            cmd = _build_child_command(model_key, args, per_model_jobs)
            print(f"\n=== Benchmarking {model_key} (serial) ===")
            print("Command:", " ".join(cmd))
            proc = subprocess.Popen(
                cmd,
                cwd=str(PROJECT_ROOT),
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
            )
            procs[model_key] = proc
            _stream_child(model_key, proc, tracker, len(selected_models))
            rc = proc.wait()
            print(f"[{model_key}] exit code: {rc}")
            if rc != 0:
                raise SystemExit(rc)
    else:
        for model_key in selected_models:
            cmd = _build_child_command(model_key, args, per_model_jobs)
            print(f"\n=== Benchmarking {model_key} (parallel) ===")
            print("Command:", " ".join(cmd))
            proc = subprocess.Popen(
                cmd,
                cwd=str(PROJECT_ROOT),
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
            )
            procs[model_key] = proc
            thread = threading.Thread(target=_stream_child, args=(model_key, proc, tracker, len(selected_models)), daemon=True)
            threads.append(thread)
            thread.start()

        for thread in threads:
            thread.join()

        failed = []
        for model_key, proc in procs.items():
            rc = proc.wait()
            print(f"[{model_key}] exit code: {rc}")
            if rc != 0:
                failed.append((model_key, rc))
        if failed:
            raise SystemExit(failed[0][1])

    results = []
    report_override_map = {
        "rf": args.rf_out_report,
        "et": args.et_out_report,
    }
    for model_key in selected_models:
        cfg = CHILD_MODEL_CONFIG[model_key]
        report_path = report_override_map.get(model_key, cfg["report"])
        joblib_path = cfg["joblib"]
        result = _rehydrate_result(report_path, joblib_path)
        results.append(result)

        print("Model:", result["model_name"])
        print("Best params:", result["best_params"])
        print("CV average precision:", result["cv_best_average_precision"])
        print("CV best recall:", result.get("cv_best_recall"))
        print("Estimated node count:", result["estimated_node_count"])
        print("Threshold from validation cost:", result["validation_threshold_result"])
        print("Validation AP:", result.get("validation_average_precision"))
        print("Validation recall:", result.get("validation_recall"))
        print("Validation balanced accuracy:", result.get("validation_balanced_accuracy"))
        print("Test accuracy:", result["test_accuracy"])
        print("Test average precision:", result["test_average_precision"])
        cm = result["test_confusion_matrix"]
        print("Test confusion matrix:\n[[%d %d]\n [%d %d]]" % (cm["tn"], cm["fp"], cm["fn"], cm["tp"]))

    winner, ranked_results, winner_policy = pick_winner(
        results,
        winner_mode=args.winner_mode,
        fn_weight=args.fn_weight,
        fp_weight=args.fp_weight,
    )
    preferred_winner, winner_policy = _prefer_rf_deployment_winner(ranked_results, winner_policy)
    if preferred_winner is not None:
        winner = preferred_winner

    print("\n=== Full benchmark table ===")
    for i, r in enumerate(ranked_results, start=1):
        cm = r["test_confusion_matrix"]
        print(
            f"#{i} {r['model_name']}: "
            f"WinnerScore={r.get('winner_score', float('nan')):.6f}, "
            f"CV_AP={r['cv_best_average_precision']:.6f}, "
            f"CV_Recall={r.get('cv_best_recall', float('nan')):.6f}, "
            f"Val_AP={r.get('validation_average_precision', float('nan')):.6f}, "
            f"Val_Recall={r.get('validation_recall', float('nan')):.6f}, "
            f"Val_BalAcc={r.get('validation_balanced_accuracy', float('nan')):.6f}, "
            f"Test_AP={r['test_average_precision']:.6f}, "
            f"Test_ACC={r['test_accuracy']:.6f}, "
            f"Balanced_ACC={r.get('test_balanced_accuracy', float('nan')):.6f}, "
            f"ROC_AUC={r.get('test_roc_auc', float('nan')):.6f}, "
            f"F1={r.get('test_f1', float('nan')):.6f}, "
            f"Recall={r.get('test_recall', float('nan')):.6f}, "
            f"Specificity={r.get('test_specificity', float('nan')):.6f}, "
            f"Thr={r['threshold']:.6f}, Nodes={r['estimated_node_count']}, "
            f"ValCM=[TN={r['validation_threshold_result']['tn']}, FP={r['validation_threshold_result']['fp']}, "
            f"FN={r['validation_threshold_result']['fn']}, TP={r['validation_threshold_result']['tp']}], "
            f"TestCM=[TN={cm['tn']}, FP={cm['fp']}, FN={cm['fn']}, TP={cm['tp']}]"
        )

    print("\n=== Winner ===")
    print("Winner mode:", args.winner_mode)
    print("Winner:", winner["model_name"])
    print("Winner score:", winner.get("winner_score"))
    print("Winner reason:", winner.get("winner_reason", "—"))

    settings = {
        "csv": args.csv,
        "fn_weight": args.fn_weight,
        "fp_weight": args.fp_weight,
        "min_recall": args.min_recall,
        "min_precision": args.min_precision,
        "max_fpr": args.max_fpr,
        "min_threshold": args.min_threshold,
        "n_iter": args.n_iter,
        "models": selected_models,
        "winner_mode": args.winner_mode,
        "parallel_models": bool(not args.serial_models and len(selected_models) > 1),
        "model_n_jobs": per_model_jobs,
        "feature_report": args.feature_report,
        "rf_feature_report": args.rf_feature_report,
        "et_feature_report": args.et_feature_report,
        "arc_tolerance_mode": args.arc_tolerance_mode,
        "pre_arc_window": int(args.pre_arc_window),
        "post_arc_window": int(args.post_arc_window),
        "soft_neighbor_weight": float(args.soft_neighbor_weight),
        "expanded_neighbor_weight": float(args.expanded_neighbor_weight),
        "hard_negative_ring": int(args.hard_negative_ring),
        "final_negative_ratio": float(args.final_negative_ratio),
        "positive_oversample": float(args.positive_oversample),
        "mismatch_fp_boost": float(args.mismatch_fp_boost),
        "mismatch_fn_boost": float(args.mismatch_fn_boost),
        "mismatch_focus_ratio": float(args.mismatch_focus_ratio),
        "mismatch_verified_only": bool(args.mismatch_verified_only),
        "generation": str(selected_generation or "legacy"),
        "model_features": {
            str(r.get("model_key", r.get("model_name", ""))): list(r.get("arc_base_feature_names", []) or [])
            for r in ranked_results
        },
    }
    report_overrides = {
        "selected_generation": str(selected_generation or "legacy"),
        "source_model_generation_summary": dict((winner.get("source_model_generation_summary") or {})),
        "resolved_paths": {
            "selected_generation": str(selected_generation or "legacy"),
            "resolved_arc_dataset_path": str(args.csv),
            "resolved_feature_report_path": str(args.feature_report or DEFAULT_FEATURE_REPORT),
            "resolved_report_path": str(args.out_report),
            "resolved_rf_report_path": str(args.rf_out_report),
            "resolved_et_report_path": str(args.et_out_report),
            "benchmark_folder": str(getattr(generation_paths, "benchmark_dir", "")),
            "generation_manifest_path": str(getattr(generation_paths, "manifest_path", "")),
        },
        "archive_output_paths": generation_paths.archive_output_paths() if generation_paths is not None else {},
        "canonical_export_paths": generation_paths.canonical_export_paths() if generation_paths is not None else {},
        "generation_advancement_assessment": winner.get("generation_advancement_assessment", {}),
    }
    save_duel_bundle(
        winner=winner,
        results=ranked_results,
        out_header=args.out_header,
        out_joblib=args.out_joblib,
        out_report=args.out_report,
        settings=settings,
        winner_policy=winner_policy,
        feature_names=winner.get("feature_names", ARC_FEATURES),
        report_overrides=report_overrides,
    )

    companion_map = {
        "rf": args.rf_out_report,
        "et": args.et_out_report,
    }
    for result in ranked_results:
        out_path = companion_map.get(result.get("model_key"))
        if not out_path:
            continue
        ensure_dir(out_path)
        payload = strip_estimator(result)
        payload["feature_names"] = list(result.get("feature_names", ARC_FEATURES))
        payload["arc_base_feature_names"] = list(result.get("arc_base_feature_names", []) or [])
        payload["arc_context_feature_names"] = list(result.get("arc_context_feature_names", []) or [])
        payload["model_input_feature_ids"] = list(result.get("model_input_feature_ids", []) or [])
        payload["arc_base_feature_ids"] = list(result.get("arc_base_feature_ids", []) or [])
        payload["settings"] = {
            **settings,
            "model": result.get("model_key"),
            "features": list(result.get("arc_base_feature_names", []) or []),
        }
        payload["winner_policy"] = winner_policy
        payload["generated_by"] = "train_duel"
        with open(out_path, "w", encoding="utf-8") as f:
            json.dump(payload, f, indent=2)
        print("Saved:", out_path)

    if generation_paths is not None:
        archived = archive_generation_exports(generation_paths, kinds=["duel"])
        update_generation_manifest(
            generation_paths,
            {
                "selected_generation": selected_generation,
                "source_model_generations_seen": sorted(
                    set(
                        key
                        for result in ranked_results
                        for key in (result.get("source_model_generation_summary") or {}).keys()
                    )
                ),
                "trainer_settings_snapshot": {"duel": settings},
                "duel_training": {
                    "report_path": str(args.out_report),
                    "rf_report_path": str(args.rf_out_report),
                    "et_report_path": str(args.et_out_report),
                    "winner_model": str(winner.get("model_name", "")),
                    "winner_model_key": str(winner.get("model_key", "")),
                    "winner_features": list(winner.get("arc_base_feature_names", []) or []),
                    "archive_outputs": archived,
                    "generation_advancement_assessment": winner.get("generation_advancement_assessment", {}),
                },
                "recommended_next_action": (
                    f"Trained on {selected_generation}. "
                    f"The canonical duel export is now the active deployment target. "
                    + (
                        f"Place next collection raw CSVs in tinyml/data/{next_generation_tag(selected_generation)}/raw."
                        if next_generation_tag(selected_generation)
                        else "Review held-out winner metrics before advancing beyond the current generation."
                    )
                ),
            },
        )

    final_total = int(sum(max(1, v.get("total", 1)) for v in tracker.values()))
    emit_progress(
        final_total,
        final_total,
        {
            "stage": "workflow_done",
            "message": f"All models finished. Winner: {winner['model_name']}",
            "all_models_done": True,
        },
    )

    if selected_generation:
        print("Selected generation:", selected_generation)
    print("Saved:", args.out_joblib)
    print("Saved:", args.out_header)
    print("Saved:", args.out_report)


if __name__ == "__main__":
    main()
