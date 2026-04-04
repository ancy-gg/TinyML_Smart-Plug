import argparse
import json
import os
import subprocess
import sys
import threading
from pathlib import Path

import joblib

from tinyml_common import (
    ensure_dir,
    pick_winner,
    save_duel_bundle,
    strip_estimator,
    estimate_search_plan,
)

SCRIPT_DIR = Path(__file__).resolve().parent
TINYML_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TINYML_DIR.parent

CSV_PATH = str(PROJECT_ROOT / "tinyml" / "data" / "cleaned_data.csv")
OUT_HEADER = str(PROJECT_ROOT / "tinyml" / "TinyMLTreeEnsemble.h")
OUT_JOBLIB = str(PROJECT_ROOT / "tinyml" / "trainer" / "TinyMLTreeEnsemble.joblib")
OUT_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "benchmark_report.json")
RF_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLTreeEnsemble_RF_report.json")
ET_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLTreeEnsemble_ET_report.json")
RF_JOBLIB = str(PROJECT_ROOT / "tinyml" / "trainer" / "TinyMLTreeEnsemble_RF.joblib")
ET_JOBLIB = str(PROJECT_ROOT / "tinyml" / "trainer" / "TinyMLTreeEnsemble_ET.joblib")
RF_HEADER = str(PROJECT_ROOT / "tinyml" / "TinyMLTreeEnsemble_RF.h")
ET_HEADER = str(PROJECT_ROOT / "tinyml" / "TinyMLTreeEnsemble_ET.h")

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


def _per_model_n_jobs(model_count: int, override: int) -> int:
    if override is not None and int(override) != 0:
        return int(override)
    cpu_total = os.cpu_count() or 1
    if model_count <= 1:
        return -1
    return max(1, cpu_total // model_count)


def _build_child_command(model_key: str, args, n_jobs: int) -> list[str]:
    cfg = CHILD_MODEL_CONFIG[model_key]
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
        cfg["report"],
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
    if args.include_invalid:
        cmd.append("--include_invalid")
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
    ap.add_argument("--include_invalid", action="store_true")
    ap.add_argument("--fn_weight", type=float, default=80.0)
    ap.add_argument("--fp_weight", type=float, default=4.0)
    ap.add_argument("--min_recall", type=float, default=0.97)
    ap.add_argument("--min_precision", type=float, default=0.90)
    ap.add_argument("--max_fpr", type=float, default=0.03)
    ap.add_argument("--min_threshold", type=float, default=0.08)
    ap.add_argument("--n_iter", type=int, default=60)
    ap.add_argument(
        "--winner_mode",
        default="arc_guard",
        choices=["arc_guard", "safety_composite", "legacy_cv_ap"],
    )
    ap.add_argument(
        "--models",
        nargs="+",
        default=["rf", "et"],
        choices=["rf", "et"],
    )
    ap.add_argument("--model_n_jobs", type=int, default=0)
    ap.add_argument("--serial_models", action="store_true")
    args = ap.parse_args()

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
            thread = threading.Thread(
                target=_stream_child,
                args=(model_key, proc, tracker, len(selected_models)),
                daemon=True,
            )
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
        print(
            "Test confusion matrix:\n[[%d %d]\n [%d %d]]"
            % (cm["tn"], cm["fp"], cm["fn"], cm["tp"])
        )

    winner, ranked_results, winner_policy = pick_winner(
        results,
        winner_mode=args.winner_mode,
        fn_weight=args.fn_weight,
        fp_weight=args.fp_weight,
    )

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
    }
    save_duel_bundle(
        winner=winner,
        results=ranked_results,
        out_header=args.out_header,
        out_joblib=args.out_joblib,
        out_report=args.out_report,
        settings=settings,
        winner_policy=winner_policy,
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
        payload["settings"] = settings
        payload["winner_policy"] = winner_policy
        payload["generated_by"] = "train_duel"
        with open(out_path, "w", encoding="utf-8") as f:
            json.dump(payload, f, indent=2)
        print("Saved:", out_path)

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

    print("Saved:", args.out_joblib)
    print("Saved:", args.out_header)
    print("Saved:", args.out_report)


if __name__ == "__main__":
    main()
