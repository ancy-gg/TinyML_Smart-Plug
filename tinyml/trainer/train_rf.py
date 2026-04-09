import argparse
import json
import os
from pathlib import Path

from tinyml_common import (
    ARC_FEATURES,
    build_group_inventory_report,
    build_policy_subset_metrics_report,
    build_short_burst_report,
    build_subset_metrics_report,
    calibrate_family_threshold_policy,
    evaluate_threshold_policy,
    load_clean_dataset,
    make_group_splits,
    save_model_bundle,
    train_one_model,
    estimate_search_plan,
)

SCRIPT_DIR = Path(__file__).resolve().parent
TINYML_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TINYML_DIR.parent

CSV_PATH = str(PROJECT_ROOT / "tinyml" / "data" / "arc_training.csv")
OUT_HEADER = str(PROJECT_ROOT / "tinyml" / "model" / "TinyMLTreeEnsemble_RF.h")
OUT_JOBLIB = str(PROJECT_ROOT / "tinyml" / "trainer" / "lib" / "TinyMLTreeEnsemble_RF.joblib")
OUT_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLTreeEnsemble_RF_report.json")


def emit_progress(current: int, total: int, payload: dict) -> None:
    payload = dict(payload or {})
    payload["current"] = int(current)
    payload["total"] = int(total)
    print("[[PROGRESS]] " + json.dumps(payload, sort_keys=True), flush=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default=CSV_PATH)
    ap.add_argument("--out_header", default=OUT_HEADER)
    ap.add_argument("--out_joblib", default=OUT_JOBLIB)
    ap.add_argument("--out_report", default=OUT_REPORT)
    ap.add_argument("--include_invalid", action="store_true")
    ap.add_argument("--fn_weight", type=float, default=80.0)
    ap.add_argument("--fp_weight", type=float, default=4.0)
    ap.add_argument("--min_recall", type=float, default=0.97)
    ap.add_argument("--min_precision", type=float, default=0.90)
    ap.add_argument("--max_fpr", type=float, default=0.03)
    ap.add_argument("--min_threshold", type=float, default=0.08)
    ap.add_argument("--n_iter", type=int, default=72)
    ap.add_argument("--n_jobs", type=int, default=-1)
    args = ap.parse_args()

    if args.n_jobs != 0:
        os.environ["TINYML_N_JOBS"] = str(args.n_jobs)

    df_meta, X, y, groups, w = load_clean_dataset(
        args.csv,
        include_invalid=args.include_invalid,
        feature_names=ARC_FEATURES,
    )
    splits = make_group_splits(X, y, groups)
    print("CV split summary:", splits.get("cv_group_summary", {}))

    train_idx = splits["train_idx"]
    val_idx = splits["val_idx"]
    train_full_idx = splits["train_full_idx"]

    result = train_one_model(
        model_key="rf",
        X_train=splits["X_train_full"].iloc[train_idx],
        y_train=splits["y_train_full"].iloc[train_idx],
        groups_train=splits["groups_train_full"].iloc[train_idx],
        w_train=w[train_full_idx][train_idx],
        X_val=splits["X_train_full"].iloc[val_idx],
        y_val=splits["y_train_full"].iloc[val_idx],
        X_test=splits["X_test"],
        y_test=splits["y_test"],
        fn_weight=args.fn_weight,
        fp_weight=args.fp_weight,
        min_recall=args.min_recall,
        n_iter=args.n_iter,
        min_precision=args.min_precision,
        max_fpr=args.max_fpr,
        min_threshold=args.min_threshold,
        progress_callback=emit_progress,
    )

    val_meta = df_meta.iloc[train_full_idx].iloc[val_idx].reset_index(drop=True)
    val_score = result["estimator"].predict_proba(splits["X_train_full"].iloc[val_idx])[:, 1]
    result["threshold_policy"] = calibrate_family_threshold_policy(
        val_meta,
        splits["y_train_full"].iloc[val_idx].to_numpy(),
        val_score,
        base_threshold=result["threshold"],
        fn_weight=args.fn_weight,
        fp_weight=args.fp_weight,
        min_recall=args.min_recall,
        min_precision=args.min_precision,
        max_fpr=args.max_fpr,
        min_threshold=args.min_threshold,
    )
    result["validation_policy_metrics"] = evaluate_threshold_policy(
        val_meta,
        splits["y_train_full"].iloc[val_idx].to_numpy(),
        val_score,
        result["threshold_policy"],
    )

    y_score = result["estimator"].predict_proba(splits["X_test"])[:, 1]
    test_meta = df_meta.iloc[splits["test_idx"]].reset_index(drop=True)
    result["test_policy_metrics"] = evaluate_threshold_policy(
        test_meta,
        splits["y_test"].to_numpy(),
        y_score,
        result["threshold_policy"],
    )

    full_load_col = "device_family" if "device_family" in df_meta.columns else ("parsed_load_type" if "parsed_load_type" in df_meta.columns else ("load_type" if "load_type" in df_meta.columns else None))
    test_load_col = "device_family" if "device_family" in test_meta.columns else ("parsed_load_type" if "parsed_load_type" in test_meta.columns else ("load_type" if "load_type" in test_meta.columns else None))
    test_device_col = "device_name" if "device_name" in test_meta.columns else None

    result["test_per_trial"] = build_subset_metrics_report(test_meta, splits["y_test"].to_numpy(), y_score, result["threshold"], "trial_id")
    result["test_per_trial_policy"] = build_policy_subset_metrics_report(test_meta, splits["y_test"].to_numpy(), y_score, result["threshold_policy"], "trial_id")

    if test_load_col is not None:
        result["test_per_load"] = build_subset_metrics_report(test_meta, splits["y_test"].to_numpy(), y_score, result["threshold"], test_load_col)
        result["test_per_load_policy"] = build_policy_subset_metrics_report(test_meta, splits["y_test"].to_numpy(), y_score, result["threshold_policy"], test_load_col)
    else:
        result["test_per_load"] = []
        result["test_per_load_policy"] = []

    if test_device_col is not None:
        result["test_per_device"] = build_subset_metrics_report(test_meta, splits["y_test"].to_numpy(), y_score, result["threshold"], test_device_col)
        result["test_per_device_policy"] = build_policy_subset_metrics_report(test_meta, splits["y_test"].to_numpy(), y_score, result["threshold_policy"], test_device_col)
    else:
        result["test_per_device"] = []
        result["test_per_device_policy"] = []

    if "context_family_code_runtime" in test_meta.columns:
        result["test_per_runtime_context"] = build_subset_metrics_report(test_meta, splits["y_test"].to_numpy(), y_score, result["threshold"], "context_family_code_runtime")
        result["test_per_runtime_context_policy"] = build_policy_subset_metrics_report(test_meta, splits["y_test"].to_numpy(), y_score, result["threshold_policy"], "context_family_code_runtime")
    else:
        result["test_per_runtime_context"] = []
        result["test_per_runtime_context_policy"] = []

    result["short_burst_sensitivity"] = build_short_burst_report(test_meta, splits["y_test"].to_numpy(), y_score, result["threshold"], max_positive_run_rows=3)
    result["leave_one_trial_out_inventory"] = build_group_inventory_report(df_meta, "trial_id")
    result["leave_one_load_type_out_inventory"] = build_group_inventory_report(df_meta, full_load_col) if full_load_col is not None else []
    result["leave_one_device_out_inventory"] = build_group_inventory_report(df_meta, "device_name") if "device_name" in df_meta.columns else []

    search_plan = estimate_search_plan(args.n_iter)
    print("Search plan:", search_plan)
    print("Model:", result["model_name"])
    print("Best params:", result["best_params"])
    print("CV average precision:", result["cv_best_average_precision"])
    print("CV best recall:", result.get("cv_best_recall"))
    print("Estimated node count:", result["estimated_node_count"])
    print("Threshold from validation cost:", result["validation_threshold_result"])
    print("Test accuracy:", result["test_accuracy"])
    print("Test average precision:", result["test_average_precision"])

    cm = result["test_confusion_matrix"]
    print("Test confusion matrix:\n[[%d %d]\n [%d %d]]" % (cm["tn"], cm["fp"], cm["fn"], cm["tp"]))

    settings = {
        "csv": args.csv,
        "fn_weight": args.fn_weight,
        "fp_weight": args.fp_weight,
        "min_recall": args.min_recall,
        "min_precision": args.min_precision,
        "max_fpr": args.max_fpr,
        "min_threshold": args.min_threshold,
        "n_iter": args.n_iter,
        "n_jobs": args.n_jobs,
        "model": "rf",
    }
    save_model_bundle(
        result=result,
        out_header=args.out_header,
        out_joblib=args.out_joblib,
        out_report=args.out_report,
        settings=settings,
        feature_names=ARC_FEATURES,
    )

    print("Saved:", args.out_joblib)
    print("Saved:", args.out_header)
    print("Saved:", args.out_report)


if __name__ == "__main__":
    main()