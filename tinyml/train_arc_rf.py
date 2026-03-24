import os
import glob
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

DEFAULT_CSV_GLOB = r"tinyml/data/*.csv"
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


def ensure_dir(path):
    d = os.path.dirname(path)
    if d:
        os.makedirs(d, exist_ok=True)


def pick_group_column(df):
    for c in GROUP_COL_CANDIDATES:
        if c in df.columns:
            return c
    return None


def resolve_csv_files(csv_list, csv_glob):
    files = []

    if csv_list:
        for item in csv_list:
            matches = glob.glob(item)
            if matches:
                files.extend(matches)
            elif os.path.isfile(item):
                files.append(item)

    if csv_glob:
        files.extend(glob.glob(csv_glob))

    files = [os.path.abspath(f) for f in files if os.path.isfile(f)]
    files = sorted(list(dict.fromkeys(files)))  # unique, preserve order

    if not files:
        raise ValueError("No CSV files found. Check --csv or --csv_glob.")

    return files


def load_multiple_csvs(csv_files):
    frames = []
    useful_cols = list(dict.fromkeys(FEATURES + [TARGET] + GROUP_COL_CANDIDATES))

    for i, path in enumerate(csv_files):
        print("[%d/%d] Loading: %s" % (i + 1, len(csv_files), path))
        df_part = pd.read_csv(path)

        keep_cols = [c for c in useful_cols if c in df_part.columns]
        if keep_cols:
            df_part = df_part[keep_cols].copy()
        else:
            df_part = df_part.copy()

        base_name = os.path.splitext(os.path.basename(path))[0]
        group_col = pick_group_column(df_part)

        if group_col is None:
            df_part["session_id"] = "file_%04d_%s" % (i + 1, base_name)
        else:
            df_part[group_col] = base_name + "__" + df_part[group_col].astype(str)

        df_part["_source_file"] = os.path.basename(path)
        frames.append(df_part)

    if not frames:
        raise ValueError("No CSV data loaded.")

    df = pd.concat(frames, ignore_index=True)
    print("Combined rows before cleaning: %d" % len(df))
    return df


def clean_df(df):
    missing = [c for c in FEATURES + [TARGET] if c not in df.columns]
    if missing:
        raise ValueError("Missing required columns: %s" % missing)

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
            best = {
                "thr": float(thr),
                "cost": float(cost),
                "tn": int(tn),
                "fp": int(fp),
                "fn": int(fn),
                "tp": int(tp)
            }
    return best


def model_size_estimate(rf):
    return int(sum(est.tree_.node_count for est in rf.estimators_))


def postprocess_m2c_header(c_code):
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

    def repl(m):
        indent = m.group('indent')
        dst = m.group('dst').strip()
        a = m.group('a').strip()
        b = m.group('b').strip()
        return "%sset_output2(%s, %s, %s);" % (indent, dst, a, b)

    c_code = memcpy_pat.sub(repl, c_code)
    return c_code


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", nargs="*", default=None,
                    help="Optional list of CSV files or wildcard patterns")
    ap.add_argument("--csv_glob", default=DEFAULT_CSV_GLOB,
                    help="Glob pattern for CSVs, default: tinyml/data/*.csv")
    ap.add_argument("--out_header", default=OUT_HEADER)
    ap.add_argument("--out_joblib", default=OUT_JOBLIB)
    args = ap.parse_args()

    csv_files = resolve_csv_files(args.csv, args.csv_glob)

    print("CSV files to load:")
    for f in csv_files:
        print(" - %s" % f)

    df = load_multiple_csvs(csv_files)
    df = clean_df(df)

    print("Rows after cleaning: %d" % len(df))
    print("Class counts after cleaning:")
    print(df[TARGET].value_counts(dropna=False))

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
    split_iter = gss.split(X, y, groups=groups)
    train_idx, test_idx = next(split_iter)

    X_train, y_train = X.iloc[train_idx], y.iloc[train_idx]
    X_test, y_test = X.iloc[test_idx], y.iloc[test_idx]
    groups_train = groups.iloc[train_idx]
    groups_test = groups.iloc[test_idx]

    print("Train rows: %d  Test rows: %d" % (len(X_train), len(X_test)))
    print("Train groups: %d  Test groups: %d" % (groups_train.nunique(), groups_test.nunique()))

    base = RandomForestClassifier(
        random_state=42,
        class_weight="balanced_subsample",
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

    n_splits = min(5, groups_train.nunique())
    if n_splits < 2:
        raise ValueError("Need at least 2 distinct groups/sessions for GroupKFold.")

    cv = GroupKFold(n_splits=n_splits)

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
    split_iter2 = gss2.split(X_train, y_train, groups=groups_train)
    tr2_idx, val_idx = next(split_iter2)

    X_tr2, y_tr2 = X_train.iloc[tr2_idx], y_train.iloc[tr2_idx]
    X_val, y_val = X_train.iloc[val_idx], y_train.iloc[val_idx]

    best.fit(X_tr2, y_tr2)
    val_proba = best.predict_proba(X_val)[:, 1]

    thr_info = select_threshold_cost(y_val, val_proba, fn_weight=1000.0, fp_weight=1.0)
    thr = thr_info["thr"]

    print("\n--- Threshold (VALIDATION) ---")
    print("Chosen threshold: %.2f  (cost=%.1f)" % (thr, thr_info["cost"]))
    print("VAL: TN=%d FP=%d FN=%d TP=%d" % (
        thr_info["tn"], thr_info["fp"], thr_info["fn"], thr_info["tp"]
    ))

    best.fit(X_train, y_train)
    test_proba = best.predict_proba(X_test)[:, 1]
    test_pred = (test_proba >= thr).astype(int)

    tn, fp, fn, tp = confusion_matrix(y_test, test_pred).ravel()
    ap_score = average_precision_score(y_test, test_proba)

    print("\n--- FINAL TEST (group-held-out) ---")
    print("AP (avg precision): %.4f" % ap_score)
    print("Threshold used: %.2f" % thr)
    print("TN=%d FP=%d FN=%d TP=%d" % (tn, fp, fn, tp))
    print("Accuracy: %.4f" % accuracy_score(y_test, test_pred))
    print("\nClassification report:\n", classification_report(y_test, test_pred, digits=4))

    ensure_dir(args.out_joblib)
    joblib.dump(best, args.out_joblib)
    print("\nSaved Python model: %s" % args.out_joblib)

    ensure_dir(args.out_header)
    c_code = m2c.export_to_c(best, function_name="arc_rf_predict")
    c_code = postprocess_m2c_header(c_code)
    feat_lines = "\n".join(["// [%d] %s" % (i, name) for i, name in enumerate(FEATURES)])

    header = """#pragma once
// Auto-generated by m2cgen from scikit-learn RandomForest
// Generated on: %s

#define ARC_THRESHOLD %.4f

// Input Feature Order:
%s

// output[0] = Probability of Normal
// output[1] = Probability of Arc Fault

%s
""" % (pd.Timestamp.now(), thr, feat_lines, c_code)

    with open(args.out_header, "w", encoding="utf-8") as f:
        f.write(header)

    print("Saved TinyML header: %s" % args.out_header)


if __name__ == "__main__":
    main()