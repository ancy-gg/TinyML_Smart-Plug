import os
import glob
import argparse
import pandas as pd
import numpy as np

FEATURES = [
    "cycle_nmse", "zcv", "zc_dwell_ratio", "cycle_rms_drop_ratio", "peak_fluct_cv",
    "midband_residual_rms", "hf_band_energy_ratio", "spec_entropy", "neg_dip_event_ratio", "pre_dip_spike_ratio",
]
TARGET = "label_arc"
GROUP_COL_CANDIDATES = ["session_id", "session", "sid"]
DEFAULT_INPUT_GLOB = r"tinyml/data/raw/*.csv"
DEFAULT_OUTPUT = r"tinyml/data/cleaned_data.csv"


def ensure_dir(path):
    d = os.path.dirname(path)
    if d:
        os.makedirs(d, exist_ok=True)


def pick_group_column(df):
    for c in GROUP_COL_CANDIDATES:
        if c in df.columns:
            return c
    return None


def resolve_csv_files(csv_glob, output_path):
    files = []
    out_abs = os.path.abspath(output_path)
    for f in sorted(glob.glob(csv_glob)):
        if os.path.abspath(f) == out_abs:
            continue
        if os.path.isfile(f):
            files.append(os.path.abspath(f))
    if not files:
        raise ValueError("No CSV files found in tinyml/data.")
    return files


def normalize_feature_names(df):
    df = df.copy()
    df.columns = [str(c).strip() for c in df.columns]
    if "cycle_rms_drop_ratio" not in df.columns and "pulse_count_per_cycle" in df.columns:
        df["cycle_rms_drop_ratio"] = pd.to_numeric(df["pulse_count_per_cycle"], errors="coerce")
        df["legacy_pulse_feature"] = 1
    elif "cycle_rms_drop_ratio" in df.columns:
        df["legacy_pulse_feature"] = 0

    for missing in ["neg_dip_event_ratio", "pre_dip_spike_ratio", "thd_i"]:
        if missing not in df.columns:
            df[missing] = 0.0
    return df


def load_and_tag_csvs(csv_files):
    frames = []
    for i, path in enumerate(csv_files):
        print("[%d/%d] Loading: %s" % (i + 1, len(csv_files), path))
        df = normalize_feature_names(pd.read_csv(path).copy())
        base = os.path.splitext(os.path.basename(path))[0]
        group_col = pick_group_column(df)
        if group_col is None:
            df["session_id"] = "file_%04d_%s" % (i + 1, base)
        else:
            df[group_col] = base + "__" + df[group_col].astype(str)
        df["_source_file"] = os.path.basename(path)
        frames.append(df)
    return pd.concat(frames, ignore_index=True)


def coerce_numeric_if_present(df, cols):
    for c in cols:
        if c in df.columns:
            df[c] = pd.to_numeric(df[c], errors="coerce")
    return df


def clean_dataset(df):
    df = df.copy()
    required = FEATURES + [TARGET]
    missing = [c for c in required if c not in df.columns]
    if missing:
        raise ValueError("Missing required columns: %s" % missing)

    maybe_numeric = FEATURES + [TARGET, "feat_valid", "current_valid", "adc_fs_hz", "fault_state", "thd_i"]
    df = coerce_numeric_if_present(df, maybe_numeric)
    before = len(df)
    df = df[df[TARGET].isin([0, 1])].copy()
    x = df[FEATURES].replace([np.inf, -np.inf], np.nan)
    df = df.loc[x.notna().all(axis=1)].copy()
    if "current_valid" in df.columns:
        df = df[df["current_valid"] == 1].copy()
    if "adc_fs_hz" in df.columns:
        df = df[df["adc_fs_hz"] > 0].copy()

    if "fault_state" in df.columns:
        df["fault_state"] = pd.to_numeric(df["fault_state"], errors="coerce").fillna(0).astype(int)

    if {"feat_valid", "current_valid", "v_rms", "i_rms"}.issubset(df.columns):
        df["event_like_invalid"] = (
            (df["feat_valid"] == 0) &
            (df["current_valid"] == 1) &
            (df["v_rms"] >= 170.0)
        ).astype(int)

    if "feat_valid" in df.columns:
        df = df[df["feat_valid"] == 1].copy()

    clip_hi = {
        "cycle_nmse": 2.0,
        "zcv": 10.0,
        "zc_dwell_ratio": 1.0,
        "cycle_rms_drop_ratio": 100.0,
        "peak_fluct_cv": 3.0,
        "midband_residual_rms": 10.0,
        "hf_band_energy_ratio": 1.0,
        "spec_entropy": 1.0,
        "neg_dip_event_ratio": 1.0,
        "pre_dip_spike_ratio": 2.0,
        "thd_i": 500.0,
    }
    for c, hi in clip_hi.items():
        if c in df.columns:
            df[c] = df[c].clip(0.0, hi)

    dedupe_cols = [c for c in FEATURES + [TARGET] if c in df.columns]
    if "session_id" in df.columns:
        dedupe_cols.append("session_id")
    df = df.drop_duplicates(subset=dedupe_cols).copy()
    df[TARGET] = df[TARGET].astype(int)

    print("Rows before cleaning:", before)
    print("Rows after cleaning :", len(df))
    print("Rows removed        :", before - len(df))
    return df


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv_glob", default=DEFAULT_INPUT_GLOB)
    ap.add_argument("--output", default=DEFAULT_OUTPUT)
    args = ap.parse_args()

    csv_files = resolve_csv_files(args.csv_glob, args.output)
    df = load_and_tag_csvs(csv_files)
    df = clean_dataset(df)
    ensure_dir(args.output)
    df.to_csv(args.output, index=False)
    print("Saved merged dataset to:", args.output)


if __name__ == "__main__":
    main()
