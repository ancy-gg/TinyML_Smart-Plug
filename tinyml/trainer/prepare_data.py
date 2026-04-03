import argparse
import glob
import os
from pathlib import Path

import numpy as np
import pandas as pd

from tinyml_common import FEATURES, TARGET, ensure_dir, pick_group_column


SCRIPT_DIR = Path(__file__).resolve().parent
TINYML_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TINYML_DIR.parent

DEFAULT_INPUT_GLOB = str(PROJECT_ROOT / "tinyml" / "data" / "raw" / "*.csv")
DEFAULT_OUTPUT = str(PROJECT_ROOT / "tinyml" / "data" / "cleaned_data.csv")

REAL_NEG_WEIGHT = 3.5
REAL_POS_WEIGHT = 5.0
SCAFFOLD_NEG_WEIGHT = 0.35
SCAFFOLD_POS_WEIGHT = 0.45
TRANSIENT_NORMAL_WEIGHT = 2.0


def resolve_csv_files(csv_glob: str, output_path: str):
    files = []
    out_abs = os.path.abspath(output_path)
    for path in sorted(glob.glob(csv_glob)):
        if os.path.abspath(path) == out_abs:
            continue
        if os.path.isfile(path):
            files.append(os.path.abspath(path))
    if not files:
        raise ValueError("No CSV files found in tinyml/data.")
    return files


def normalize_feature_names(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()
    df.columns = [str(c).strip() for c in df.columns]
    if (
        "cycle_rms_drop_ratio" not in df.columns
        and "pulse_count_per_cycle" in df.columns
    ):
        df["cycle_rms_drop_ratio"] = pd.to_numeric(
            df["pulse_count_per_cycle"],
            errors="coerce",
        )
        df["legacy_pulse_feature"] = 1
    elif "cycle_rms_drop_ratio" in df.columns:
        df["legacy_pulse_feature"] = 0

    if "irms_drop_vs_baseline" not in df.columns and "pre_dip_spike_ratio" in df.columns:
        df["irms_drop_vs_baseline"] = pd.to_numeric(
            df["pre_dip_spike_ratio"],
            errors="coerce",
        )

    for missing in ["neg_dip_event_ratio", "irms_drop_vs_baseline", "thd_i"]:
        if missing not in df.columns:
            df[missing] = 0.0
    return df


def infer_source_kind(path: str):
    name = os.path.basename(path).lower()
    if "scaffold" in name:
        return "scaffold", 1
    return "real", 3


def load_and_tag_csvs(csv_files):
    frames = []
    for i, path in enumerate(csv_files):
        print(f"[{i + 1}/{len(csv_files)}] Loading: {path}")
        df = normalize_feature_names(pd.read_csv(path).copy())
        base = os.path.splitext(os.path.basename(path))[0]
        group_col = pick_group_column(df)

        if group_col is None:
            df["session_id"] = f"file_{i + 1:04d}_{base}"
        else:
            df[group_col] = base + "__" + df[group_col].astype(str)

        source_kind, source_priority = infer_source_kind(path)
        df["_source_file"] = os.path.basename(path)
        df["source_kind"] = source_kind
        df["source_priority"] = source_priority
        if "rf_train_row" not in df.columns:
            df["rf_train_row"] = 1
        frames.append(df)

    return pd.concat(frames, ignore_index=True)


def coerce_numeric_if_present(df: pd.DataFrame, cols):
    for col in cols:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")
    return df


def attach_training_metadata(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()

    if "i_rms" not in df.columns:
        df["i_rms"] = np.nan
    if "v_rms" not in df.columns:
        df["v_rms"] = np.nan

    zero_or_off = (
        pd.to_numeric(df["i_rms"], errors="coerce").fillna(0.0) < 0.08
    ) | (
        pd.to_numeric(df["v_rms"], errors="coerce").fillna(0.0) < 170.0
    )

    if "feat_valid" in df.columns:
        zero_or_off |= (
            pd.to_numeric(df["feat_valid"], errors="coerce").fillna(0) != 1
        )
    if "current_valid" in df.columns:
        zero_or_off |= (
            pd.to_numeric(df["current_valid"], errors="coerce").fillna(0) != 1
        )

    df.loc[zero_or_off, TARGET] = 0
    df.loc[zero_or_off, "rf_train_row"] = 0

    df["sample_weight"] = np.where(
        df["source_kind"] == "real",
        np.where(df[TARGET] == 1, REAL_POS_WEIGHT, REAL_NEG_WEIGHT),
        np.where(df[TARGET] == 1, SCAFFOLD_POS_WEIGHT, SCAFFOLD_NEG_WEIGHT),
    ).astype(float)

    if "session_id" in df.columns and "i_rms" in df.columns:
        sort_cols = (
            ["session_id", "epoch_ms"]
            if "epoch_ms" in df.columns
            else ["session_id"]
        )
        df = df.sort_values(sort_cols).copy()
        di = df.groupby("session_id")["i_rms"].diff().abs().fillna(0.0)
        turn_like = (di >= 0.60) | (
            (df["i_rms"] >= 0.10) & (df["i_rms"] <= 0.50)
        )
        transient_norm = (
            (df[TARGET] == 0)
            & (df["rf_train_row"] == 1)
            & turn_like
        )
        df.loc[transient_norm, "sample_weight"] = np.maximum(
            df.loc[transient_norm, "sample_weight"],
            TRANSIENT_NORMAL_WEIGHT,
        )
        df["transition_normal"] = transient_norm.astype(int)
    else:
        df["transition_normal"] = 0

    sig_cols = [c for c in FEATURES if c in df.columns]
    rounded = df[sig_cols].round(4).astype(str)
    df["_feature_sig"] = rounded.agg("|".join, axis=1)
    if TARGET in df.columns:
        df["_feature_sig"] = (
            df["_feature_sig"] + "|y=" + df[TARGET].astype(str)
        )

    df = df.sort_values(
        ["source_priority", "sample_weight"],
        ascending=[False, False],
    ).copy()
    df = df.drop_duplicates(subset=["_feature_sig"], keep="first").copy()
    return df


def clean_dataset(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()
    required = FEATURES + [TARGET]
    missing = [c for c in required if c not in df.columns]
    if missing:
        raise ValueError(f"Missing required columns: {missing}")

    maybe_numeric = FEATURES + [
        TARGET,
        "feat_valid",
        "current_valid",
        "adc_fs_hz",
        "fault_state",
        "thd_i",
        "i_rms",
        "v_rms",
        "temp_c",
        "epoch_ms",
        "rf_train_row",
    ]
    df = coerce_numeric_if_present(df, maybe_numeric)
    before = len(df)

    df = df[df[TARGET].isin([0, 1])].copy()
    x = df[FEATURES].replace([np.inf, -np.inf], np.nan)
    df = df.loc[x.notna().all(axis=1)].copy()

    if "adc_fs_hz" in df.columns:
        df = df[df["adc_fs_hz"] > 0].copy()

    clip_hi = {
        "cycle_nmse": 2.0,
        "zcv": 10.0,
        "zc_dwell_ratio": 1.0,
        "cycle_rms_drop_ratio": 1.0,
        "peak_fluct_cv": 3.0,
        "midband_residual_rms": 10.0,
        "hf_band_energy_ratio": 1.0,
        "spec_entropy": 1.0,
        "neg_dip_event_ratio": 1.0,
        "irms_drop_vs_baseline": 2.0,
        "thd_i": 500.0,
    }
    for c, hi in clip_hi.items():
        if c in df.columns:
            df[c] = df[c].clip(0.0, hi)

    df = attach_training_metadata(df)

    dedupe_cols = [c for c in FEATURES + [TARGET] if c in df.columns]
    if "session_id" in df.columns:
        dedupe_cols.append("session_id")
    df = df.sort_values(
        ["source_priority", "sample_weight"],
        ascending=[False, False],
    ).drop_duplicates(subset=dedupe_cols, keep="first").copy()

    df[TARGET] = df[TARGET].astype(int)
    df["rf_train_row"] = pd.to_numeric(
        df["rf_train_row"],
        errors="coerce",
    ).fillna(0).astype(int)

    print("Rows before cleaning:", before)
    print("Rows after cleaning :", len(df))
    print("Rows removed        :", before - len(df))
    print("Trainable rows      :", int(df["rf_train_row"].sum()))
    print("Source counts:")
    print(df["source_kind"].value_counts().to_string())
    print("Label counts:")
    print(df[TARGET].value_counts().to_string())
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
