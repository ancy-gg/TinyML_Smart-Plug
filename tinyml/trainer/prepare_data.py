import argparse
import json
import glob
import os
import re
import sys
import time
from pathlib import Path

import numpy as np
import pandas as pd

from tinyml_common import (
    FEATURES,
    ARC_SWEEP_FEATURES,
    CONTEXT_SWEEP_FEATURES,
    TARGET,
    annotate_generation_and_mismatch_columns,
    build_mismatch_summary,
    ensure_dir,
    pick_group_column,
)
try:
    from generation_paths import (
        apply_generation_defaults,
        cli_flag_present,
        ensure_generation_dirs,
        normalize_generation_tag,
        resolve_generation_paths,
        update_generation_manifest,
        validate_generation_workspace,
    )
except Exception:
    from trainer.generation_paths import (
        apply_generation_defaults,
        cli_flag_present,
        ensure_generation_dirs,
        normalize_generation_tag,
        resolve_generation_paths,
        update_generation_manifest,
        validate_generation_workspace,
    )


SCRIPT_DIR = Path(__file__).resolve().parent
TINYML_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TINYML_DIR.parent

DEFAULT_INPUT_GLOB = str(PROJECT_ROOT / "tinyml" / "data" / "raw" / "**" / "*.csv")
DEFAULT_OUTPUT = str(PROJECT_ROOT / "tinyml" / "data" / "arc_training.csv")
DEFAULT_CONTEXT_OUTPUT = str(PROJECT_ROOT / "tinyml" / "data" / "load_context.csv")
DEFAULT_REPORT = str(PROJECT_ROOT / "tinyml" / "benchmark" / "prepare_data_report.json")

REAL_NEG_WEIGHT = 3.5
REAL_POS_WEIGHT = 5.0
SCAFFOLD_NEG_WEIGHT = 0.35
SCAFFOLD_POS_WEIGHT = 0.45
TRANSIENT_NORMAL_WEIGHT = 2.0
TRUSTED_NORMAL_WEIGHT = 6.0

SCAFFOLD_MAX_SHARE_OF_REAL = 0.45
SCAFFOLD_MAX_BUCKET_WEIGHT_NO_REAL = 6.0
SCAFFOLD_MIN_ROW_WEIGHT = 0.05

DEFAULT_NEG_LABEL_TRUST = 0.95
DEFAULT_POS_LABEL_TRUST = 0.45
TRUSTED_NORMAL_LABEL_TRUST = 1.25

CONFLICT_ROUND_DECIMALS = 3
NORMAL_PREFERENCE_MARGIN = 0.20
ARC_PREFERENCE_MARGIN = 0.35

VERY_WEAK_POSITIVE_SCORE = 0.18
SOFT_POSITIVE_SCORE = 0.35
SOFT_NEGATIVE_ARC_SCORE = 0.70
STRONG_NEGATIVE_ARC_SCORE = 0.90

TRUSTED_NORMAL_NAME_TOKENS = (
    "normalonly",
    "normal_only",
    "knownnormal",
    "known_normal",
    "clean_normal",
    "clean-normal",
    "baseline",
    "steady",
    "nominal",
    "noarc",
    "no_arc",
    "arcfree",
    "arc_free",
)

FILENAME_DIVISION_ALIASES = {
    "startup": "start",
    "start": "start",
    "steady": "steady",
    "baseline": "steady",
    "arc": "arc",
    "close": "steady",
    "closing": "steady",
    "end": "steady",
    "ending": "steady",
}

FILENAME_SUFFIX_TOKENS = {
    "capture",
    "captured",
    "processed",
    "process",
    "labeled",
    "labelled",
    "edited",
    "edit",
    "review",
    "original",
    "raw",
}

FILENAME_PREFIX_TOKENS = {
    "proc",
    "processed",
    "upload",
    "uploaded",
    "sess",
    "session",
    "logger",
    "log",
}

DB_FEATURE_SPACE_VERSION = 9
DB_RATIO_FLOOR = 1e-6
DB_POWER_RATIO_FLOOR = 1e-6
DB_RATIO_CLIP = (-80.0, 20.0)
DB_RESIDUAL_CF_CLIP = (-20.0, 40.0)
DB_THD_CLIP = (0.0, 200.0)
DB_HF_CLIP = (-18.0, 18.0)

FEATURE_CLIP_BOUNDS = {
    "spectral_flux_midhf": (0.0, 200.0),
    "residual_crest_factor": DB_RESIDUAL_CF_CLIP,
    "edge_spike_ratio": DB_RATIO_CLIP,
    "midband_residual_ratio": DB_RATIO_CLIP,
    "cycle_nmse": (0.0, 200.0),
    "peak_fluct_cv": (0.0, 300.0),
    "thd_i": DB_THD_CLIP,
    "hf_energy_delta": DB_HF_CLIP,
    "zcv": (0.0, 10.0),
    "abs_irms_zscore_vs_baseline": (0.0, 25.0),
    "suspicious_run_energy": (0.0, 20.0),
    "delta_irms_abs": (0.0, 15.0),
    "delta_hf_energy": (0.0, 24.0),
    "delta_flux": (0.0, 200.0),
    "halfcycle_asymmetry": (0.0, 200.0),
    "v_sag_pct": (0.0, 100.0),
    "pulse_count_per_cycle": (0.0, 16.0),
    "zero_dwell_ratio": (0.0, 100.0),
    "low_current_ratio": (0.0, 100.0),
    "max_low_current_run_ms": (0.0, 25.0),
    "context_family_confidence": (0.0, 1.0),
    "v_rms": (0.0, 400.0),
    "i_rms": (0.0, 40.0),
}

DB_ARC_THRESHOLDS = {
    "residual_crest_factor": 12.568,
    "edge_spike_ratio": -14.894,
    "midband_residual_ratio": -21.412,
    "thd_i": 22.0,
    "hf_energy_delta": 1.500,
}

DB_NORMAL_ANCHORS = {
    "residual_crest_factor": 6.0,
    "edge_spike_ratio": -28.0,
    "midband_residual_ratio": -34.0,
    "thd_i": 6.0,
    "hf_energy_delta": 0.0,
}

DB_ALIAS_MAP = {
    "residual_crest_factor_db": "residual_crest_factor",
    "edge_spike_ratio_db": "edge_spike_ratio",
    "midband_residual_ratio_db": "midband_residual_ratio",
    "thd_i_db": "thd_i",
    "hf_energy_delta_db": "hf_energy_delta",
}

DEVICE_FAMILY_CLASSES = (
    "resistive_linear",
    "inductive_motor",
    "rectifier_smps",
    "phase_angle_controlled",
    "brush_universal_motor",
    "other_mixed",
)


def preferred_time_sort_cols(df: pd.DataFrame, session_first: bool = False) -> list[str]:
    cols = []
    if session_first and "session_id" in df.columns:
        cols.append("session_id")
    for col in ("frame_start_uptime_ms", "frame_end_uptime_ms", "uptime_ms", "epoch_ms"):
        if col in df.columns and col not in cols:
            cols.append(col)
    if session_first and not cols:
        cols = ["session_id"]
    return cols




def normalize_device_family_token(value, fallback: str = "unknown") -> str:
    raw = re.sub(r"[^a-z0-9]+", "_", str(value or "").strip().lower()).strip("_")
    alias = {
        "resistive": "resistive_linear", "resistive_linear": "resistive_linear", "heater": "resistive_linear", "heating": "resistive_linear",
        "inductive": "inductive_motor", "motor": "inductive_motor", "fan": "inductive_motor", "inductive_motor": "inductive_motor",
        "smps": "rectifier_smps", "rectifier": "rectifier_smps", "rectifier_smps": "rectifier_smps", "charger": "rectifier_smps", "adapter": "rectifier_smps",
        "dimmer": "phase_angle_controlled", "phase": "phase_angle_controlled", "dimmer_phase": "phase_angle_controlled", "phase_angle": "phase_angle_controlled", "phase_angle_controlled": "phase_angle_controlled",
        "universal": "brush_universal_motor", "universal_motor": "brush_universal_motor", "brush": "brush_universal_motor", "brush_universal_motor": "brush_universal_motor", "vacuum": "brush_universal_motor",
        "mixed": "other_mixed", "mixed_unknown": "other_mixed", "other": "other_mixed", "other_mixed": "other_mixed", "unknown": "unknown",
    }
    out = alias.get(raw, raw or fallback)
    return out if out in DEVICE_FAMILY_CLASSES else fallback


def normalize_device_name_token(value, fallback: str = "unknown_device") -> str:
    raw = re.sub(r"\.[a-z0-9]+$", "", str(value or "").strip(), flags=re.I)
    token = re.sub(r"[^a-z0-9]+", "_", raw.lower()).strip("_")
    return token or fallback


def normalize_division_tag(value, fallback: str = "steady") -> str:
    raw = re.sub(r"[^a-z0-9]+", "_", str(value or "").strip().lower()).strip("_")
    return FILENAME_DIVISION_ALIASES.get(raw, raw or fallback) or fallback

def device_family_code_from_token(value) -> int:
    token = normalize_device_family_token(value, "unknown")
    if token == "unknown":
        return -1
    return DEVICE_FAMILY_CLASSES.index(token) if token in DEVICE_FAMILY_CLASSES else -1


def _series_from_default(index, values, default=0.0, dtype=float) -> pd.Series:
    if isinstance(values, pd.Series):
        return pd.to_numeric(values, errors="coerce")
    return pd.Series(default if values is None else values, index=index, dtype=dtype)


def _scaffold_mask(df: pd.DataFrame) -> pd.Series:
    source = df.get("source_kind", pd.Series("real", index=df.index)).astype(str).str.lower().str.strip()
    mask = source.eq("scaffold") | source.str.contains("scaffold", na=False)
    if "is_scaffold" in df.columns:
        mask |= pd.to_numeric(df["is_scaffold"], errors="coerce").fillna(0).astype(int) == 1
    return pd.Series(mask, index=df.index, dtype=bool)


def apply_scaffold_gap_fill_weights(df: pd.DataFrame) -> tuple[pd.DataFrame, dict]:
    work = df.copy()
    if work.empty:
        return work, {"scaffold_rows": 0, "scaffold_scaled_rows": 0, "scaffold_effective_weight": 0.0}

    work["sample_weight"] = pd.to_numeric(
        _series_from_default(work.index, work.get("sample_weight", None), default=1.0),
        errors="coerce"
    ).fillna(1.0).clip(SCAFFOLD_MIN_ROW_WEIGHT, 100.0)

    scaffold_mask = _scaffold_mask(work)
    work["is_scaffold"] = scaffold_mask.astype(int)
    work["scaffold_weight_scale"] = pd.to_numeric(
        _series_from_default(work.index, work.get("scaffold_weight_scale", None), default=1.0),
        errors="coerce"
    ).fillna(1.0)
    work["scaffold_cap_reason"] = work.get(
        "scaffold_cap_reason",
        pd.Series("", index=work.index, dtype=object)
    ).astype(str)

    trainable_mask = pd.to_numeric(
        _series_from_default(work.index, work.get("rf_train_row", None), default=1),
        errors="coerce"
    ).fillna(1).astype(int) == 1

    def _apply(cols: list[str], tag: str):
        if not cols:
            return

        eligible = work.loc[trainable_mask].copy()
        if eligible.empty:
            return

        # IMPORTANT: iterate grouped frames directly so we keep original index labels
        for _, g in eligible.groupby(cols, dropna=False, sort=False):
            idx = g.index
            s_mask = scaffold_mask.loc[idx]

            if not bool(s_mask.any()):
                continue

            real_sum = float(pd.to_numeric(
                g.loc[~s_mask, "sample_weight"], errors="coerce"
            ).fillna(0.0).sum())

            scaff_sum = float(pd.to_numeric(
                g.loc[s_mask, "sample_weight"], errors="coerce"
            ).fillna(0.0).sum())

            if scaff_sum <= 0.0:
                continue

            if real_sum > 0.0:
                allowed = max(SCAFFOLD_MIN_ROW_WEIGHT, SCAFFOLD_MAX_SHARE_OF_REAL * real_sum)
                reason = f"cap_vs_real_{tag}"
            else:
                allowed = SCAFFOLD_MAX_BUCKET_WEIGHT_NO_REAL
                reason = f"gap_fill_only_{tag}"

            scale = min(1.0, allowed / max(scaff_sum, 1e-9))
            if scale >= 0.999999:
                continue

            s_idx = g.index[s_mask.to_numpy()]

            work.loc[s_idx, "sample_weight"] = np.maximum(
                SCAFFOLD_MIN_ROW_WEIGHT,
                pd.to_numeric(work.loc[s_idx, "sample_weight"], errors="coerce").fillna(0.0) * scale
            )
            work.loc[s_idx, "scaffold_weight_scale"] = np.minimum(
                pd.to_numeric(work.loc[s_idx, "scaffold_weight_scale"], errors="coerce").fillna(1.0),
                scale
            )

            empty_reason = work.loc[s_idx, "scaffold_cap_reason"].astype(str).str.len() == 0
            work.loc[s_idx[empty_reason.to_numpy()], "scaffold_cap_reason"] = reason

    family_cols = [c for c in ["device_family", TARGET] if c in work.columns]
    device_cols = [c for c in ["device_family", "device_name", TARGET] if c in work.columns]

    _apply(family_cols, "family")
    _apply(device_cols, "device")

    summary = {
        "scaffold_rows": int(scaffold_mask.sum()),
        "scaffold_trainable_rows": int((scaffold_mask & trainable_mask).sum()),
        "scaffold_scaled_rows": int(
            (scaffold_mask & (pd.to_numeric(work["scaffold_weight_scale"], errors="coerce").fillna(1.0) < 0.999999)).sum()
        ),
        "scaffold_effective_weight": float(
            pd.to_numeric(work.loc[scaffold_mask, "sample_weight"], errors="coerce").fillna(0.0).sum()
        ),
        "real_effective_weight": float(
            pd.to_numeric(work.loc[~scaffold_mask, "sample_weight"], errors="coerce").fillna(0.0).sum()
        ),
    }
    return work, summary


def apply_runtime_context_columns(df: pd.DataFrame, runtime_code_col: str = "context_family_code_runtime", confidence_col: str = "context_family_confidence") -> pd.DataFrame:
    df = df.copy()
    runtime_codes = pd.to_numeric(df.get(runtime_code_col, pd.Series(-1, index=df.index)), errors="coerce").fillna(-1).astype(int)
    df[runtime_code_col] = runtime_codes
    for fam in DEVICE_FAMILY_CLASSES:
        fam_idx = DEVICE_FAMILY_CLASSES.index(fam)
        df[f"ctx_family_{fam}"] = (runtime_codes == fam_idx).astype(int)
    conf = pd.to_numeric(df.get(confidence_col, pd.Series(0.0, index=df.index)), errors="coerce").fillna(0.0).clip(0.0, 1.0)
    df[confidence_col] = conf
    return df


def text_series(df: pd.DataFrame, *cols, default: str = "") -> pd.Series:
    for col in cols:
        if col in df.columns:
            return df[col].astype(str).fillna(default)
    return pd.Series(default, index=df.index, dtype=object)

def _series_from(values) -> pd.Series:
    if isinstance(values, pd.Series):
        return pd.to_numeric(values, errors="coerce")
    return pd.Series(pd.to_numeric(values, errors="coerce"), dtype=float)


def _ratio_to_db20(values, floor: float = DB_RATIO_FLOOR) -> pd.Series:
    s = _series_from(values)
    arr = np.clip(s.to_numpy(dtype=float), floor, None)
    return pd.Series(20.0 * np.log10(arr), index=s.index, dtype=float)


def _ratio_to_db10(values, floor: float = DB_POWER_RATIO_FLOOR) -> pd.Series:
    s = _series_from(values)
    arr = np.clip(s.to_numpy(dtype=float), floor, None)
    return pd.Series(10.0 * np.log10(arr), index=s.index, dtype=float)


def _thd_percent_to_db(values) -> pd.Series:
    s = _series_from(values).clip(lower=0.0)
    return _ratio_to_db20(s / 100.0)


def _db20_to_ratio(values) -> pd.Series:
    s = _series_from(values)
    arr = np.power(10.0, s.to_numpy(dtype=float) / 20.0)
    return pd.Series(arr, index=s.index, dtype=float)


def _db_to_thd_percent(values) -> pd.Series:
    return 100.0 * _db20_to_ratio(values)


def detect_feature_space_version(df: pd.DataFrame) -> int:
    for col in ("feature_space_version", "arc_feature_space_version"):
        if col in df.columns:
            vals = pd.to_numeric(df[col], errors="coerce")
            if (vals >= DB_FEATURE_SPACE_VERSION).any():
                return DB_FEATURE_SPACE_VERSION
            if (vals == 3).any():
                return 3
            if ((vals > 0) & (vals < 3)).any():
                return 2

    for alias in DB_ALIAS_MAP:
        if alias in df.columns:
            return 3

    db_votes = 0
    linear_votes = 0

    for col in ("edge_spike_ratio", "midband_residual_ratio", "thd_i"):
        if col not in df.columns:
            continue
        s = pd.to_numeric(df[col], errors="coerce").dropna()
        if s.empty:
            continue
        if (s < -1e-6).any():
            db_votes += 2
        else:
            linear_votes += 1
        if col == "thd_i":
            q95 = float(s.quantile(0.95))
            if q95 > 40.0:
                linear_votes += 2
            elif float(s.quantile(0.50)) < 0.0:
                db_votes += 2

    if "hf_energy_delta" in df.columns:
        s = pd.to_numeric(df["hf_energy_delta"], errors="coerce").dropna()
        if not s.empty:
            if float(s.abs().quantile(0.95)) > 3.0:
                db_votes += 1
            elif float(s.min()) >= -1.05 and float(s.max()) <= 1.05:
                linear_votes += 1

    return 3 if db_votes > linear_votes else 2


def coerce_log_feature_space(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()
    for alias, canonical in DB_ALIAS_MAP.items():
        if alias in df.columns and canonical not in df.columns:
            df[canonical] = pd.to_numeric(df[alias], errors="coerce")

    src_version = detect_feature_space_version(df)

    if src_version < 3:
        if "midband_residual_ratio" in df.columns:
            df["midband_residual_ratio"] = _ratio_to_db20(df["midband_residual_ratio"])
        if "residual_crest_factor" in df.columns:
            df["residual_crest_factor"] = _ratio_to_db20(df["residual_crest_factor"])
        if "edge_spike_ratio" in df.columns:
            df["edge_spike_ratio"] = _ratio_to_db20(df["edge_spike_ratio"])

        if {"hf_band_energy_ratio", "rolling_baseline_hf_band_energy_ratio"}.issubset(df.columns):
            cur = pd.to_numeric(df["hf_band_energy_ratio"], errors="coerce")
            base = pd.to_numeric(df["rolling_baseline_hf_band_energy_ratio"], errors="coerce")
            df["hf_energy_delta"] = _ratio_to_db10((cur + DB_POWER_RATIO_FLOOR) / (base + DB_POWER_RATIO_FLOOR))
        elif "hf_energy_delta" in df.columns:
            legacy_delta = pd.to_numeric(df["hf_energy_delta"], errors="coerce")
            df["hf_energy_delta"] = _ratio_to_db10((1.0 + legacy_delta).clip(lower=DB_POWER_RATIO_FLOOR))

    if src_version == 3 and "thd_i" in df.columns:
        df["thd_i"] = _db_to_thd_percent(df["thd_i"])

    for col in ("spectral_flux_midhf", "cycle_nmse", "peak_fluct_cv"):
        if col in df.columns:
            s = pd.to_numeric(df[col], errors="coerce")
            df[col] = (s * 100.0) if src_version < 4 else s

    df["feature_space_version"] = DB_FEATURE_SPACE_VERSION
    return df


def _db_threshold_score(series: pd.Series, neutral_db: float, arc_db: float, hi: float = 1.5) -> pd.Series:
    s = _series_from(series)
    span = max(float(arc_db) - float(neutral_db), 1e-6)
    return ((s - float(neutral_db)) / span).clip(0.0, hi)


def _db_negative_score(series: pd.Series, neutral_db: float = 0.0, floor_db: float = -6.0, hi: float = 1.5) -> pd.Series:
    s = _series_from(series)
    span = max(float(neutral_db) - float(floor_db), 1e-6)
    return ((float(neutral_db) - s) / span).clip(0.0, hi)


def resolve_csv_files(csv_glob: str, output_path: str):
    files = []
    out_abs = os.path.abspath(output_path)
    for path in sorted(glob.glob(csv_glob, recursive=True)):
        abs_path = os.path.abspath(path)
        if abs_path == out_abs:
            continue
        if os.path.isfile(abs_path) and abs_path.lower().endswith(".csv"):
            files.append(abs_path)
    if not files:
        raise ValueError(f"No CSV files found for pattern: {csv_glob}")
    return files


def build_unknown_context_augmented_rows(df: pd.DataFrame, target_col: str = TARGET) -> pd.DataFrame:
    work = df.copy()
    if work.empty:
        return work
    work["context_family_code_runtime"] = -1
    work["context_family_confidence"] = 0.0
    work = apply_runtime_context_columns(work)
    work["context_runtime_source"] = "augmented_unknown"
    work["context_augmented"] = 1
    if "sample_weight" not in work.columns:
        work["sample_weight"] = 1.0
    work["sample_weight"] = pd.to_numeric(work["sample_weight"], errors="coerce").fillna(1.0)
    scaffold_mask = _scaffold_mask(work)
    labels = pd.to_numeric(_series_from_default(work.index, work.get(target_col, None), default=0), errors="coerce").fillna(0).astype(int)
    neg_mask = labels == 0
    pos_mask = labels == 1
    work.loc[neg_mask & ~scaffold_mask, "sample_weight"] = np.maximum(work.loc[neg_mask & ~scaffold_mask, "sample_weight"] * 2.5, 3.0)
    work.loc[pos_mask & ~scaffold_mask, "sample_weight"] = np.maximum(work.loc[pos_mask & ~scaffold_mask, "sample_weight"] * 0.35, 0.15)
    work.loc[neg_mask & scaffold_mask, "sample_weight"] = np.maximum(work.loc[neg_mask & scaffold_mask, "sample_weight"] * 0.85, SCAFFOLD_MIN_ROW_WEIGHT)
    work.loc[pos_mask & scaffold_mask, "sample_weight"] = np.maximum(work.loc[pos_mask & scaffold_mask, "sample_weight"] * 0.20, SCAFFOLD_MIN_ROW_WEIGHT)
    work.loc[scaffold_mask, "context_runtime_source"] = "augmented_unknown_scaffold"
    return work


def normalize_feature_names(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()
    df.columns = [str(c).strip() for c in df.columns]

    alias_map = {
        "spectral_flux": "spectral_flux_midhf",
        "midhf_flux": "spectral_flux_midhf",
        "resid_crest_factor": "residual_crest_factor",
        "pre_dip_spike_ratio": "edge_spike_ratio",
        **DB_ALIAS_MAP,
    }
    rename_map = {c: alias_map[c] for c in df.columns if c in alias_map}
    if rename_map:
        df = df.rename(columns=rename_map)

    if "midband_residual_ratio" not in df.columns:
        if "midband_residual_rms" in df.columns and "rolling_baseline_irms" in df.columns:
            base = pd.to_numeric(df["rolling_baseline_irms"], errors="coerce").replace(0, np.nan)
            df["midband_residual_ratio"] = pd.to_numeric(df["midband_residual_rms"], errors="coerce") / base
        elif "midband_residual_rms" in df.columns and "i_rms" in df.columns:
            base = pd.to_numeric(df["i_rms"], errors="coerce").replace(0, np.nan)
            df["midband_residual_ratio"] = pd.to_numeric(df["midband_residual_rms"], errors="coerce") / base
        elif "midband_residual_rms" in df.columns:
            df["midband_residual_ratio"] = pd.to_numeric(df["midband_residual_rms"], errors="coerce")

    if "hf_energy_delta" not in df.columns:
        if "hf_band_energy_ratio" in df.columns and "rolling_baseline_hf_band_energy_ratio" in df.columns:
            cur = pd.to_numeric(df["hf_band_energy_ratio"], errors="coerce")
            base = pd.to_numeric(df["rolling_baseline_hf_band_energy_ratio"], errors="coerce")
            df["hf_energy_delta"] = _ratio_to_db10((cur + DB_POWER_RATIO_FLOOR) / (base + DB_POWER_RATIO_FLOOR))
        elif "hf_band_energy_ratio" in df.columns:
            df["hf_energy_delta"] = _ratio_to_db10(pd.to_numeric(df["hf_band_energy_ratio"], errors="coerce"))

    if "abs_irms_zscore_vs_baseline" not in df.columns:
        if {"i_rms", "rolling_baseline_irms_mean", "rolling_baseline_irms_std"}.issubset(df.columns):
            mu = pd.to_numeric(df["rolling_baseline_irms_mean"], errors="coerce")
            sd = pd.to_numeric(df["rolling_baseline_irms_std"], errors="coerce").replace(0, np.nan)
            df["abs_irms_zscore_vs_baseline"] = (pd.to_numeric(df["i_rms"], errors="coerce") - mu).abs() / sd
        elif "irms_drop_vs_baseline" in df.columns:
            df["abs_irms_zscore_vs_baseline"] = pd.to_numeric(df["irms_drop_vs_baseline"], errors="coerce").abs()

    df = coerce_log_feature_space(df)

    for missing in FEATURES:
        if missing not in df.columns:
            df[missing] = 0.0
    if "thd_i" not in df.columns:
        df["thd_i"] = DB_THD_CLIP[0]
    return df


def _normalized_name(path: str) -> str:
    stem = os.path.splitext(os.path.basename(path))[0].lower()
    return re.sub(r"[^a-z0-9]+", "_", stem)



def _has_any_name_token(path: str, tokens) -> bool:
    name = _normalized_name(path)
    return any(tok in name for tok in tokens)



def parse_filename_tokens(path: str) -> dict:
    stem = Path(path).stem
    structured = [p for p in stem.split("__") if p]
    if len(structured) >= 4 and re.fullmatch(r"trial_?\d+", structured[2], flags=re.I):
        trial = max(1, int(re.sub(r"[^0-9]+", "", structured[2]) or "1"))
        division = normalize_division_tag(structured[3], "steady")
        device_family = normalize_device_family_token(structured[0], "unknown")
        device_name = normalize_device_name_token(structured[1], structured[1])
        trial_key = f"{device_family}__{device_name}__trial{trial:03d}"
        return {
            "division_tag": division,
            "trial_number": int(trial),
            "parsed_load_type": device_name,
            "parsed_load_family": device_family,
            "device_family": device_family,
            "device_name": device_name,
            "trial_key": trial_key,
            "section_key": division or "unknown",
            "segment_index": 0,
        }

    parts = [p for p in re.split(r"[_\s-]+", stem) if p]
    norm_parts = [str(p).lower() for p in parts]

    def _parse_trial(token: str):
        raw = str(token or "").strip().lower()
        if not raw:
            return None
        if re.fullmatch(r"\d+", raw):
            return max(1, int(raw))
        match = re.fullmatch(r"(?:t|trial)(\d{1,4})", raw)
        if match:
            return max(1, int(match.group(1)))
        return None

    division = ""
    trial = 1
    segment_index = 0
    division_idx = None
    trial_idx = None

    for idx in range(len(norm_parts) - 1, -1, -1):
        maybe_div = FILENAME_DIVISION_ALIASES.get(norm_parts[idx], "")
        if maybe_div:
            division = maybe_div
            division_idx = idx
            break

    for idx in range(len(norm_parts) - 1, -1, -1):
        if idx == division_idx:
            continue
        maybe_trial = _parse_trial(norm_parts[idx])
        if maybe_trial:
            trial = maybe_trial
            trial_idx = idx
            break

    keep_tokens = [token for idx, token in enumerate(parts) if idx not in {division_idx, trial_idx}]

    if keep_tokens:
        seg_match = re.fullmatch(r"(?:seg|part|slice)(\d{1,4})", str(keep_tokens[-1]).lower())
        if seg_match:
            segment_index = max(1, int(seg_match.group(1)))
            keep_tokens.pop()

    while len(keep_tokens) > 1 and (str(keep_tokens[0]).lower() in FILENAME_PREFIX_TOKENS or re.fullmatch(r"\d{6,}", str(keep_tokens[0]))):
        keep_tokens.pop(0)
    while keep_tokens and str(keep_tokens[-1]).lower() in FILENAME_SUFFIX_TOKENS:
        keep_tokens.pop()

    device_name = normalize_device_name_token("_".join(keep_tokens), "unknown_device")
    device_family = normalize_device_family_token("unknown")
    trial_key = f"{device_family}__{device_name}__trial{int(trial):03d}"
    section_key = division or "unknown"
    return {
        "division_tag": division,
        "trial_number": int(trial),
        "parsed_load_type": device_name,
        "parsed_load_family": device_family,
        "device_family": device_family,
        "device_name": device_name,
        "trial_key": trial_key,
        "section_key": section_key,
        "segment_index": int(segment_index),
    }

def infer_source_kind(path: str):
    name = os.path.basename(path).lower()
    tokens = parse_filename_tokens(path)
    division = tokens.get("division_tag", "")
    if "scaffold" in name:
        return "scaffold", 1, 0
    if division == "steady":
        return "real", 4, 1
    if division == "start":
        return "real", 3, 0
    if _has_any_name_token(path, TRUSTED_NORMAL_NAME_TOKENS):
        return "real", 4, 1
    return "real", 3, 0



def load_and_tag_csvs(csv_files, *, raw_root: Path | None = None, current_training_generation: str | None = None):
    frames = []
    raw_root = (raw_root or (PROJECT_ROOT / "tinyml" / "data" / "raw")).resolve()

    for i, path in enumerate(csv_files):
        print(f"[{i + 1}/{len(csv_files)}] Loading: {path}")
        df = normalize_feature_names(pd.read_csv(path).copy())

        path_obj = Path(path).resolve()
        try:
            rel_path = path_obj.relative_to(raw_root)
        except ValueError:
            rel_path = Path(path_obj.name)

        rel_no_ext = rel_path.with_suffix("")
        section_prefix = str(rel_no_ext).replace(chr(92), "__").replace("/", "__")

        file_tokens = parse_filename_tokens(path)
        source_kind, source_priority, trusted_normal_session = infer_source_kind(path)
        source_folder = str(rel_path.parent).replace(chr(92), "/") if str(rel_path.parent) != "." else ""
        trial_stem = f"{file_tokens.get('trial_key', 'unknown__trial001')}"
        trial_prefix = "__".join([p for p in [source_folder.replace('/', '__'), trial_stem] if p]) or trial_stem

        group_col = pick_group_column(df)
        if group_col is None:
            df["session_id"] = f"section__{section_prefix}"
        else:
            df[group_col] = section_prefix + "__" + df[group_col].astype(str)
        df["section_id"] = section_prefix

        device_family_series = text_series(df, "device_family", "family", default=file_tokens.get("device_family", "unknown")).map(normalize_device_family_token)
        device_name_series = text_series(df, "device_name", "load_type", default=file_tokens.get("device_name", file_tokens.get("parsed_load_type", "unknown_device"))).map(lambda v: normalize_device_name_token(v, file_tokens.get("device_name", "unknown_device")))
        division_series = text_series(df, "division_tag", default=file_tokens.get("division_tag", "steady")).map(normalize_division_tag)
        trial_series = pd.to_numeric(df["trial_number"], errors="coerce").fillna(file_tokens.get("trial_number", 1)).astype(int) if "trial_number" in df.columns else pd.Series(int(file_tokens.get("trial_number", 1)), index=df.index, dtype=int)
        notes_series = text_series(df, "notes", default="").map(lambda v: str(v).strip())
        trusted_series = pd.to_numeric(df.get("trusted_normal_session", pd.Series(int(trusted_normal_session), index=df.index)), errors="coerce").fillna(int(trusted_normal_session)).astype(int)

        division_group_suffix = division_series.map(
            lambda v: "start_ctx" if str(v) in {"start", "startup"} else ("steady_ref" if str(v) == "steady" else ("arc_capture" if str(v) == "arc" else str(v)))
        )
        df["trial_id"] = trial_prefix + "__" + division_group_suffix.astype(str)

        df["_source_file"] = str(rel_path).replace(chr(92), "/")
        df["_source_folder"] = source_folder
        df["source_kind"] = source_kind
        df["source_priority"] = source_priority
        df["trusted_normal_session"] = trusted_series
        df["division_tag"] = division_series
        df["trial_number"] = trial_series
        df["device_family"] = device_family_series
        df["device_name"] = device_name_series
        df["parsed_load_type"] = device_name_series
        df["parsed_load_family"] = device_family_series
        df["notes"] = notes_series
        df["load_type"] = device_name_series
        df["section_key"] = file_tokens.get("section_key", "unknown")
        if "rf_train_row" not in df.columns:
            df["rf_train_row"] = 1
        df = annotate_generation_and_mismatch_columns(
            df,
            current_training_generation=current_training_generation,
        )
        frames.append(df)

    return pd.concat(frames, ignore_index=True)



def coerce_numeric_if_present(df: pd.DataFrame, cols):
    for col in cols:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")
    return df



def _num(df: pd.DataFrame, col: str, default: float = 0.0) -> pd.Series:
    if col not in df.columns:
        return pd.Series(default, index=df.index, dtype=float)
    return pd.to_numeric(df[col], errors="coerce").fillna(default)


def _clip_score(series: pd.Series, ref: float, hi: float = 1.5) -> pd.Series:
    ref = max(float(ref), 1e-6)
    return (_num(pd.DataFrame({"x": series}), "x") / ref).clip(0.0, hi)


def build_quality_scores(df: pd.DataFrame) -> tuple[pd.Series, pd.Series]:
    flux = _clip_score(_num(df, "spectral_flux_midhf"), 0.085)
    crest = _db_threshold_score(
        _num(df, "residual_crest_factor"),
        DB_NORMAL_ANCHORS["residual_crest_factor"],
        DB_ARC_THRESHOLDS["residual_crest_factor"],
    )
    edge = _db_threshold_score(
        _num(df, "edge_spike_ratio"),
        DB_NORMAL_ANCHORS["edge_spike_ratio"],
        DB_ARC_THRESHOLDS["edge_spike_ratio"],
    )
    mid = _db_threshold_score(
        _num(df, "midband_residual_ratio"),
        DB_NORMAL_ANCHORS["midband_residual_ratio"],
        DB_ARC_THRESHOLDS["midband_residual_ratio"],
    )
    nmse = _clip_score(_num(df, "cycle_nmse"), 0.090)
    peak = _clip_score(_num(df, "peak_fluct_cv"), 0.012)
    thd = _db_threshold_score(
        _num(df, "thd_i"),
        DB_NORMAL_ANCHORS["thd_i"],
        DB_ARC_THRESHOLDS["thd_i"],
    )
    hf_delta_pos = _db_threshold_score(
        _num(df, "hf_energy_delta"),
        DB_NORMAL_ANCHORS["hf_energy_delta"],
        DB_ARC_THRESHOLDS["hf_energy_delta"],
    )
    hf_delta_neg = _db_negative_score(_num(df, "hf_energy_delta"), neutral_db=0.0, floor_db=-6.0)
    zcv = _clip_score(_num(df, "zcv"), 0.20)
    iz = _clip_score(_num(df, "abs_irms_zscore_vs_baseline"), 2.35)
    pulse = _clip_score(_num(df, "pulse_count_per_cycle"), 0.35)
    zero_dwell = _clip_score(_num(df, "zero_dwell_ratio"), 18.0)
    low_ratio = _clip_score(_num(df, "low_current_ratio"), 10.0)
    low_run = _clip_score(_num(df, "max_low_current_run_ms"), 1.10)

    arc_base = (
        0.16 * flux
        + 0.11 * crest
        + 0.14 * edge
        + 0.11 * mid
        + 0.08 * nmse
        + 0.05 * peak
        + 0.04 * thd
        + 0.05 * hf_delta_pos
        + 0.03 * zcv
        + 0.04 * iz
        + 0.10 * pulse
        + 0.06 * zero_dwell
        + 0.06 * low_ratio
        + 0.07 * low_run
    )

    if "session_id" in df.columns:
        order_cols = preferred_time_sort_cols(df, session_first=True)
        tmp = df[order_cols].copy()
        tmp["_arc_base"] = arc_base.to_numpy(dtype=float)
        tmp = tmp.sort_values(order_cols).copy()
        tmp["_arc_cluster"] = tmp.groupby("session_id", sort=False)["_arc_base"].transform(
            lambda s: s.rolling(window=5, min_periods=1, center=True).max()
        )
        arc_cluster = tmp.sort_index()["_arc_cluster"]
    else:
        arc_cluster = arc_base

    feat_valid = (_num(df, "feat_valid", 1.0) >= 1.0).astype(float)
    cur_valid = (_num(df, "current_valid", 1.0) >= 1.0).astype(float)
    v_rms = _num(df, "v_rms")
    i_rms = _num(df, "i_rms")
    v_normal = ((v_rms >= 205.0) & (v_rms <= 255.0)).astype(float)
    i_active = ((i_rms >= 0.08) & (i_rms <= 20.0)).astype(float)
    trusted = (_num(df, "trusted_normal_session") >= 1.0).astype(float)

    arc_like = (0.76 * arc_base) + (0.24 * arc_cluster.clip(0.0, 1.5))
    arc_like = (arc_like * (0.85 + 0.15 * feat_valid) * (0.90 + 0.10 * cur_valid)).clip(0.0, 1.5)

    normal_like = (
        (1.0 - arc_like.clip(0.0, 1.0)) * 0.62
        + 0.10 * feat_valid
        + 0.09 * cur_valid
        + 0.10 * v_normal
        + 0.07 * i_active
        + 0.10 * hf_delta_neg
        + 0.08 * (1.0 - flux.clip(0.0, 1.0))
        + 0.07 * (1.0 - edge.clip(0.0, 1.0))
        + 0.12 * trusted
    ).clip(0.0, 1.5)

    return arc_like.astype(float), normal_like.astype(float)


def _choose_reason(existing: pd.Series, mask: pd.Series, reason: str) -> pd.Series:
    out = existing.copy()
    out.loc[mask] = reason
    return out



def _build_label_trust(df: pd.DataFrame) -> pd.Series:
    base = np.where(
        df[TARGET].astype(int) == 0,
        DEFAULT_NEG_LABEL_TRUST,
        DEFAULT_POS_LABEL_TRUST,
    ).astype(float)

    trusted_normal = pd.to_numeric(
        df.get("trusted_normal_session", 0),
        errors="coerce",
    ).fillna(0).astype(int)
    base = np.where(
        (trusted_normal == 1) & (df[TARGET].astype(int) == 0),
        TRUSTED_NORMAL_LABEL_TRUST,
        base,
    )

    for trust_col in ("label_trust", "label_confidence", "label_quality"):
        if trust_col in df.columns:
            explicit = pd.to_numeric(df[trust_col], errors="coerce")
            mask = explicit.notna()
            base = np.where(mask, explicit.to_numpy(dtype=float), base)
            break

    base = np.clip(base, 0.05, 2.00)
    return pd.Series(base, index=df.index, dtype=float)

    df = df.reset_index(drop=True)

def attach_training_metadata(df: pd.DataFrame) -> tuple[pd.DataFrame, dict]:
    df = df.copy()

    if "i_rms" not in df.columns:
        df["i_rms"] = np.nan
    if "v_rms" not in df.columns:
        df["v_rms"] = np.nan

    if "trusted_normal_session" not in df.columns:
        df["trusted_normal_session"] = 0
    df["trusted_normal_session"] = pd.to_numeric(
        df["trusted_normal_session"],
        errors="coerce",
    ).fillna(0).astype(int)

    if "rf_train_row" not in df.columns:
        df["rf_train_row"] = 1
    df["rf_train_row"] = pd.to_numeric(df["rf_train_row"], errors="coerce").fillna(1).astype(int)

    division_tag = df.get("division_tag", pd.Series("", index=df.index, dtype=object)).astype(str).str.lower()

    # Explicit trusted-normal files remain strong negatives.
    df.loc[df["trusted_normal_session"] == 1, TARGET] = 0

    # Section tags are priors only. Keep explicit row labels intact and let trust/weight
    # capture our confidence rather than rewriting labels from the filename.
    df["section_prior"] = "neutral"
    df.loc[division_tag == "arc", "section_prior"] = "arc_eligible"
    df.loc[division_tag == "steady", "section_prior"] = "steady_hint"
    df.loc[division_tag == "start", "section_prior"] = "transient_hint"

    df["label_trust"] = _build_label_trust(df)
    steady_neg_hint = (division_tag == "steady") & (df[TARGET].astype(int) == 0) & (df["trusted_normal_session"] == 0)
    transient_neg_hint = (division_tag == "start") & (df[TARGET].astype(int) == 0) & (df["trusted_normal_session"] == 0)
    df.loc[steady_neg_hint, "label_trust"] = np.minimum(df.loc[steady_neg_hint, "label_trust"], 0.82)
    df.loc[transient_neg_hint, "label_trust"] = np.minimum(df.loc[transient_neg_hint, "label_trust"], 0.68)

    df["arc_like_score"], df["normal_like_score"] = build_quality_scores(df)
    df["clean_reason"] = "kept"
    df["clean_quality"] = "trainable"

    invalid_off_flag = (_num(df, "invalid_off_flag") > 0.5) if "invalid_off_flag" in df.columns else pd.Series(False, index=df.index)
    invalid_loaded_flag = (_num(df, "invalid_loaded_flag") > 0.5) if "invalid_loaded_flag" in df.columns else pd.Series(False, index=df.index)
    if "feat_valid" in df.columns:
        invalid_loaded_flag |= (_num(df, "feat_valid") < 1.0) & (_num(df, "v_rms") >= 170.0) & (_num(df, "i_rms") >= 0.20)
        invalid_off_flag |= (_num(df, "feat_valid") < 1.0) & (_num(df, "v_rms") < 170.0)
    if "current_valid" in df.columns:
        invalid_off_flag |= (_num(df, "current_valid") < 1.0) & (_num(df, "v_rms") < 170.0)
    if "adc_fs_hz" in df.columns:
        invalid_off_flag |= (_num(df, "adc_fs_hz") <= 0.0)

    transport_glitch_flag = pd.Series(False, index=df.index)
    if "sampling_quality_bad" in df.columns:
        transport_glitch_flag |= (_num(df, "sampling_quality_bad") > 0.5)
    if "queue_drop_count" in df.columns:
        transport_glitch_flag |= (_num(df, "queue_drop_count") > 0.5)
    if "timing_skew_ms" in df.columns:
        transport_glitch_flag |= (_num(df, "timing_skew_ms").abs() > 250.0)

    df["invalid_off_flag"] = invalid_off_flag.astype(int)
    df["invalid_loaded_flag"] = invalid_loaded_flag.astype(int)
    df["transport_glitch_flag"] = transport_glitch_flag.astype(int)

    df.loc[invalid_off_flag, "rf_train_row"] = 0
    df.loc[invalid_off_flag, "clean_reason"] = "invalid_off"
    df.loc[invalid_off_flag, "clean_quality"] = "excluded_invalid_off"

    df.loc[invalid_loaded_flag, "rf_train_row"] = 0
    df.loc[invalid_loaded_flag & ~invalid_off_flag, "clean_reason"] = "invalid_loaded_context"
    df.loc[invalid_loaded_flag & ~invalid_off_flag, "clean_quality"] = "context_invalid_loaded"

    df.loc[transport_glitch_flag, "rf_train_row"] = 0
    df.loc[transport_glitch_flag & (df["clean_reason"] == "kept"), "clean_reason"] = "transport_or_timing_glitch"
    df.loc[transport_glitch_flag & (df["clean_quality"] == "trainable"), "clean_quality"] = "excluded_transport_glitch"

    positive_mask = (df[TARGET].astype(int) == 1) & (df["rf_train_row"] == 1)
    very_weak_positive = positive_mask & (df["arc_like_score"] < VERY_WEAK_POSITIVE_SCORE)
    soft_positive = positive_mask & ~very_weak_positive & (df["arc_like_score"] < SOFT_POSITIVE_SCORE)

    df.loc[very_weak_positive, "rf_train_row"] = 0
    df.loc[very_weak_positive, "clean_reason"] = "very_weak_positive_label"
    df.loc[very_weak_positive, "clean_quality"] = "excluded_weak_positive"

    df.loc[soft_positive, "label_trust"] = np.minimum(df.loc[soft_positive, "label_trust"], 0.30)
    df.loc[soft_positive, "clean_reason"] = "soft_positive_low_trust"
    df.loc[soft_positive, "clean_quality"] = "soft_positive"

    flat_extinguish_positive = positive_mask & (
        _num(df, "spectral_flux_midhf") < 0.040
    ) & (
        _num(df, "edge_spike_ratio") < -20.915
    ) & (
        _num(df, "peak_fluct_cv") < 0.010
    ) & (
        _num(df, "hf_energy_delta") < 0.80
    ) & (
        _num(df, "abs_irms_zscore_vs_baseline") >= 1.20
    )

    df.loc[flat_extinguish_positive, "rf_train_row"] = 0
    df.loc[flat_extinguish_positive, "clean_reason"] = "flat_extinguish_positive"
    df.loc[flat_extinguish_positive, "clean_quality"] = "excluded_extinguish_plateau"

    negative_mask = (df[TARGET].astype(int) == 0) & (df["rf_train_row"] == 1) & (df["trusted_normal_session"] == 0)
    strong_arc_like_negative = negative_mask & (df["arc_like_score"] >= STRONG_NEGATIVE_ARC_SCORE)
    soft_arc_like_negative = negative_mask & ~strong_arc_like_negative & (df["arc_like_score"] >= SOFT_NEGATIVE_ARC_SCORE)

    df["hard_negative"] = 0
    df.loc[strong_arc_like_negative, "hard_negative"] = 1
    df.loc[strong_arc_like_negative, "label_trust"] = np.maximum(df.loc[strong_arc_like_negative, "label_trust"], 0.80)
    df.loc[strong_arc_like_negative, "clean_reason"] = "strong_arc_like_negative"
    df.loc[strong_arc_like_negative, "clean_quality"] = "hard_negative"

    df.loc[soft_arc_like_negative, "label_trust"] = np.minimum(df.loc[soft_arc_like_negative, "label_trust"], 0.75)
    df.loc[soft_arc_like_negative, "clean_reason"] = "soft_arc_like_negative"
    df.loc[soft_arc_like_negative, "clean_quality"] = "soft_negative"

    base_weight = np.where(
        df["source_kind"] == "real",
        np.where(df[TARGET] == 1, REAL_POS_WEIGHT, REAL_NEG_WEIGHT),
        np.where(df[TARGET] == 1, SCAFFOLD_POS_WEIGHT, SCAFFOLD_NEG_WEIGHT),
    ).astype(float)

    df["sample_weight"] = (base_weight * df["label_trust"]).astype(float)

    trusted_normal_mask = (
        (df["trusted_normal_session"] == 1)
        & (df[TARGET] == 0)
        & (pd.to_numeric(df["rf_train_row"], errors="coerce").fillna(0) == 1)
    )
    df.loc[trusted_normal_mask, "sample_weight"] = np.maximum(
        df.loc[trusted_normal_mask, "sample_weight"],
        TRUSTED_NORMAL_WEIGHT,
    )

    if "session_id" in df.columns and "i_rms" in df.columns:
        sort_cols = preferred_time_sort_cols(df, session_first=True)
        df = df.sort_values(sort_cols).copy()
        di = df.groupby("session_id")["i_rms"].diff().abs().fillna(0.0)
        turn_like = (di >= 0.60) | (
            (_num(df, "i_rms") >= 0.10) & (_num(df, "i_rms") <= 0.50)
        )
        transient_norm = (
            (df[TARGET] == 0)
            & (df["rf_train_row"] == 1)
            & (turn_like | (division_tag == "start"))
            & (df["trusted_normal_session"] == 0)
            & (df["arc_like_score"] < SOFT_NEGATIVE_ARC_SCORE)
        )
        df.loc[transient_norm, "sample_weight"] = np.maximum(
            df.loc[transient_norm, "sample_weight"],
            TRANSIENT_NORMAL_WEIGHT * df.loc[transient_norm, "label_trust"],
        )
        df["transition_normal"] = transient_norm.astype(int)
        df = df.sort_index().copy()
    else:
        df["transition_normal"] = 0

    if "trial_id" in df.columns:
        df["conflict_scope"] = df["trial_id"].astype(str)
    elif "session_id" in df.columns:
        df["conflict_scope"] = df["session_id"].astype(str)
    else:
        df["conflict_scope"] = df.get("parsed_load_type", pd.Series("unknown", index=df.index, dtype=object)).astype(str)

    sig_cols = [c for c in FEATURES if c in df.columns]
    if df.empty or not sig_cols:
        df["_feature_sig"] = pd.Series("", index=df.index, dtype=object)
        df["_conflict_sig"] = pd.Series("", index=df.index, dtype=object)
    else:
        rounded_exact = df[sig_cols].round(4).astype(str)
        rounded_coarse = df[sig_cols].round(CONFLICT_ROUND_DECIMALS).astype(str)
        df["_feature_sig"] = rounded_exact.agg("|".join, axis=1)
        df["_conflict_sig"] = rounded_coarse.agg("|".join, axis=1)

    df, scaffold_summary = apply_scaffold_gap_fill_weights(df)

    quality_summary = {
        "invalid_off_rows": int(invalid_off_flag.sum()),
        "invalid_loaded_rows": int(invalid_loaded_flag.sum()),
        "very_weak_positive_rows": int(very_weak_positive.sum()),
        "soft_positive_rows": int(soft_positive.sum()),
        "flat_extinguish_positive_rows": int(flat_extinguish_positive.sum()),
        "strong_arc_like_negative_rows": int(strong_arc_like_negative.sum()),
        "soft_arc_like_negative_rows": int(soft_arc_like_negative.sum()),
        **{str(k): (float(v) if "weight" in str(k) else int(v)) for k, v in scaffold_summary.items()},
    }
    return df, quality_summary


def _choose_representative_row(group: pd.DataFrame) -> pd.Series:
    ranked = group.sort_values(
        ["rf_train_row", "label_trust", "source_priority", "sample_weight"],
        ascending=[False, False, False, False],
    )
    return ranked.iloc[0].copy()



def resolve_near_duplicate_label_conflicts(df: pd.DataFrame) -> tuple[pd.DataFrame, dict]:
    if "_conflict_sig" not in df.columns:
        return df, {
            "groups_seen": 0,
            "mixed_groups": 0,
            "prefer_trusted_normal": 0,
            "prefer_more_trusted_normal": 0,
            "keep_more_trusted_arc": 0,
            "drop_ambiguous": 0,
        }

    stats = {
        "groups_seen": 0,
        "mixed_groups": 0,
        "prefer_trusted_normal": 0,
        "prefer_more_trusted_normal": 0,
        "keep_more_trusted_arc": 0,
        "drop_ambiguous": 0,
    }

    kept_rows = []
    for _, group in df.groupby(["conflict_scope", "_conflict_sig"], sort=False, dropna=False):
        stats["groups_seen"] += 1
        g = group.copy()
        g[TARGET] = pd.to_numeric(g[TARGET], errors="coerce").fillna(0).astype(int)
        g["rf_train_row"] = pd.to_numeric(g["rf_train_row"], errors="coerce").fillna(0).astype(int)
        g["label_trust"] = pd.to_numeric(g["label_trust"], errors="coerce").fillna(0.0)
        g["source_priority"] = pd.to_numeric(g["source_priority"], errors="coerce").fillna(0)
        g["sample_weight"] = pd.to_numeric(g["sample_weight"], errors="coerce").fillna(0.0)
        g["trusted_normal_session"] = pd.to_numeric(_series_from_default(g.index, g.get("trusted_normal_session", None), default=0), errors="coerce").fillna(0).astype(int)
        g["arc_like_score"] = pd.to_numeric(_series_from_default(g.index, g.get("arc_like_score", None), default=0.0), errors="coerce").fillna(0.0)
        g["normal_like_score"] = pd.to_numeric(_series_from_default(g.index, g.get("normal_like_score", None), default=0.0), errors="coerce").fillna(0.0)

        labels = set(g.loc[g["rf_train_row"] == 1, TARGET].astype(int).tolist())
        if len(labels) <= 1:
            chosen = _choose_representative_row(g)
            chosen["label_conflict"] = 0
            chosen["conflict_policy"] = "none"
            kept_rows.append(chosen)
            continue

        stats["mixed_groups"] += 1
        neg = g[(g[TARGET] == 0) & (g["rf_train_row"] == 1)].copy()
        pos = g[(g[TARGET] == 1) & (g["rf_train_row"] == 1)].copy()
        best_neg = _choose_representative_row(neg)
        best_pos = _choose_representative_row(pos)

        neg_trust = float(best_neg.get("label_trust", 0.0))
        pos_trust = float(best_pos.get("label_trust", 0.0))
        neg_trusted_session = int(best_neg.get("trusted_normal_session", 0)) == 1
        neg_conf = neg_trust * (1.0 + 0.85 * float(best_neg.get("normal_like_score", 0.0)) + 0.35 * float(best_neg.get("trusted_normal_session", 0)))
        pos_conf = pos_trust * (1.0 + 1.15 * float(best_pos.get("arc_like_score", 0.0)))
        pos_arc = float(best_pos.get("arc_like_score", 0.0))

        if neg_trusted_session and pos_arc < 0.60:
            chosen = best_neg.copy()
            chosen["label_conflict"] = 1
            chosen["conflict_policy"] = "prefer_trusted_normal"
            stats["prefer_trusted_normal"] += 1
        elif (neg_conf - pos_conf) >= NORMAL_PREFERENCE_MARGIN:
            chosen = best_neg.copy()
            chosen["label_conflict"] = 1
            chosen["conflict_policy"] = "prefer_more_trusted_normal"
            stats["prefer_more_trusted_normal"] += 1
        elif (pos_conf - neg_conf) >= ARC_PREFERENCE_MARGIN or pos_arc >= STRONG_NEGATIVE_ARC_SCORE:
            chosen = best_pos.copy()
            chosen["label_conflict"] = 1
            chosen["conflict_policy"] = "keep_more_trusted_arc"
            stats["keep_more_trusted_arc"] += 1
        else:
            chosen = _choose_representative_row(g).copy()
            chosen["label_conflict"] = 1
            chosen["conflict_policy"] = "drop_ambiguous"
            chosen["rf_train_row"] = 0
            chosen["clean_quality"] = "ambiguous_conflict"
            stats["drop_ambiguous"] += 1

        kept_rows.append(chosen)

    out = pd.DataFrame(kept_rows).reset_index(drop=True)
    return out, stats



def clean_dataset(df: pd.DataFrame, augment_unknown_context: bool = False) -> tuple[pd.DataFrame, dict]:
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
        "frame_start_uptime_ms",
        "frame_end_uptime_ms",
        "feature_compute_end_uptime_ms",
        "log_enqueue_uptime_ms",
        "timing_skew_ms",
        "rf_train_row",
        "trusted_normal_session",
        "label_trust",
        "sample_weight",
    ]
    df = coerce_numeric_if_present(df, maybe_numeric)
    before = len(df)

    division_series = df.get("division_tag", pd.Series("", index=df.index, dtype=object)).astype(str).str.lower()
    raw_target = pd.to_numeric(df[TARGET], errors="coerce")
    explicit_arc_truth_mask = raw_target.isin([0, 1])
    context_only_placeholder_mask = division_series.isin(["start", "startup"]) & ~explicit_arc_truth_mask
    arc_training_placeholder_mask = division_series.isin(["steady", "arc"]) & ~explicit_arc_truth_mask

    df["arc_truth_available"] = explicit_arc_truth_mask.astype(int)
    df["context_only_placeholder_arc_target"] = context_only_placeholder_mask.astype(int)
    df["arc_training_placeholder_arc_target"] = arc_training_placeholder_mask.astype(int)

    # Keep unlabeled start rows for the context dataset only. These get a benign
    # placeholder arc target of 0 but remain marked as context-only placeholders.
    if bool(context_only_placeholder_mask.any()):
        df.loc[context_only_placeholder_mask, TARGET] = 0
        if "label_truth_source" in df.columns:
            placeholder_source = df["label_truth_source"].astype(str).replace({"": "human_label"})
            placeholder_source.loc[context_only_placeholder_mask] = "context_family_only"
            df["label_truth_source"] = placeholder_source

    # Keep unlabeled steady rows and unlabeled non-event rows inside arc captures as
    # explicit negatives for arc training. This is required because field CSVs often
    # mark only the spark frames as 1 and leave all other rows as -1.
    if bool(arc_training_placeholder_mask.any()):
        df.loc[arc_training_placeholder_mask, TARGET] = 0
        if "label_truth_source" in df.columns:
            placeholder_source = df["label_truth_source"].astype(str).replace({"": "human_label"})
            placeholder_source.loc[arc_training_placeholder_mask] = "division_default_negative"
            df["label_truth_source"] = placeholder_source

    df = df[df[TARGET].isin([0, 1])].copy()
    x = df[FEATURES].replace([np.inf, -np.inf], np.nan)
    df = df.loc[x.notna().all(axis=1)].copy()

    if "adc_fs_hz" in df.columns:
        df = df[df["adc_fs_hz"] > 0].copy()

    for c, (lo, hi) in FEATURE_CLIP_BOUNDS.items():
        if c in df.columns:
            df[c] = df[c].clip(lo, hi)

    df, quality_summary = attach_training_metadata(df)
    df, conflict_stats = resolve_near_duplicate_label_conflicts(df)

    # Final exact dedupe after conflict handling. Keep dedupe local to the current
    # trial/session scope so similar arc signatures from unrelated captures are not
    # collapsed into one row.
    local_dedupe_cols = (["conflict_scope"] if "conflict_scope" in df.columns else []) + FEATURES
    df = df.sort_values(
        ["rf_train_row", "label_trust", "source_priority", "sample_weight"],
        ascending=[False, False, False, False],
    ).drop_duplicates(subset=local_dedupe_cols, keep="first").copy()

    df[TARGET] = pd.to_numeric(df[TARGET], errors="coerce").fillna(0).astype(int)
    df["device_family"] = text_series(df, "device_family", "parsed_load_family", default="unknown").map(normalize_device_family_token)
    df["device_name"] = text_series(df, "device_name", "parsed_load_type", "load_type", default="unknown_device").map(normalize_device_name_token)
    df["device_family_code"] = df["device_family"].map(device_family_code_from_token).fillna(-1).astype(int)
    if "context_family_code_runtime" in df.columns:
        runtime_codes = pd.to_numeric(df["context_family_code_runtime"], errors="coerce").fillna(-1).astype(int)
    else:
        runtime_codes = df["device_family_code"].astype(int)
    df["context_family_code_runtime"] = runtime_codes
    default_ctx_conf = pd.Series(np.where(runtime_codes >= 0, 1.0, 0.0), index=df.index, dtype=float)
    ctx_conf = df.get("context_family_confidence", default_ctx_conf)
    if not isinstance(ctx_conf, pd.Series):
        ctx_conf = pd.Series(ctx_conf, index=df.index)
    df["context_family_confidence"] = pd.to_numeric(ctx_conf, errors="coerce").fillna(0.0).clip(0.0, 1.0)
    df = apply_runtime_context_columns(df)
    df["rf_train_row"] = pd.to_numeric(
        df["rf_train_row"],
        errors="coerce",
    ).fillna(0).astype(int)
    df["label_trust"] = pd.to_numeric(df["label_trust"], errors="coerce").fillna(0.0)
    df["sample_weight"] = pd.to_numeric(df["sample_weight"], errors="coerce").fillna(1.0)
    df["trusted_normal_session"] = pd.to_numeric(
        df["trusted_normal_session"],
        errors="coerce",
    ).fillna(0).astype(int)
    df["label_conflict"] = pd.to_numeric(_series_from_default(df.index, df.get("label_conflict", None), default=0), errors="coerce").fillna(0).astype(int)
    df["hard_negative"] = pd.to_numeric(_series_from_default(df.index, df.get("hard_negative", None), default=0), errors="coerce").fillna(0).astype(int)
    df["context_runtime_source"] = "metadata_family"
    df["context_augmented"] = 0

    augmentation_rows = 0
    if augment_unknown_context:
        aug_src = build_unknown_context_augmented_rows(df[df["rf_train_row"] == 1].copy(), target_col=TARGET)
        if not aug_src.empty:
            augmentation_rows = int(len(aug_src))
            df = pd.concat([df, aug_src], ignore_index=True, sort=False)
    if "conflict_scope" not in df.columns:
        df["conflict_scope"] = "global"

    base_rows_after = int(len(df) - augmentation_rows)
    rows_removed = int(before - base_rows_after)

    print("Rows before cleaning:", before)
    print("Rows after cleaning :", base_rows_after)
    print("Rows removed        :", rows_removed)
    if augmentation_rows:
        print("Augmented rows added:", augmentation_rows)
    print("Trainable rows      :", int(df["rf_train_row"].sum()))
    print("Trusted normal rows :", int((df["trusted_normal_session"] == 1).sum()))
    print("Conflict-marked rows:", int((df["label_conflict"] == 1).sum()))
    print("Conflict resolution summary:")
    for key, value in conflict_stats.items():
        print(f"  {key}: {value}")
    print("Source counts:")
    print(df["source_kind"].value_counts().to_string())
    print("Label counts:")
    print(df[TARGET].value_counts().to_string())
    if "conflict_policy" in df.columns:
        print("Conflict policies kept:")
        print(df["conflict_policy"].value_counts().to_string())
    summary = {
        "rows_before_cleaning": int(before),
        "rows_after_cleaning": int(base_rows_after),
        "rows_removed": int(rows_removed),
        "context_only_placeholder_rows_preserved": int(pd.to_numeric(df.get("context_only_placeholder_arc_target", pd.Series(0, index=df.index)), errors="coerce").fillna(0).astype(int).sum()),
        "arc_training_placeholder_rows_preserved": int(pd.to_numeric(df.get("arc_training_placeholder_arc_target", pd.Series(0, index=df.index)), errors="coerce").fillna(0).astype(int).sum()),
        "arc_truth_available_rows": int(pd.to_numeric(df.get("arc_truth_available", pd.Series(0, index=df.index)), errors="coerce").fillna(0).astype(int).sum()),
        "augmentation_rows_added": int(augmentation_rows),
        "rows_after_cleaning_with_augmentation": int(len(df)),
        "trainable_rows": int(df["rf_train_row"].sum()),
        "trusted_normal_rows": int((df["trusted_normal_session"] == 1).sum()),
        "conflict_marked_rows": int((df["label_conflict"] == 1).sum()),
        "label_counts": {str(k): int(v) for k, v in df[TARGET].value_counts().to_dict().items()},
        "source_counts": {str(k): int(v) for k, v in df["source_kind"].value_counts().to_dict().items()},
        "conflict_policy_counts": (
            {str(k): int(v) for k, v in df["conflict_policy"].value_counts().to_dict().items()}
            if "conflict_policy" in df.columns
            else {}
        ),
        "quality_summary": {
            str(k): (float(v) if isinstance(v, (float, np.floating)) and not float(v).is_integer() else int(v) if isinstance(v, (int, np.integer, float, np.floating)) else v)
            for k, v in quality_summary.items()
        },
        "division_counts": ({str(k): int(v) for k, v in df["division_tag"].fillna("").replace("", "unknown").value_counts().to_dict().items()} if "division_tag" in df.columns else {}),
        "trial_counts": ({str(k): int(v) for k, v in df["trial_id"].astype(str).value_counts().to_dict().items()} if "trial_id" in df.columns else {}),
        "device_family_counts": ({str(k): int(v) for k, v in df["device_family"].astype(str).value_counts().to_dict().items()} if "device_family" in df.columns else {}),
        "device_family_code_counts": ({str(int(k)): int(v) for k, v in pd.to_numeric(df.get("device_family_code", pd.Series([], dtype=int)), errors="coerce").fillna(-1).astype(int).value_counts().to_dict().items()} if "device_family_code" in df.columns else {}),
        "context_runtime_code_counts": ({str(int(k)): int(v) for k, v in pd.to_numeric(df.get("context_family_code_runtime", pd.Series([], dtype=int)), errors="coerce").fillna(-1).astype(int).value_counts().to_dict().items()} if "context_family_code_runtime" in df.columns else {}),
        "unknown_division_rows": int((df.get("division_tag", pd.Series("", index=df.index, dtype=object)).astype(str).str.strip() == "").sum()),
        "section_prior_counts": ({str(k): int(v) for k, v in df.get("section_prior", pd.Series("neutral", index=df.index)).astype(str).value_counts().to_dict().items()}),
        "conflict_resolution": {str(k): int(v) for k, v in conflict_stats.items()},
    }
    return df, summary



def reorder_export_columns(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()
    feature_cols = [c for c in FEATURES if c in df.columns]
    context_cols = [c for c in df.columns if str(c).startswith("ctx_family_")]
    if "context_family_confidence" in df.columns and "context_family_confidence" not in context_cols:
        context_cols.append("context_family_confidence")

    preferred_prefix = [
        "epoch_ms",
        "uptime_ms",
        "frame_start_uptime_ms",
        "frame_end_uptime_ms",
        "frame_dt_ms",
        "trial_id",
        "section_id",
        "session_id",
        "trial_number",
        "division_tag",
        "device_family",
        "device_family_code",
        "device_name",
        "parsed_load_family",
        "parsed_load_type",
        "load_type",
        TARGET,
    ]

    preferred_suffix = [
        "rf_train_row",
        "sample_weight",
        "label_trust",
        "clean_reason",
        "clean_quality",
        "source_kind",
        "source_priority",
        "trusted_normal_session",
        "context_family_code_runtime",
        "context_runtime_source",
        "context_augmented",
        "section_prior",
        "notes",
        "_source_file",
        "_source_folder",
    ]

    ordered = []
    seen = set()

    def add(cols):
        for c in cols:
            if c in df.columns and c not in seen:
                ordered.append(c)
                seen.add(c)

    add(preferred_prefix)
    add(feature_cols)
    add(context_cols)
    add(preferred_suffix)
    add(list(df.columns))
    return df.loc[:, ordered]


def _value_counts_dict(series: pd.Series) -> dict:
    if series is None:
        return {}
    s = series.fillna("unknown").astype(str)
    return {str(k): int(v) for k, v in s.value_counts().to_dict().items()}


def _numeric_feature_stats(df: pd.DataFrame) -> dict:
    stats = {}
    for col in [c for c in FEATURES if c in df.columns]:
        s = pd.to_numeric(df[col], errors="coerce").replace([np.inf, -np.inf], np.nan).dropna()
        if s.empty:
            stats[col] = {"rows": 0, "mean": 0.0, "std": 0.0, "min": 0.0, "max": 0.0}
        else:
            stats[col] = {
                "rows": int(len(s)),
                "mean": float(s.mean()),
                "std": float(s.std(ddof=0)),
                "min": float(s.min()),
                "max": float(s.max()),
            }
    return stats


def _numeric_feature_rows(df: pd.DataFrame) -> list[dict]:
    stats = _numeric_feature_stats(df)
    rows = []
    for feature_name, values in stats.items():
        rows.append({
            "feature": str(feature_name),
            "rows": int(values.get("rows", 0)),
            "mean": float(values.get("mean", 0.0)),
            "std": float(values.get("std", 0.0)),
            "min": float(values.get("min", 0.0)),
            "max": float(values.get("max", 0.0)),
        })
    return rows


def build_prepare_report(
    full_df: pd.DataFrame,
    arc_df: pd.DataFrame,
    context_df: pd.DataFrame,
    summary: dict,
    csv_files,
    csv_glob: str,
    arc_output: str,
    context_output: str,
    elapsed_s: float,
    *,
    selected_generation: str | None = None,
    generation_paths=None,
) -> dict:
    warnings = []
    arc_divs = sorted(set(arc_df.get("division_tag", pd.Series(dtype=object)).astype(str).str.lower().tolist())) if len(arc_df) else []
    ctx_divs = sorted(set(context_df.get("division_tag", pd.Series(dtype=object)).astype(str).str.lower().tolist())) if len(context_df) else []
    if any(d in {"start", "startup"} for d in arc_divs):
        warnings.append("arc_training_contains_start_rows")
    if any(d not in {"start", "startup"} for d in ctx_divs if d):
        warnings.append("load_context_contains_non_start_rows")
    if len(context_df) == 0:
        warnings.append("load_context_is_empty")
    if len(arc_df) == 0:
        warnings.append("arc_training_is_empty")
    if int(summary.get("context_only_placeholder_rows_preserved", 0) or 0) > 0:
        warnings.append("context_only_rows_used_placeholder_arc_target")

    active_generation = str(selected_generation or "legacy")
    resolved_paths = {
        "selected_generation": active_generation,
        "resolved_raw_input_glob": str(csv_glob),
        "resolved_arc_dataset_path": str(arc_output),
        "resolved_context_dataset_path": str(context_output),
        "benchmark_folder": str(getattr(generation_paths, "benchmark_dir", "")),
        "manifest_path": str(getattr(generation_paths, "manifest_path", "")),
        "archive_output_paths": getattr(generation_paths, "archive_output_paths", lambda: {})(),
        "canonical_export_paths": getattr(generation_paths, "canonical_export_paths", lambda: {})(),
    }

    full_mismatch = build_mismatch_summary(full_df, current_training_generation=selected_generation)
    arc_mismatch = build_mismatch_summary(arc_df, current_training_generation=selected_generation)
    context_mismatch = build_mismatch_summary(context_df, current_training_generation=selected_generation)
    feature_stats = {
        "arc_training": _numeric_feature_rows(arc_df),
        "load_context": _numeric_feature_rows(context_df),
    }

    return {
        "report_type": "prepare_data_report",
        "selected_generation": active_generation,
        "current_training_generation": active_generation,
        "feature_names": list(FEATURES),
        "source_csv_count": int(len(csv_files or [])),
        "source_csv_files": [str(p) for p in (csv_files or [])],
        "input_files": {
            "count": int(len(csv_files or [])),
            "files": [str(p) for p in (csv_files or [])],
        },
        "elapsed_seconds": float(elapsed_s),
        "elapsed_hms": time.strftime("%H:%M:%S", time.gmtime(max(0.0, float(elapsed_s)))),
        "timing": {
            "duration_seconds": float(elapsed_s),
            "duration_hms": time.strftime("%H:%M:%S", time.gmtime(max(0.0, float(elapsed_s)))),
        },
        "summary": dict(summary or {}),
        "resolved_paths": resolved_paths,
        "outputs": {
            "arc_training_path": str(arc_output),
            "load_context_path": str(context_output),
            "arc_training_rows": int(len(arc_df)),
            "load_context_rows": int(len(context_df)),
        },
        "warnings": warnings,
        "division_counts": {
            "cleaned_all": _value_counts_dict(full_df.get("division_tag", pd.Series(dtype=object))),
            "arc_training": _value_counts_dict(arc_df.get("division_tag", pd.Series(dtype=object))),
            "load_context": _value_counts_dict(context_df.get("division_tag", pd.Series(dtype=object))),
        },
        "split_checks": {
            "arc_training_division_counts": _value_counts_dict(arc_df.get("division_tag", pd.Series(dtype=object))),
            "load_context_division_counts": _value_counts_dict(context_df.get("division_tag", pd.Series(dtype=object))),
            "arc_training_label_counts": {str(k): int(v) for k, v in pd.to_numeric(arc_df.get(TARGET, pd.Series(dtype=float)), errors="coerce").fillna(-1).astype(int).value_counts().to_dict().items()} if TARGET in arc_df.columns else {},
            "load_context_family_counts": _value_counts_dict(context_df.get("device_family", pd.Series(dtype=object))),
            "warnings": warnings,
        },
        "full_dataset": {
            "rows": int(len(full_df)),
            "division_counts": _value_counts_dict(full_df.get("division_tag", pd.Series(dtype=object))),
            "label_counts": {str(k): int(v) for k, v in pd.to_numeric(full_df.get(TARGET, pd.Series(dtype=float)), errors="coerce").fillna(-1).astype(int).value_counts().to_dict().items()} if TARGET in full_df.columns else {},
            "device_family_counts": _value_counts_dict(full_df.get("device_family", pd.Series(dtype=object))),
        },
        "mismatch_summary": full_mismatch,
        "arc_training_mismatch_summary": arc_mismatch,
        "load_context_mismatch_summary": context_mismatch,
        "source_model_generation_summary": full_mismatch.get("source_model_generation_counts", {}),
        "feature_stats": feature_stats,
        "arc_training_feature_stats": _numeric_feature_stats(arc_df),
        "load_context_feature_stats": _numeric_feature_stats(context_df),
    }



def main():
    argv = sys.argv[1:]
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv_glob", default=DEFAULT_INPUT_GLOB)
    ap.add_argument("--output", default=DEFAULT_OUTPUT)
    ap.add_argument("--arc_output", default=None)
    ap.add_argument("--context_output", default=DEFAULT_CONTEXT_OUTPUT)
    ap.add_argument("--out_report", default=DEFAULT_REPORT)
    ap.add_argument("--generation", default=None)
    ap.add_argument("--augment_unknown_context", action="store_true")
    args = ap.parse_args()

    t0 = time.time()
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
                "csv_glob": ("--csv_glob", str(generation_paths.raw_dir / "**" / "*.csv")),
                "output": ("--output", generation_paths.arc_csv),
                "arc_output": ("--arc_output", generation_paths.arc_csv),
                "context_output": ("--context_output", generation_paths.context_csv),
                "out_report": ("--out_report", generation_paths.prepare_report),
            },
        )
        if not cli_flag_present("--csv_glob", argv):
            validation_errors = validate_generation_workspace(
                generation_paths,
                require_raw_folder=True,
                require_raw_csv=True,
            )
            if validation_errors:
                raise ValueError(" ; ".join(validation_errors))

    arc_output = args.arc_output or args.output

    csv_files = resolve_csv_files(args.csv_glob, arc_output)
    df = load_and_tag_csvs(
        csv_files,
        raw_root=getattr(generation_paths, "raw_dir", None),
        current_training_generation=selected_generation,
    )
    df, summary = clean_dataset(df, augment_unknown_context=args.augment_unknown_context)
    df = annotate_generation_and_mismatch_columns(df, current_training_generation=selected_generation)

    division = df.get("division_tag", pd.Series("steady", index=df.index)).astype(str).str.lower()
    arc_df = reorder_export_columns(df[division.isin(["steady", "arc"])].copy())
    context_df = reorder_export_columns(df[division.isin(["start", "startup"])].copy())

    ensure_dir(arc_output)
    arc_df.to_csv(arc_output, index=False)
    ensure_dir(args.context_output)
    context_df.to_csv(args.context_output, index=False)

    elapsed_s = float(time.time() - t0)
    summary["arc_training_rows"] = int(len(arc_df))
    summary["load_context_rows"] = int(len(context_df))
    summary["arc_feature_count_available"] = int(len([c for c in ARC_SWEEP_FEATURES if c in arc_df.columns]))
    summary["context_feature_count_available"] = int(len([c for c in CONTEXT_SWEEP_FEATURES if c in context_df.columns]))
    summary["arc_training_path"] = arc_output
    summary["load_context_path"] = args.context_output
    summary["selected_generation"] = str(selected_generation or "legacy")
    summary["resolved_raw_input_glob"] = str(args.csv_glob)
    summary["elapsed_seconds"] = elapsed_s
    summary["elapsed_hms"] = time.strftime("%H:%M:%S", time.gmtime(max(0.0, elapsed_s)))
    summary["mismatch_rows"] = int(build_mismatch_summary(df).get("mismatch_rows", 0))
    summary["fp_override_rows"] = int(build_mismatch_summary(df).get("fp_override_rows", 0))
    summary["fn_override_rows"] = int(build_mismatch_summary(df).get("fn_override_rows", 0))

    report = build_prepare_report(
        full_df=df,
        arc_df=arc_df,
        context_df=context_df,
        summary=summary,
        csv_files=csv_files,
        csv_glob=args.csv_glob,
        arc_output=arc_output,
        context_output=args.context_output,
        elapsed_s=elapsed_s,
        selected_generation=selected_generation,
        generation_paths=generation_paths,
    )

    if args.out_report:
        ensure_dir(args.out_report)
        with open(args.out_report, "w", encoding="utf-8") as f:
            json.dump(report, f, indent=2)
        print("Saved prepare report to:", args.out_report)

    if generation_paths is not None:
        update_generation_manifest(
            generation_paths,
            {
                "selected_generation": selected_generation,
                "source_csv_count": int(len(csv_files)),
                "source_model_generations_seen": sorted(
                    {
                        str(v)
                        for v in annotate_generation_and_mismatch_columns(df, current_training_generation=selected_generation)
                        .get("source_model_generation", pd.Series(dtype=object))
                        .astype(str)
                        .tolist()
                        if str(v).strip()
                    }
                ),
                "prepare_data": {
                    "report_path": str(args.out_report),
                    "csv_glob": str(args.csv_glob),
                    "arc_training_path": str(arc_output),
                    "load_context_path": str(args.context_output),
                    "summary": dict(summary or {}),
                },
                "recommended_next_action": f"Run subset sweep or training for {selected_generation} after reviewing tinyml/benchmark/{selected_generation}.",
            },
        )

    if selected_generation:
        print(f"Selected generation: {selected_generation}")
    print("Saved arc training dataset to:", arc_output)
    print("Saved load context dataset to:", args.context_output)
    print("__CLEANER_SUMMARY__ " + json.dumps(summary, sort_keys=True))


if __name__ == "__main__":
    main()
