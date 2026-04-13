import argparse
import json
import sys
import warnings
from pathlib import Path

import numpy as np
import pandas as pd
from sklearn.metrics import accuracy_score, balanced_accuracy_score, classification_report, confusion_matrix
from sklearn.model_selection import StratifiedShuffleSplit

from tinyml_common import (
    CONTEXT_ALL_COMPUTED_FEATURES,
    CONTEXT_FEATURES,
    CONTEXT_SWEEP_FEATURES,
    DEVICE_FAMILY_CLASSES,
    DEVICE_FAMILY_CODE_MAP,
    DEVICE_FAMILY_NAME_FROM_CODE,
    DEVICE_FAMILY_UNKNOWN_CODE,
    apply_scaffold_gap_fill_cap,
    build_mismatch_summary,
    clean_df,
    normalize_feature_names,
    save_context_bundle,
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

CSV_PATH = str(PROJECT_ROOT / "tinyml" / "data" / "load_context.csv")
OUT_HEADER = str(PROJECT_ROOT / "tinyml" / "model" / "TinyMLContextModel.h")
OUT_JOBLIB = str(PROJECT_ROOT / "tinyml" / "trainer" / "lib" / "TinyMLContextModel.joblib")
OUT_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLContextModel_report.json")
DEFAULT_FEATURE_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "TinyMLContextFeatureSubsetSweep_report.json")

CONTEXT_UNKNOWN_CONFIDENCE = 0.45
CONTEXT_WINDOW_ROWS = 50


def family_name_from_code(code: int) -> str:
    return DEVICE_FAMILY_NAME_FROM_CODE.get(int(code), "unknown")


def _load_feature_list_from_report(report_path: str | None) -> list[str] | None:
    path = Path(report_path or "")
    if not path.is_file():
        return None
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return None
    candidates = (
        payload.get("recommended_context_features")
        or payload.get("recommended_features")
        or payload.get("default_training_features")
        or payload.get("feature_names")
        or []
    )
    out = [str(x).strip() for x in candidates if str(x).strip() in CONTEXT_SWEEP_FEATURES]
    return out or None


def resolve_context_features(explicit_features=None, feature_report: str | None = None, *, allow_legacy_default_report: bool = True) -> list[str]:
    explicit = [str(x).strip() for x in (explicit_features or []) if str(x).strip()]
    explicit = [x for x in explicit if x in CONTEXT_SWEEP_FEATURES]
    if explicit:
        return validate_feature_subset(explicit, allowed_features=CONTEXT_SWEEP_FEATURES, role="context training")
    from_report = _load_feature_list_from_report(feature_report)
    if from_report:
        return validate_feature_subset(from_report, allowed_features=CONTEXT_SWEEP_FEATURES, role="context training")
    auto_report = _load_feature_list_from_report(DEFAULT_FEATURE_REPORT) if allow_legacy_default_report else None
    if auto_report:
        return validate_feature_subset(auto_report, allowed_features=CONTEXT_SWEEP_FEATURES, role="context training")
    return list(CONTEXT_FEATURES)


def build_context_frame(df: pd.DataFrame, feature_names: list[str]) -> pd.DataFrame:
    df = normalize_feature_names(df.copy())
    if "device_family_code" not in df.columns:
        fam = df.get("device_family", pd.Series("unknown", index=df.index)).astype(str)
        fam = fam.str.strip().str.lower()
        df["device_family_code"] = fam.map(lambda s: DEVICE_FAMILY_CODE_MAP.get(str(s), DEVICE_FAMILY_UNKNOWN_CODE)).fillna(DEVICE_FAMILY_UNKNOWN_CODE).astype(int)

    df = clean_df(df, include_invalid=False, feature_names=feature_names, target_col="device_family_code")
    df = apply_scaffold_gap_fill_cap(df, target_col="device_family_code", trainable_col=None, bucket_cols=[c for c in ["device_family", "device_family_code"] if c in df.columns])
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
    transient_like = division.str.contains("start", na=False) | division.str.contains("transient", na=False)
    preferred = work[transient_like].copy()
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

        row_weights = pd.to_numeric(window.get("sample_weight", pd.Series(1.0, index=window.index)), errors="coerce").fillna(1.0).clip(lower=0.05)
        source_kind_rows = window.get("source_kind", pd.Series("real", index=window.index)).astype(str).str.lower()
        scaffold_like_rows = source_kind_rows.str.contains("scaffold", na=False)
        real_rows = int((~scaffold_like_rows).sum())
        scaffold_rows = int(scaffold_like_rows.sum())
        rec = {
            "group": str(group_value),
            "device_family_code": fam_code,
            "device_family": fam_name,
            "device_name": str(window.get("device_name", pd.Series("unknown_device", index=window.index)).iloc[0]),
            "row_count": int(len(window)),
            "window_rows_target": int(CONTEXT_WINDOW_ROWS),
            "sample_weight": float(row_weights.mean()),
            "source_kind": "scaffold" if scaffold_rows > real_rows else "real",
            "real_rows": real_rows,
            "scaffold_rows": scaffold_rows,
        }
        for feat in feature_names:
            if feat in window.columns:
                vals = pd.to_numeric(window[feat], errors="coerce")
                valid = vals.notna()
                if bool(valid.any()):
                    w = row_weights.loc[valid]
                    rec[feat] = float(np.average(vals.loc[valid].to_numpy(dtype=float), weights=w.to_numpy(dtype=float)))
                else:
                    rec[feat] = 0.0
            else:
                rec[feat] = 0.0
        rows.append(rec)

    out = pd.DataFrame(rows)
    if out.empty:
        raise ValueError("No grouped context rows were produced.")
    return out


def weighted_standardize(train_x: np.ndarray, test_x: np.ndarray, weights: np.ndarray | None = None):
    if weights is None:
        weights = np.ones(train_x.shape[0], dtype=float)
    weights = np.asarray(weights, dtype=float).reshape(-1)
    if weights.size != train_x.shape[0]:
        weights = np.ones(train_x.shape[0], dtype=float)
    weights = np.clip(weights, 1e-9, None)
    weights = weights / np.sum(weights)
    mu = np.sum(train_x * weights[:, None], axis=0)
    var = np.sum(((train_x - mu) ** 2) * weights[:, None], axis=0)
    sigma = np.sqrt(np.maximum(var, 1e-12))
    sigma = np.where(sigma < 1e-6, 1.0, sigma)
    return (train_x - mu) / sigma, (test_x - mu) / sigma, mu, sigma


def softmax(x: np.ndarray) -> np.ndarray:
    x = x - np.max(x, axis=1, keepdims=True)
    e = np.exp(x)
    denom = np.sum(e, axis=1, keepdims=True)
    denom = np.where(denom <= 0, 1.0, denom)
    return e / denom


def prototype_predict_proba(x_std: np.ndarray, centroids: np.ndarray, active_class_mask=None) -> np.ndarray:
    d2 = np.sum((x_std[:, None, :] - centroids[None, :, :]) ** 2, axis=2)
    logits = -d2
    if active_class_mask is not None:
        mask = np.asarray(active_class_mask, dtype=bool).reshape(1, -1)
        logits = np.where(mask, logits, -1.0e9)
    return softmax(logits)


def main():
    argv = sys.argv[1:]
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default=CSV_PATH)
    ap.add_argument("--out_header", default=OUT_HEADER)
    ap.add_argument("--out_joblib", default=OUT_JOBLIB)
    ap.add_argument("--out_report", default=OUT_REPORT)
    ap.add_argument("--generation", default=None)
    ap.add_argument("--n_iter", type=int, default=1)
    ap.add_argument("--min_recall", type=float, default=0.0)
    ap.add_argument("--min_precision", type=float, default=0.0)
    ap.add_argument("--max_fpr", type=float, default=1.0)
    ap.add_argument("--min_threshold", type=float, default=0.0)
    ap.add_argument("--unknown_confidence", type=float, default=CONTEXT_UNKNOWN_CONFIDENCE)
    ap.add_argument("--feature_report", default=None)
    ap.add_argument("--features", nargs="*", default=None)
    args = ap.parse_args()

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
                "csv": ("--csv", generation_paths.context_csv),
                "out_report": ("--out_report", generation_paths.context_report),
                "feature_report": ("--feature_report", generation_paths.context_subset_report),
            },
        )
        if not cli_flag_present("--csv", argv):
            errors = validate_generation_workspace(generation_paths, require_context_csv=True, require_nonempty_context_csv=True)
            if errors:
                raise ValueError(" ; ".join(errors))

    selected_features = resolve_context_features(args.features, args.feature_report, allow_legacy_default_report=(selected_generation is None))
    raw_df = pd.read_csv(args.csv)
    ctx_df = build_context_frame(raw_df, selected_features)
    class_counts = ctx_df["device_family"].value_counts().to_dict()
    valid_classes = [fam for fam in DEVICE_FAMILY_CLASSES if class_counts.get(fam, 0) > 0]
    missing_classes = [fam for fam in DEVICE_FAMILY_CLASSES if fam not in valid_classes]
    training_warnings = []
    if len(valid_classes) < 2:
        msg = (
            "Context training has fewer than 2 populated known families. "
            "Proceeding anyway and disabling missing classes instead of exporting them as active zero centroids."
        )
        warnings.warn(msg)
        training_warnings.append(msg)
    if missing_classes:
        msg = "Context training is missing class coverage for: " + ", ".join(missing_classes)
        warnings.warn(msg)
        training_warnings.append(msg)

    X = ctx_df[selected_features].astype(float).to_numpy()
    y = ctx_df["device_family_code"].astype(int).to_numpy()
    row_weights = pd.to_numeric(ctx_df.get("sample_weight", pd.Series(1.0, index=ctx_df.index)), errors="coerce").fillna(1.0).to_numpy(dtype=float)

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
    w_train = row_weights[train_idx]
    X_train_std, X_test_std, means, stds = weighted_standardize(X_train, X_test, w_train)

    centroids = np.zeros((len(DEVICE_FAMILY_CLASSES), X_train_std.shape[1]), dtype=float)
    centroid_map = {}
    zero_centroid_labels = []
    active_class_mask = []
    for fam_idx, fam_name in enumerate(DEVICE_FAMILY_CLASSES):
        mask = y_train == fam_idx
        has_rows = bool(np.any(mask))
        active_class_mask.append(1 if has_rows else 0)
        if has_rows:
            fam_w = np.clip(w_train[mask], 1e-9, None)
            centroids[fam_idx] = np.average(X_train_std[mask], axis=0, weights=fam_w)
        else:
            zero_centroid_labels.append(fam_name)
        centroid_map[fam_name] = centroids[fam_idx].tolist()
    if zero_centroid_labels:
        msg = "Disabling missing classes during context inference and exporting zero centroids only as placeholders for: " + ", ".join(zero_centroid_labels)
        warnings.warn(msg)
        training_warnings.append(msg)

    proba = prototype_predict_proba(X_test_std, centroids, active_class_mask=active_class_mask)
    pred = np.argmax(proba, axis=1)
    conf = np.max(proba, axis=1)
    unknown_conf = float(args.unknown_confidence)
    pred_with_unknown = np.where(conf >= unknown_conf, pred, DEVICE_FAMILY_UNKNOWN_CODE)

    labels = list(range(len(DEVICE_FAMILY_CLASSES)))
    cm = confusion_matrix(y_test, pred, labels=labels)
    class_report = classification_report(
        y_test,
        pred,
        labels=labels,
        target_names=DEVICE_FAMILY_CLASSES,
        output_dict=True,
        zero_division=0,
    )

    pred_unknown_rate = float(np.mean(pred_with_unknown == DEVICE_FAMILY_UNKNOWN_CODE)) if len(pred_with_unknown) else 0.0
    per_class = []
    for fam_idx, fam in enumerate(DEVICE_FAMILY_CLASSES):
        mask = y_test == fam_idx
        class_rows = int(mask.sum())
        class_acc = float(np.mean(pred[mask] == fam_idx)) if class_rows else 0.0
        mean_conf = float(np.mean(conf[mask])) if class_rows else 0.0
        unknown_rate = float(np.mean(pred_with_unknown[mask] == DEVICE_FAMILY_UNKNOWN_CODE)) if class_rows else 0.0
        metrics = class_report.get(fam, {}) or {}
        predicted_rows = int(np.sum(pred == fam_idx)) if active_class_mask[fam_idx] else 0
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
            "predicted_rows": predicted_rows,
            "active_in_model": bool(active_class_mask[fam_idx]),
        })

    heldout_accuracy = float(accuracy_score(y_test, pred)) if len(y_test) else 0.0
    heldout_balanced_accuracy = float(balanced_accuracy_score(y_test, pred)) if len(np.unique(y_test)) > 1 else 0.0
    raw_mismatch_summary = build_mismatch_summary(raw_df, current_training_generation=selected_generation)

    report = {
        "model_name": "ContextPrototype",
        "selected_generation": str(selected_generation or "legacy"),
        "feature_names": selected_features,
        "class_labels": DEVICE_FAMILY_CLASSES,
        "unknown_confidence_threshold": unknown_conf,
        "row_count": int(len(ctx_df)),
        "train_rows": int(len(train_idx)),
        "test_rows": int(len(test_idx)),
        "class_counts": {k: int(v) for k, v in class_counts.items()},
        "effective_class_weight": {fam: float(pd.to_numeric(ctx_df.loc[ctx_df["device_family"] == fam, "sample_weight"], errors="coerce").fillna(0.0).sum()) for fam in DEVICE_FAMILY_CLASSES},
        "source_counts": {str(k): int(v) for k, v in ctx_df.get("source_kind", pd.Series(dtype=object)).astype(str).value_counts().to_dict().items()},
        "real_rows": int((ctx_df.get("source_kind", pd.Series("real", index=ctx_df.index)).astype(str).str.lower() != "scaffold").sum()),
        "scaffold_rows": int((ctx_df.get("source_kind", pd.Series("real", index=ctx_df.index)).astype(str).str.lower() == "scaffold").sum()),
        "valid_classes": valid_classes,
        "missing_classes": missing_classes,
        "export_zero_centroid_classes": zero_centroid_labels,
        "active_class_mask": active_class_mask,
        "warnings": training_warnings,
        "dataset_mismatch_summary": raw_mismatch_summary,
        "context_window_mismatch_summary": build_mismatch_summary(ctx_df, current_training_generation=selected_generation),
        "source_model_generation_summary": raw_mismatch_summary.get("source_model_generation_counts", {}),
        "accuracy": heldout_accuracy,
        "balanced_accuracy": heldout_balanced_accuracy,
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
            "feature_names": list(selected_features),
            "means": means.tolist(),
            "stds": stds.tolist(),
            "centroids": centroid_map,
            "active_class_mask": list(active_class_mask),
        },
        "resolved_paths": {
            "selected_generation": str(selected_generation or "legacy"),
            "resolved_context_dataset_path": str(args.csv),
            "resolved_feature_report_path": str(args.feature_report or DEFAULT_FEATURE_REPORT),
            "resolved_report_path": str(args.out_report),
            "benchmark_folder": str(getattr(generation_paths, "benchmark_dir", "")),
            "generation_manifest_path": str(getattr(generation_paths, "manifest_path", "")),
        },
        "archive_output_paths": generation_paths.archive_output_paths() if generation_paths is not None else {},
        "canonical_export_paths": generation_paths.canonical_export_paths() if generation_paths is not None else {},
        "generation_advancement_assessment": {
            "justified": bool(heldout_accuracy >= 0.80 and pred_unknown_rate <= 0.40),
            "basis": "Held-out context balanced-accuracy and unknown-rate checks only.",
            "held_out_snapshot": {
                "accuracy": heldout_accuracy,
                "balanced_accuracy": heldout_balanced_accuracy,
                "unknown_rate": pred_unknown_rate,
            },
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
        "feature_report": args.feature_report,
        "features": list(selected_features),
        "model": "context",
        "generation": str(selected_generation or "legacy"),
    }

    save_context_bundle(
        report=report,
        out_header=args.out_header,
        out_joblib=args.out_joblib,
        out_report=args.out_report,
        settings=settings,
    )

    if generation_paths is not None:
        archived = archive_generation_exports(generation_paths, kinds=["context"])
        update_generation_manifest(
            generation_paths,
            {
                "selected_generation": selected_generation,
                "source_model_generations_seen": sorted(report.get("source_model_generation_summary", {}).keys()),
                "trainer_settings_snapshot": {"context": settings},
                "context_training": {
                    "report_path": str(args.out_report),
                    "feature_report_path": str(args.feature_report or DEFAULT_FEATURE_REPORT),
                    "feature_names": list(selected_features),
                    "archive_outputs": archived,
                    "generation_advancement_assessment": report.get("generation_advancement_assessment", {}),
                },
                "recommended_next_action": (
                    f"Trained on {selected_generation}. "
                    f"Canonical context header remains the active deployment target. "
                    + (
                        f"Place next collection raw CSVs in tinyml/data/{next_generation_tag(selected_generation)}/raw."
                        if next_generation_tag(selected_generation)
                        else "Review class coverage before advancing beyond the current generation."
                    )
                ),
            },
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
        "feature_count": len(selected_features),
        "features": selected_features,
    }, indent=2))
    if selected_generation:
        print("Selected generation:", selected_generation)
    print("Saved:", args.out_joblib)
    print("Saved:", args.out_header)
    print("Saved:", args.out_report)


if __name__ == "__main__":
    main()
