import os
import argparse
import pandas as pd
import numpy as np
import joblib
import m2cgen as m2c

from sklearn.model_selection import GroupShuffleSplit, GroupKFold, RandomizedSearchCV
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import confusion_matrix, classification_report, accuracy_score, average_precision_score

CSV_PATH = r"tinyml/data/cleaned_data.csv"
OUT_HEADER = r"tinyml/TinyML_RF.h"
OUT_JOBLIB = r"tinyml/TinyML_RF.joblib"

# New preferred 10-feature set
FEATURES = [
    "cycle_nmse", "zcv", "zc_dwell_ratio", "cycle_rms_drop_ratio", "peak_fluct_cv",
    "midband_residual_rms", "hf_band_energy_ratio", "wpe_entropy", "spec_entropy", "thd_i",
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


def normalize_feature_names(df):
    df = df.copy()
    # strip ugly whitespace from headers first
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
        "cycle_nmse": "cycle_nmse",
        "zcv": "zcv",
        "zc_dwell_ratio": "zc_dwell_ratio",
        "cycle_rms_drop_ratio": "cycle_rms_drop_ratio",
        "peak_fluct_cv": "peak_fluct_cv",
        "midband_residual_rms": "midband_residual_rms",
        "hf_band_energy_ratio": "hf_band_energy_ratio",
        "wpe_entropy": "wpe_entropy",
        "spec_entropy": "spec_entropy",
        "thd_i": "thd_i",
        # legacy compatibility
        "pulse_count_per_cycle": "pulse_count_per_cycle",
    }

    rename_map = {}
    for c in df.columns:
        key = str(c).strip()
        if key in aliases:
            rename_map[c] = aliases[key]
    if rename_map:
        df = df.rename(columns=rename_map)

    # backward compatibility: if the new feature is absent but legacy pulse_count exists,
    # use it as a temporary stand-in so training can still proceed.
    if "cycle_rms_drop_ratio" not in df.columns and "pulse_count_per_cycle" in df.columns:
        df["cycle_rms_drop_ratio"] = pd.to_numeric(df["pulse_count_per_cycle"], errors="coerce")

    return df


def clean_df(df, include_invalid=False):
    df = normalize_feature_names(df.copy())

    missing = [c for c in FEATURES + [TARGET] if c not in df.columns]
    if missing:
        raise ValueError("Missing required columns: %s" % missing)

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
            curv = pd.to_numeric(df.get("current_valid", 1), errors="coerce").fillna(1)
            df = df[(pd.to_numeric(df["feat_valid"], errors="coerce") == 1) & (curv == 1)].copy()
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
        "wpe_entropy": 1.0,
        "spec_entropy": 1.0,
        "thd_i": 400.0,
    }
    for c, hi in clip_hi.items():
        df[c] = df[c].clip(0.0, hi)
    df[TARGET] = df[TARGET].astype(int)
    return df


def select_threshold_cost(y_true, y_proba, fn_weight=250.0, fp_weight=1.0):
    thresholds = np.arange(0.05, 0.96, 0.01)
    best = {"thr": 0.5, "cost": float("inf")}
    for thr in thresholds:
        y_pred = (y_proba >= thr).astype(int)
        tn, fp, fn, tp = confusion_matrix(y_true, y_pred).ravel()
        cost = fn_weight * fn + fp_weight * fp
        if cost < best["cost"]:
            best = {"thr": float(thr), "cost": float(cost), "tn": int(tn), "fp": int(fp), "fn": int(fn), "tp": int(tp)}
    return best


def model_size_estimate(rf):
    return int(sum(est.tree_.node_count for est in rf.estimators_))


def postprocess_m2c_header(c_code):
    import re
    c_code = re.sub(r'(^|\n)void\s+add_vectors\s*\(\s*double\s*\*\s*v1\s*,\s*double\s*\*\s*v2\s*,\s*int\s+size\s*,\s*double\s*\*\s*result\s*\)\s*\{',
                    r'\1static inline void add_vectors(double *v1, double *v2, int size, double *result) {', c_code, flags=re.MULTILINE)
    c_code = re.sub(r'(^|\n)void\s+mul_vector_number\s*\(\s*double\s*\*\s*v1\s*,\s*double\s+num\s*,\s*int\s+size\s*,\s*double\s*\*\s*result\s*\)\s*\{',
                    r'\1static inline void mul_vector_number(double *v1, double num, int size, double *result) {', c_code, flags=re.MULTILINE)
    c_code = re.sub(r'(^|\n)void\s+arc_rf_predict\s*\(\s*double\s*\*\s*input\s*,\s*double\s*\*\s*output\s*\)\s*\{',
                    '\\1static inline void set_output2(double *dst, double a, double b) {\n    dst[0] = a;\n    dst[1] = b;\n}\nstatic inline void arc_rf_predict(double * input, double * output) {',
                    c_code, flags=re.MULTILINE)
    memcpy_pat = re.compile(r'(?m)^(?P<indent>\s*)memcpy\(\s*(?P<dst>[A-Za-z_]\w*)\s*,\s*\(\s*double\s*\[\s*\]\s*\)\s*\{\s*(?P<a>[^,{}]+?)\s*,\s*(?P<b>[^{}]+?)\s*\}\s*,\s*2\s*\*\s*sizeof\s*\(\s*double\s*\)\s*\)\s*;\s*$')
    def repl(m):
        return "%sset_output2(%s, %s, %s);" % (m.group('indent'), m.group('dst').strip(), m.group('a').strip(), m.group('b').strip())
    return memcpy_pat.sub(repl, c_code)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default=CSV_PATH)
    ap.add_argument("--out_header", default=OUT_HEADER)
    ap.add_argument("--out_joblib", default=OUT_JOBLIB)
    ap.add_argument("--include_invalid", action="store_true",
                    help="Experimental: train on all finite rows instead of only rf_train_row/feat_valid rows.")
    ap.add_argument("--fn_weight", type=float, default=250.0)
    ap.add_argument("--fp_weight", type=float, default=1.0)
    args = ap.parse_args()

    if not os.path.isfile(args.csv):
        raise ValueError("Merged dataset not found: %s" % args.csv)

    raw_df = pd.read_csv(args.csv)
    raw_df = normalize_feature_names(raw_df)

    for c in ["rf_train_row", "event_like_invalid", "collapse_like", "fast_vdip_like"]:
        if c in raw_df.columns:
            print(f"{c} rows:", int(pd.to_numeric(raw_df[c], errors='coerce').fillna(0).sum()))

    df = clean_df(raw_df, include_invalid=args.include_invalid)
    if df.empty or df[TARGET].nunique() < 2:
        raise ValueError("Training requires both classes in cleaned_data.csv.")

    group_col = pick_group_column(df)
    if group_col is None:
        df["_group"] = np.arange(len(df)) // 200
        group_col = "_group"

    groups = df[group_col].astype(str)
    X = df[FEATURES].astype(float)
    y = df[TARGET].astype(int)

    gss = GroupShuffleSplit(n_splits=1, test_size=0.20, random_state=42)
    train_idx, test_idx = next(gss.split(X, y, groups=groups))
    X_train, y_train = X.iloc[train_idx], y.iloc[train_idx]
    X_test, y_test = X.iloc[test_idx], y.iloc[test_idx]
    groups_train = groups.iloc[train_idx]

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
    n_splits = min(5, groups_train.nunique())
    if n_splits < 2:
        raise ValueError("Need at least 2 distinct sessions in cleaned_data.csv.")
    cv = GroupKFold(n_splits=n_splits)
    search = RandomizedSearchCV(base, param_dist, n_iter=30, scoring="average_precision", cv=cv, n_jobs=-1, verbose=1, random_state=42)
    search.fit(X_train, y_train, groups=groups_train)
    best = search.best_estimator_
    print("Best params:", search.best_params_)
    print("Estimated RF node count:", model_size_estimate(best))

    y_score = best.predict_proba(X_test)[:, 1]
    best_thr = select_threshold_cost(y_test.to_numpy(), y_score, fn_weight=args.fn_weight, fp_weight=args.fp_weight)
    y_pred = (y_score >= best_thr["thr"]).astype(int)
    print("Threshold:", best_thr)
    print("Accuracy:", accuracy_score(y_test, y_pred))
    print("Average precision:", average_precision_score(y_test, y_score))
    print(confusion_matrix(y_test, y_pred))
    print(classification_report(y_test, y_pred, digits=4))

    ensure_dir(args.out_joblib)
    ensure_dir(args.out_header)
    joblib.dump(best, args.out_joblib)

    c_code = m2c.export_to_c(best, function_name="arc_rf_predict")
    c_code = postprocess_m2c_header(c_code)
    with open(args.out_header, "w", encoding="utf-8") as f:
        f.write("#pragma once\n")
        f.write("// Auto-generated by m2cgen from scikit-learn RandomForest\n")
        f.write("#define ARC_THRESHOLD %.4f\n\n" % best_thr["thr"])
        f.write("#include <string.h>\n")
        f.write(c_code)
    print("Saved:", args.out_joblib)
    print("Saved:", args.out_header)


if __name__ == "__main__":
    main()
