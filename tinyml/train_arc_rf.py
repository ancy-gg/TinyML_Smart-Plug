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

DEFAULT_CSV = r"tinyml/gen_zero_scaffold_data.csv"
OUT_HEADER = r"tinyml/TinyML_RF.h"
OUT_JOBLIB = r"tinyml/TinyML_RF.joblib"

FEATURES = [
    "cycle_nmse",
    "zcv",
    "zc_dwell_ratio",
    "pulse_count_per_cycle",
    "peak_fluct_cv",
    "midband_residual_rms",
    "hf_band_energy_ratio",
    "wpe_entropy",
    "spec_entropy",
    "thd_i",
]
TARGET = "label_arc"
GROUP_COL_CANDIDATES = ["session_id", "session", "sid"]


def ensure_dir(path: str):
    d = os.path.dirname(path)
    if d:
        os.makedirs(d, exist_ok=True)


def pick_group_column(df: pd.DataFrame):
    for c in GROUP_COL_CANDIDATES:
        if c in df.columns:
            return c
    return None


def clean_df(df: pd.DataFrame) -> pd.DataFrame:
    missing = [c for c in FEATURES + [TARGET] if c not in df.columns]
    if missing:
        raise ValueError(f"Missing required columns: {missing}")

    df = df.copy()
    df[TARGET] = pd.to_numeric(df[TARGET], errors="coerce")
    df = df[df[TARGET].isin([0, 1])].copy()

    x = df[FEATURES].replace([np.inf, -np.inf], np.nan)
    mask_ok = x.notna().all(axis=1)
    df = df.loc[mask_ok].copy()

    df["cycle_nmse"] = df["cycle_nmse"].clip(0.0, 2.0)
    df["zcv"] = df["zcv"].clip(0.0, 10.0)
    df["zc_dwell_ratio"] = df["zc_dwell_ratio"].clip(0.0, 1.0)
    df["pulse_count_per_cycle"] = df["pulse_count_per_cycle"].clip(0.0, 50.0)
    df["peak_fluct_cv"] = df["peak_fluct_cv"].clip(0.0, 2.0)
    df["midband_residual_rms"] = df["midband_residual_rms"].clip(0.0, 5.0)
    df["hf_band_energy_ratio"] = df["hf_band_energy_ratio"].clip(0.0, 1.0)
    df["wpe_entropy"] = df["wpe_entropy"].clip(0.0, 1.0)
    df["spec_entropy"] = df["spec_entropy"].clip(0.0, 1.0)
    df["thd_i"] = df["thd_i"].clip(0.0, 400.0)

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


def postprocess_m2c_header(c_code: str) -> str:
    """Make m2cgen RandomForest output compile cleanly as a C++ header.

    m2cgen emits C compound literals in some memcpy calls, e.g.
        memcpy(var80, (double[]){1.0, 0.0}, 2 * sizeof(double));
    which is valid in C but not in this PlatformIO C++ build.
    Replace those with a tiny helper and also make helper functions inline.
    """
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
        '\\1static inline void set_output2(double *dst, double a, double b) {\n'
        '    dst[0] = a;\n'
        '    dst[1] = b;\n'
        '}\n'
        'static inline void arc_rf_predict(double * input, double * output) {',
        c_code,
        flags=re.MULTILINE,
    )

    memcpy_pat = re.compile(
        r'(?m)^(?P<indent>\s*)memcpy\(\s*'
        r'(?P<dst>[A-Za-z_]\w*)\s*,\s*'
        r'\(\s*double\s*\[\s*\]\s*\)\s*\{\s*'
        r'(?P<a>[^,{}]+?)\s*,\s*(?P<b>[^{}]+?)\s*\}\s*,\s*'
        r'2\s*\*\s*sizeof\s*\(\s*double\s*\)\s*\)\s*;\s*$'
    )

    def repl(m: re.Match) -> str:
        indent = m.group('indent')
        dst = m.group('dst').strip()
        a = m.group('a').strip()
        b = m.group('b').strip()
        return f"{indent}set_output2({dst}, {a}, {b});"

    c_code = memcpy_pat.sub(repl, c_code)
    return c_code


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default=DEFAULT_CSV)
    ap.add_argument("--out_header", default=OUT_HEADER)
    ap.add_argument("--out_joblib", default=OUT_JOBLIB)
    args = ap.parse_args()

    print(f"Loading dataset: {args.csv}")
    df = pd.read_csv(args.csv)
    df = clean_df(df)

    if df.empty:
        raise ValueError("No labeled rows remain after filtering. label_arc must be 0 or 1.")
    if df[TARGET].nunique() < 2:
        raise ValueError("Training requires both classes. Provide at least one non-arc row and one arc row.")

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

    base = RandomForestClassifier(random_state=42, class_weight="balanced_subsample", n_jobs=-1)

    param_dist = {
        "n_estimators": [40, 60, 80, 120],
        "max_depth": [6, 8, 10, 12, None],
        "min_samples_leaf": [1, 2, 4, 6],
        "min_samples_split": [2, 4, 8, 12],
        "max_features": ["sqrt", 0.6, 0.8],
        "bootstrap": [True],
        "ccp_alpha": [0.0, 0.0002, 0.0005, 0.001],
    }

    cv = GroupKFold(n_splits=min(5, groups_train.nunique()))
    search = RandomizedSearchCV(
        estimator=base,
        param_distributions=param_dist,
        n_iter=30,
        scoring="average_precision",
        cv=cv,
        n_jobs=-1,
        verbose=1,
        random_state=42,
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
    c_code = postprocess_m2c_header(c_code)
    feat_lines = "\n".join([f"// [{i}] {name}" for i, name in enumerate(FEATURES)])

    header = f"""#pragma once
// Auto-generated by m2cgen from scikit-learn RandomForest
// Generated on: {pd.Timestamp.now()}

#define ARC_THRESHOLD {thr:.4f}

// Input Feature Order:
{feat_lines}

// output[0] = Probability of Normal
// output[1] = Probability of Arc Fault

{c_code}
"""
    with open(args.out_header, "w", encoding="utf-8") as f:
        f.write(header)

    print(f"Saved TinyML header: {args.out_header}")


if __name__ == "__main__":
    main()
