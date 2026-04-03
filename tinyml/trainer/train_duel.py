import argparse
import json
from pathlib import Path

from tinyml_common import (
    ensure_dir,
    load_clean_dataset,
    make_group_splits,
    pick_winner,
    save_duel_bundle,
    strip_estimator,
    train_one_model,
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
    ap.add_argument("--n_iter", type=int, default=20)
    ap.add_argument(
        "--winner_mode",
        default="safety_composite",
        choices=["safety_composite", "legacy_cv_ap"],
    )
    ap.add_argument(
        "--models",
        nargs="+",
        default=["rf", "et"],
        choices=["rf", "et"],
    )
    args = ap.parse_args()

    _, X, y, groups, w = load_clean_dataset(
        args.csv,
        include_invalid=args.include_invalid,
    )
    splits = make_group_splits(X, y, groups)

    train_idx = splits["train_idx"]
    val_idx = splits["val_idx"]
    train_full_idx = splits["train_full_idx"]

    X_train = splits["X_train_full"].iloc[train_idx]
    y_train = splits["y_train_full"].iloc[train_idx]
    groups_train = splits["groups_train_full"].iloc[train_idx]
    w_train = w[train_full_idx][train_idx]

    X_val = splits["X_train_full"].iloc[val_idx]
    y_val = splits["y_train_full"].iloc[val_idx]

    results = []
    for model_key in args.models:
        print(f"\n=== Benchmarking {model_key} ===")
        result = train_one_model(
            model_key=model_key,
            X_train=X_train,
            y_train=y_train,
            groups_train=groups_train,
            w_train=w_train,
            X_val=X_val,
            y_val=y_val,
            X_test=splits["X_test"],
            y_test=splits["y_test"],
            fn_weight=args.fn_weight,
            fp_weight=args.fp_weight,
            min_recall=args.min_recall,
            n_iter=args.n_iter,
        )
        results.append(result)

        print("Model:", result["model_name"])
        print("Best params:", result["best_params"])
        print("CV average precision:", result["cv_best_average_precision"])
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
        "n_iter": args.n_iter,
        "models": args.models,
        "winner_mode": args.winner_mode,
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

    print("Saved:", args.out_joblib)
    print("Saved:", args.out_header)
    print("Saved:", args.out_report)


if __name__ == "__main__":
    main()
