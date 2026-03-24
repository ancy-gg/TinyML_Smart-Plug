import os
import glob
import argparse
import pandas as pd
import numpy as np

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

DEFAULT_INPUT_GLOB = r"tinyml/data/*.csv"
DEFAULT_OUTPUT = r"tinyml/merged_arc_dataset.csv"


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
    files = sorted(list(dict.fromkeys(files)))

    if not files:
        raise ValueError("No CSV files found.")

    return files


def load_and_tag_csvs(csv_files):
    frames = []

    for i, path in enumerate(csv_files):
        print("[%d/%d] Loading: %s" % (i + 1, len(csv_files), path))
        df = pd.read_csv(path).copy()

        base = os.path.splitext(os.path.basename(path))[0]
        group_col = pick_group_column(df)

        if group_col is None:
            df["session_id"] = "file_%04d_%s" % (i + 1, base)
        else:
            df[group_col] = base + "__" + df[group_col].astype(str)

        df["_source_file"] = os.path.basename(path)
        frames.append(df)

    if not frames:
        raise ValueError("No CSVs were loaded.")

    merged = pd.concat(frames, ignore_index=True)
    return merged


def coerce_numeric_if_present(df, cols):
    for c in cols:
        if c in df.columns:
            df[c] = pd.to_numeric(df[c], errors="coerce")
    return df


def clean_dataset(df, keep_only_normal_nonarc=True):
    df = df.copy()

    required = FEATURES + [TARGET]
    missing = [c for c in required if c not in df.columns]
    if missing:
        raise ValueError("Missing required columns: %s" % missing)

    maybe_numeric = FEATURES + [
        TARGET, "feat_valid", "current_valid", "adc_fs_hz", "fault_state"
    ]
    df = coerce_numeric_if_present(df, maybe_numeric)

    before = len(df)

    # label must be 0 or 1
    df = df[df[TARGET].isin([0, 1])].copy()

    # remove inf/nan in required features
    x = df[FEATURES].replace([np.inf, -np.inf], np.nan)
    mask_ok = x.notna().all(axis=1)
    df = df.loc[mask_ok].copy()

    # optional validity filters
    if "feat_valid" in df.columns:
        df = df[df["feat_valid"] == 1].copy()

    if "current_valid" in df.columns:
        df = df[df["current_valid"] == 1].copy()

    if "adc_fs_hz" in df.columns:
        df = df[df["adc_fs_hz"] > 0].copy()

    # optional clean non-arc rule
    if keep_only_normal_nonarc and "fault_state" in df.columns:
        df = df[
            (df[TARGET] == 1) |
            ((df[TARGET] == 0) & (df["fault_state"] == 0))
        ].copy()

    # basic physical clipping
    df["cycle_nmse"] = df["cycle_nmse"].clip(0.0, 2.0)
    df["zcv"] = df["zcv"].clip(0.0, 10.0)
    df["zc_dwell_ratio"] = df["zc_dwell_ratio"].clip(0.0, 1.0)
    df["pulse_count_per_cycle"] = df["pulse_count_per_cycle"].clip(0.0, 100.0)
    df["peak_fluct_cv"] = df["peak_fluct_cv"].clip(0.0, 3.0)
    df["midband_residual_rms"] = df["midband_residual_rms"].clip(0.0, 10.0)
    df["hf_band_energy_ratio"] = df["hf_band_energy_ratio"].clip(0.0, 1.0)
    df["wpe_entropy"] = df["wpe_entropy"].clip(0.0, 1.0)
    df["spec_entropy"] = df["spec_entropy"].clip(0.0, 1.0)
    df["thd_i"] = df["thd_i"].clip(0.0, 500.0)

    # remove exact duplicate rows using only meaningful columns
    dedupe_cols = [c for c in FEATURES + [TARGET] if c in df.columns]
    if "session_id" in df.columns:
        dedupe_cols.append("session_id")
    df = df.drop_duplicates(subset=dedupe_cols).copy()

    df[TARGET] = df[TARGET].astype(int)

    print("\nCleaning summary")
    print("Rows before cleaning: %d" % before)
    print("Rows after cleaning : %d" % len(df))
    print("Rows removed        : %d" % (before - len(df)))

    return df


def print_summary(df):
    print("\nDataset summary")
    print("Total rows: %d" % len(df))
    print("\nClass counts:")
    print(df[TARGET].value_counts(dropna=False))

    if "session_id" in df.columns:
        print("\nUnique sessions: %d" % df["session_id"].nunique())

    if "_source_file" in df.columns:
        print("\nRows per source file:")
        print(df["_source_file"].value_counts())

    print("\nFeature min/max:")
    for c in FEATURES:
        print("%-22s min=%10.6f  max=%10.6f" % (c, df[c].min(), df[c].max()))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", nargs="*", default=None,
                    help="Optional specific CSV files or wildcard patterns")
    ap.add_argument("--csv_glob", default=DEFAULT_INPUT_GLOB,
                    help="Default: tinyml/data/*.csv")
    ap.add_argument("--output", default=DEFAULT_OUTPUT)
    ap.add_argument("--allow_nonzero_fault_nonarc", action="store_true",
                    help="If set, keep non-arc rows even when fault_state != 0")
    args = ap.parse_args()

    csv_files = resolve_csv_files(args.csv, args.csv_glob)

    print("CSV files found:")
    for f in csv_files:
        print(" - %s" % f)

    df = load_and_tag_csvs(csv_files)
    df = clean_dataset(
        df,
        keep_only_normal_nonarc=(not args.allow_nonzero_fault_nonarc)
    )
    print_summary(df)

    ensure_dir(args.output)
    df.to_csv(args.output, index=False)
    print("\nSaved merged dataset to: %s" % args.output)


if __name__ == "__main__":
    main()