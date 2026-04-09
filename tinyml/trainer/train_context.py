import argparse
import json
from pathlib import Path

import numpy as np
import pandas as pd
from sklearn.metrics import accuracy_score, balanced_accuracy_score, classification_report, confusion_matrix
from sklearn.model_selection import StratifiedShuffleSplit

from tinyml_common import (
    CONTEXT_FEATURES,
    DEVICE_FAMILY_CLASSES,
    DEVICE_FAMILY_CODE_MAP,
    DEVICE_FAMILY_NAME_FROM_CODE,
    DEVICE_FAMILY_UNKNOWN_CODE,
    clean_df,
    normalize_feature_names,
    save_context_bundle,
)

SCRIPT_DIR = Path(__file__).resolve().parent
TINYML_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TINYML_DIR.parent

CSV_PATH = str(PROJECT_ROOT / "tinyml" / "data" / "load_context.csv")
OUT_HEADER = str(PROJECT_ROOT / "tinyml" / "model" / "TinyMLContextModel.h")
OUT_JOBLIB = str(PROJECT_ROOT / "tinyml" / "trainer" / "lib" / "TinyMLContextModel.joblib")
OUT_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLContextModel_report.json")

CONTEXT_UNKNOWN_CONFIDENCE = 0.45
CONTEXT_WINDOW_ROWS = 50


def family_name_from_code(code: int) -> str:
    return DEVICE_FAMILY_NAME_FROM_CODE.get(int(code), "unknown")


def build_context_frame(df: pd.DataFrame) -> pd.DataFrame:
    df = normalize_feature_names(df.copy())
    if "device_family_code" not in df.columns:
        fam = df.get("device_family", pd.Series("unknown", index=df.index)).astype(str)
        fam = fam.str.strip().str.lower()
        df["device_family_code"] = fam.map(lambda s: DEVICE_FAMILY_CODE_MAP.get(str(s), DEVICE_FAMILY_UNKNOWN_CODE)).fillna(DEVICE_FAMILY_UNKNOWN_CODE).astype(int)

    df = clean_df(df, include_invalid=False, feature_names=CONTEXT_FEATURES, target_col="device_family_code")
    df = df[df["device_family_code"].astype(int) >= 0].copy()
    if df.empty:
        raise ValueError("No known-family rows available after context cleaning.")

    group_col = "trial_id" if "trial_id" in df.columns else ("session_id" if "session_id" in df.columns else ("section_id" if "section_id" in df.columns else None))
    if group_col is None:
        work_group = pd.Series(np.arange(len(df)), index=df.index, dtype=int).map(lambda i: f"ctx_{int(i):06d}")
        df["_ctx_group"] = work_group
        group_col = "_ctx_group"

    time_col = "frame_start_uptime_ms" if "frame_start_uptime_ms" in df.columns else ("frame_end_uptime_ms" if "frame_end_uptime_ms" in df.columns else ("uptime_ms" if "uptime_ms" in df.columns else ("epoch_ms" if "epoch_ms" in df.columns else None)))
    work = df.copy()
    if time_col is not None:
        work = work.sort_values([group_col, time_col]).reset_index(drop=True)

    division = work.get("division_tag", pd.Series("", index=work.index)).astype(str).str.lower()
    preferred = work[division.isin(["start", "startup"])].copy()
    if preferred.empty:
        preferred = work.copy()

    rows = []
    for group_value, g in preferred.groupby(group_col, sort=False):
        if g.empty:
            continue
        g = g.sort_values(time_col).reset_index(drop=True) if time_col is not None else g.reset_index(drop=True)
        fam_code = int(g["device_family_code"].astype(int).mode().iloc[0])
        if fam_code < 0:
            continue
        fam_name = family_name_from_code(fam_code)

        i_rms = pd.to_numeric(g.get("i_rms", pd.Series(0.0, index=g.index)), errors="coerce").fillna(0.0)
        nonzero_idx = i_rms[i_rms >= 0.05].index
        if len(nonzero_idx) == 0:
            continue
        start_idx = int(nonzero_idx[0])
        window = g.iloc[start_idx:start_idx + CONTEXT_WINDOW_ROWS].copy()
        if window.empty:
            continue

        rec = {
            "group": str(group_value),
            "device_family_code": fam_code,
            "device_family": fam_name,
            "device_name": str(window.get("device_name", pd.Series("unknown_device", index=window.index)).iloc[0]),
            "row_count": int(len(window)),
            "window_rows_target": int(CONTEXT_WINDOW_ROWS),
        }
        for feat in CONTEXT_FEATURES:
            rec[feat] = float(pd.to_numeric(window[feat], errors="coerce").dropna().mean()) if feat in window.columns else 0.0
        rows.append(rec)

    out = pd.DataFrame(rows)
    if out.empty:
        raise ValueError("No grouped context rows were produced.")
    return out


def standardize(train_x: np.ndarray, test_x: np.ndarray):
    mu = train_x.mean(axis=0)
    sigma = train_x.std(axis=0)
    sigma = np.where(sigma < 1e-6, 1.0, sigma)
    return (train_x - mu) / sigma, (test_x - mu) / sigma, mu, sigma


def softmax(x: np.ndarray) -> np.ndarray:
    x = x - np.max(x, axis=1, keepdims=True)
    e = np.exp(x)
    denom = np.sum(e, axis=1, keepdims=True)
    denom = np.where(denom <= 0, 1.0, denom)
    return e / denom


def prototype_predict_proba(x_std: np.ndarray, centroids: np.ndarray) -> np.ndarray:
    d2 = np.sum((x_std[:, None, :] - centroids[None, :, :]) ** 2, axis=2)
    logits = -d2
    return softmax(logits)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default=CSV_PATH)
    ap.add_argument("--out_header", default=OUT_HEADER)
    ap.add_argument("--out_joblib", default=OUT_JOBLIB)
    ap.add_argument("--out_report", default=OUT_REPORT)
    ap.add_argument("--n_iter", type=int, default=1)
    ap.add_argument("--min_recall", type=float, default=0.0)
    ap.add_argument("--min_precision", type=float, default=0.0)
    ap.add_argument("--max_fpr", type=float, default=1.0)
    ap.add_argument("--min_threshold", type=float, default=0.0)
    ap.add_argument("--unknown_confidence", type=float, default=CONTEXT_UNKNOWN_CONFIDENCE)
    args = ap.parse_args()

    raw_df = pd.read_csv(args.csv)
    ctx_df = build_context_frame(raw_df)
    class_counts = ctx_df["device_family"].value_counts().to_dict()
    valid_classes = [fam for fam in DEVICE_FAMILY_CLASSES if class_counts.get(fam, 0) > 0]
    if len(valid_classes) < 2:
        raise ValueError("Need at least 2 known device families to train the context model.")

    X = ctx_df[CONTEXT_FEATURES].astype(float).to_numpy()
    y = ctx_df["device_family_code"].astype(int).to_numpy()

    class_min = min(int(class_counts.get(fam, 0)) for fam in valid_classes) if valid_classes else 0
    if len(ctx_df) >= len(valid_classes) * 2 and class_min >= 2:
        test_rows = max(len(valid_classes), int(round(len(ctx_df) * 0.25)))
        test_rows = min(test_rows, len(ctx_df) - len(valid_classes))
        if test_rows >= len(valid_classes) and (len(ctx_df) - test_rows) >= len(valid_classes):
            splitter = StratifiedShuffleSplit(n_splits=1, test_size=test_rows, random_state=42)
            train_idx, test_idx = next(splitter.split(X, y))
        else:
            train_idx = np.arange(len(ctx_df))
            test_idx = np.arange(len(ctx_df))
    elif len(ctx_df) >= 4:
        rng = np.random.default_rng(42)
        order = rng.permutation(len(ctx_df))
        cut = max(1, int(round(len(ctx_df) * 0.25)))
        test_idx = np.sort(order[:cut])
        train_idx = np.sort(order[cut:])
    else:
        train_idx = np.arange(len(ctx_df))
        test_idx = np.arange(len(ctx_df))

    X_train, X_test = X[train_idx], X[test_idx]
    y_train, y_test = y[train_idx], y[test_idx]
    X_train_std, X_test_std, means, stds = standardize(X_train, X_test)

    centroids = np.zeros((len(DEVICE_FAMILY_CLASSES), X_train_std.shape[1]), dtype=float)
    centroid_map = {}
    for fam_idx, fam_name in enumerate(DEVICE_FAMILY_CLASSES):
        mask = y_train == fam_idx
        if np.any(mask):
            centroids[fam_idx] = X_train_std[mask].mean(axis=0)
        centroid_map[fam_name] = centroids[fam_idx].tolist()

    proba = prototype_predict_proba(X_test_std, centroids)
    pred = np.argmax(proba, axis=1)
    conf = np.max(proba, axis=1)
    unknown_conf = float(args.unknown_confidence)
    pred_with_unknown = np.where(conf >= unknown_conf, pred, DEVICE_FAMILY_UNKNOWN_CODE)

    labels = list(range(len(DEVICE_FAMILY_CLASSES)))
    cm = confusion_matrix(y_test, pred, labels=labels)
    per_class = []
    class_report = classification_report(
        y_test,
        pred,
        labels=labels,
        target_names=DEVICE_FAMILY_CLASSES,
        output_dict=True,
        zero_division=0,
    )

    pred_unknown_rate = float(np.mean(pred_with_unknown == DEVICE_FAMILY_UNKNOWN_CODE)) if len(pred_with_unknown) else 0.0
    for fam_idx, fam in enumerate(DEVICE_FAMILY_CLASSES):
        mask = y_test == fam_idx
        class_rows = int(mask.sum())
        class_acc = float(np.mean(pred[mask] == fam_idx)) if class_rows else 0.0
        mean_conf = float(np.mean(conf[mask])) if class_rows else 0.0
        unknown_rate = float(np.mean(pred_with_unknown[mask] == DEVICE_FAMILY_UNKNOWN_CODE)) if class_rows else 0.0
        metrics = class_report.get(fam, {}) or {}
        per_class.append({
            "family": fam,
            "family_code": fam_idx,
            "rows": class_rows,
            "accuracy": class_acc,
            "mean_confidence": mean_conf,
            "unknown_rate": unknown_rate,
            "precision": float(metrics.get("precision", 0.0)),
            "recall": float(metrics.get("recall", 0.0)),
            "f1": float(metrics.get("f1-score", 0.0)),
            "support": int(metrics.get("support", class_rows) or 0),
            "predicted_rows": int(np.sum(pred == fam_idx)),
        })

    report = {
        "model_name": "ContextPrototype",
        "feature_names": CONTEXT_FEATURES,
        "class_labels": DEVICE_FAMILY_CLASSES,
        "unknown_confidence_threshold": unknown_conf,
        "row_count": int(len(ctx_df)),
        "train_rows": int(len(train_idx)),
        "test_rows": int(len(test_idx)),
        "class_counts": {k: int(v) for k, v in class_counts.items()},
        "accuracy": float(accuracy_score(y_test, pred)),
        "balanced_accuracy": float(balanced_accuracy_score(y_test, pred)),
        "accuracy_with_unknown_gate": float(np.mean(pred_with_unknown == y_test)) if len(y_test) else 0.0,
        "mean_confidence": float(np.mean(conf)) if len(conf) else 0.0,
        "unknown_rate": pred_unknown_rate,
        "confusion_matrix": cm.tolist(),
        "per_class": per_class,
        "classification_report": class_report,
        "means": means.tolist(),
        "stds": stds.tolist(),
        "centroids": centroid_map,
        "estimator": {
            "model_type": "prototype_centroid",
            "feature_names": list(CONTEXT_FEATURES),
            "means": means.tolist(),
            "stds": stds.tolist(),
            "centroids": centroid_map,
        },
    }

    settings = {
        "csv": args.csv,
        "n_iter": args.n_iter,
        "min_recall": args.min_recall,
        "min_precision": args.min_precision,
        "max_fpr": args.max_fpr,
        "min_threshold": args.min_threshold,
        "unknown_confidence": unknown_conf,
        "model": "context",
    }

    save_context_bundle(
        report=report,
        out_header=args.out_header,
        out_joblib=args.out_joblib,
        out_report=args.out_report,
        settings=settings,
    )

    print(json.dumps({
        "model_name": report["model_name"],
        "row_count": report["row_count"],
        "train_rows": report["train_rows"],
        "test_rows": report["test_rows"],
        "accuracy": report["accuracy"],
        "balanced_accuracy": report["balanced_accuracy"],
        "mean_confidence": report["mean_confidence"],
        "unknown_rate": report["unknown_rate"],
    }, indent=2))
    print("Saved:", args.out_joblib)
    print("Saved:", args.out_header)
    print("Saved:", args.out_report)


if __name__ == "__main__":
    main()