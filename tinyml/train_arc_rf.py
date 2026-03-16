import os
import argparse
import pandas as pd
import numpy as np

from sklearn.model_selection import GroupShuffleSplit, GroupKFold, RandomizedSearchCV
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import (
    confusion_matrix, classification_report, accuracy_score,
    average_precision_score
)
import joblib
import m2cgen as m2c

# -----------------------------
# Config (edit defaults here)
# -----------------------------
DEFAULT_CSV = r"tinyml/Arcfault_dataset.csv"
OUT_HEADER = r"tinyml/TinyML_RF.h"
OUT_JOBLIB = r"tinyml/TinyML_RF.joblib"

# Must exactly match firmware order in ArcModel.h and DataLogger.csv header.
FEATURES = [
    "spectral_entropy",
    "spectral_flatness",
    "thd_pct",
    "zcv",
    "hf_ratio",
    "hf_var",
    "cyc_var",
    "v_rms",
    "i_rms",
    "temp_c",
]
TARGET = "label_arc"

GROUP_COL_CANDIDATES = ["session_id", "session", "sid"]


def ensure_dir(path: str):
    d = os.path.dirname(path)
    if d:
        os.makedirs(d, exist_ok=True)


def pick_group_column(df: pd.DataFrame) -> str | None:
    for c in GROUP_COL_CANDIDATES:
        if c in df.columns:
            return c
    return None


def clean_df(df: pd.DataFrame) -> pd.DataFrame:
    missing = [c for c in FEATURES + [TARGET] if c not in df.columns]
    if missing:
        raise ValueError(f"Missing required columns: {missing}")

    x = df[FEATURES].replace([np.inf, -np.inf], np.nan)
    mask_ok = x.notna().all(axis=1)
    df = df.loc[mask_ok].copy()

    df["spectral_entropy"] = df["spectral_entropy"].clip(0.0, 1.0)
    df["spectral_flatness"] = df["spectral_flatness"].clip(0.0, 1.0)
    df["thd_pct"] = df["thd_pct"].clip(0.0, 300.0)
    df["zcv"] = df["zcv"].clip(0.0, 50.0)
    df["hf_ratio"] = df["hf_ratio"].clip(0.0, 1.0)
    df["hf_var"] = df["hf_var"].clip(0.0, 1.0)
    df["cyc_var"] = df["cyc_var"].clip(0.0, 1.0)
    df["v_rms"] = df["v_rms"].clip(0.0, 400.0)
    df["i_rms"] = df["i_rms"].clip(0.0, 100.0)
    df["temp_c"] = df["temp_c"].clip(-40.0, 150.0)

    df[TARGET] = df[TARGET].astype(int).clip(0, 1)
    return df


def select_threshold_cost(y_true, y_proba, fn_weight=1000.0, fp_weight=1.0):
    thresholds = np.arange(0.05, 0.96, 0.01)
    best = {"thr": 0.5, "cost": float("inf"), "tn": 0, "fp": 0, "fn": 0, "tp": 0}

    for thr in thresholds:
        y_pred = (y_proba >= thr).astype(int)
        tn, fp, fn, tp = confusion_matrix(y_true, y_pred).ravel()
        cost = fn_weight * fn + fp_weight * fp
        if cost < best["cost"]:
            best = {"thr": float(thr), "cost": float(cost), "tn": tn, "fp": fp, "fn": fn, "tp": tp}

    return best


def model_size_estimate(rf: RandomForestClassifier) -> int:
    return int(sum(est.tree_.node_count for est in rf.estimators_))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default=DEFAULT_CSV)
    ap.add_argument("--out_header", default=OUT_HEADER)
    ap.add_argument("--out_joblib", default=OUT_JOBLIB)
    args = ap.parse_args()

    print(f"Loading dataset: {args.csv}")
    df = pd.read_csv(args.csv)
    df = clean_df(df)

    group_col = pick_group_column(df)
    if group_col is None:
        print("WARNING: No session_id column found. Falling back to random grouping (less reliable).")
        df["_group"] = np.arange(len(df)) // 200
        group_col = "_group"

    groups = df[group_col].astype(str)
    X = df[FEATURES].astype(float)
    y = df[TARGET].astype(int)

    gss = GroupShuffleSplit(n_splits=1, test_size=0.20, random_state=42)
    (train_idx, test_idx), = gss.split(X, y, groups=groups)

    X_train, y_train = X.iloc[train_idx], y.iloc[train_idx]
    X_test, y_test = X.iloc[test_idx], y.iloc[test_idx]
    groups_train = groups.iloc[train_idx]

    print(f"Train rows: {len(X_train)}  Test rows: {len(X_test)}")
    print(f"Train groups: {groups_train.nunique()}  Test groups: {groups.iloc[test_idx].nunique()}")

    base = RandomForestClassifier(
        random_state=42,
        class_weight="balanced",
        n_jobs=-1
    )

    param_dist = {
        "n_estimators": [40, 60, 80, 120],
        "max_depth": [6, 8, 10, 12, None],
        "min_samples_leaf": [1, 2, 4, 6],
        "min_samples_split": [2, 4, 8, 12],
        "max_features": ["sqrt", 0.6, 0.8],
        "bootstrap": [True],
        "ccp_alpha": [0.0, 0.0002, 0.0005, 0.001],
    }

    cv = GroupKFold(n_splits=5)
    search = RandomizedSearchCV(
        estimator=base,
        param_distributions=param_dist,
        n_iter=30,
        scoring="average_precision",
        cv=cv,
        n_jobs=-1,
        verbose=1,
        random_state=42
    )

    print("\n--- Training: RandomizedSearchCV (GroupKFold) ---")
    search.fit(X_train, y_train, groups=groups_train)
    best = search.best_estimator_
    print("Best params:", search.best_params_)
    print("Estimated RF node count:", model_size_estimate(best))

    gss2 = GroupShuffleSplit(n_splits=1, test_size=0.20, random_state=123)
    (tr2_idx, val_idx), = gss2.split(X_train, y_train, groups=groups_train)

    X_tr2, y_tr2 = X_train.iloc[tr2_idx], y_train.iloc[tr2_idx]
    X_val, y_val = X_train.iloc[val_idx], y_train.iloc[val_idx]

    best.fit(X_tr2, y_tr2)
    val_proba = best.predict_proba(X_val)[:, 1]

    thr_info = select_threshold_cost(y_val, val_proba, fn_weight=1000.0, fp_weight=1.0)
    thr = thr_info["thr"]

    print("\n--- Threshold (VALIDATION) ---")
    print(f"Chosen threshold: {thr:.2f}  (cost={thr_info['cost']:.1f})")
    print(f"VAL: TN={thr_info['tn']} FP={thr_info['fp']} FN={thr_info['fn']} TP={thr_info['tp']}")

    best.fit(X_train, y_train)

    test_proba = best.predict_proba(X_test)[:, 1]
    test_pred = (test_proba >= thr).astype(int)

    tn, fp, fn, tp = confusion_matrix(y_test, test_pred).ravel()
    ap_score = average_precision_score(y_test, test_proba)

    print("\n--- FINAL TEST (group-held-out) ---")
    print(f"AP (avg precision): {ap_score:.4f}")
    print(f"Threshold used: {thr:.2f}")
    print(f"TN={tn} FP={fp} FN={fn} TP={tp}")
    print(f"Accuracy: {accuracy_score(y_test, test_pred):.4f}")
    print("\nClassification report:\n", classification_report(y_test, test_pred, digits=4))

    ensure_dir(args.out_joblib)
    joblib.dump(best, args.out_joblib)
    print(f"\nSaved Python model: {args.out_joblib}")

    ensure_dir(args.out_header)
    c_code = m2c.export_to_c(best, function_name="arc_rf_predict")

    feat_lines = "\n".join([f"// [{i}] {name}" for i, name in enumerate(FEATURES)])

    header = f"""#pragma once
// Auto-generated by m2cgen from scikit-learn RandomForest
// Generated on: {pd.Timestamp.now()}

#define ARC_THRESHOLD {thr:.4f}

// Input Feature Order:
{feat_lines}

#ifdef __cplusplus
extern \"C\" {{
#endif

// output[0] = Probability of Normal
// output[1] = Probability of Arc Fault
void arc_rf_predict(double *input, double *output);

#ifdef __cplusplus
}}
#endif

{c_code}
"""
    with open(args.out_header, "w", encoding="utf-8") as f:
        f.write(header)

    print(f"Saved TinyML header: {args.out_header}")


if __name__ == "__main__":
    main()
