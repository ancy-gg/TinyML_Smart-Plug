import json
import os
import re
import warnings
from dataclasses import dataclass
from typing import Dict, List, Tuple

import joblib
try:
    import m2cgen as m2c
except Exception:
    m2c = None
import numpy as np
import pandas as pd
try:
    from generation_paths import normalize_generation_tag
except Exception:
    from trainer.generation_paths import normalize_generation_tag

from sklearn.ensemble import ExtraTreesClassifier, RandomForestClassifier
from sklearn.metrics import (
    accuracy_score,
    average_precision_score,
    balanced_accuracy_score,
    classification_report,
    confusion_matrix,
    f1_score,
    matthews_corrcoef,
    precision_score,
    recall_score,
    roc_auc_score,
)
from sklearn.base import clone
from sklearn.model_selection import GroupKFold, GroupShuffleSplit, StratifiedShuffleSplit

try:
    from sklearn.model_selection import StratifiedGroupKFold
except Exception:
    StratifiedGroupKFold = None


LEGACY_COMPUTED_FEATURES = [
    "abs_irms_zscore_vs_baseline",
    "delta_irms_abs",
    "halfcycle_asymmetry",
    "suspicious_run_energy",
    "delta_hf_energy",
    "delta_flux",
    "midband_residual_ratio",
    "zcv",
    "spectral_flux_midhf",
    "peak_fluct_cv",
    "residual_crest_factor",
    "thd_i",
    "hf_energy_delta",
    "edge_spike_ratio",
    "v_sag_pct",
    "cycle_nmse",
]

EXTRA_COMPUTED_FEATURES = [
    "pulse_count_per_cycle",
    "zero_dwell_ratio",
    "low_current_ratio",
    "max_low_current_run_ms",
]

ALL_COMPUTED_FEATURES = list(LEGACY_COMPUTED_FEATURES) + list(EXTRA_COMPUTED_FEATURES)

# Curated candidate pool for subset sweep. Keep this pool intentionally smaller
# than the full computed feature space so combination search remains practical,
# but bias it toward restrike / zero-current-gap features because those map better
# to separation-recontact arcs than generic temporal deltas alone.
ARC_SWEEP_FEATURES = [
    "pulse_count_per_cycle",
    "max_low_current_run_ms",
    "zero_dwell_ratio",
    "low_current_ratio",
    "abs_irms_zscore_vs_baseline",
    "delta_irms_abs",
    "halfcycle_asymmetry",
    "zcv",
    "midband_residual_ratio",
    "edge_spike_ratio",
    "residual_crest_factor",
    "thd_i",
    "spectral_flux_midhf",
    "hf_energy_delta",
    "peak_fluct_cv",
]

CONTEXT_SWEEP_FEATURES = [
    "delta_irms_abs",
    "halfcycle_asymmetry",
    "midband_residual_ratio",
    "abs_irms_zscore_vs_baseline",
    "zcv",
    "residual_crest_factor",
    "peak_fluct_cv",
    "thd_i",
    "spectral_flux_midhf",
    "hf_energy_delta",
]

ARC_PWA_VISIBLE_FEATURES = [
    "pulse_count_per_cycle",
    "max_low_current_run_ms",
    "zero_dwell_ratio",
    "low_current_ratio",
    "thd_i",
    "spectral_flux_midhf",
    "hf_energy_delta",
    "residual_crest_factor",
    "peak_fluct_cv",
    "zcv",
    "delta_irms_abs",
    "midband_residual_ratio",
    "edge_spike_ratio",
]
ARC_DEFAULT_BASE_FEATURES = list(ALL_COMPUTED_FEATURES)
CONTEXT_DEFAULT_FEATURES = list(CONTEXT_SWEEP_FEATURES)
ARC_BASE_FEATURES_RANKED = [
    "pulse_count_per_cycle",
    "max_low_current_run_ms",
    "zero_dwell_ratio",
    "low_current_ratio",
    "abs_irms_zscore_vs_baseline",
    "delta_irms_abs",
    "halfcycle_asymmetry",
    "zcv",
    "midband_residual_ratio",
    "edge_spike_ratio",
    "residual_crest_factor",
    "thd_i",
    "spectral_flux_midhf",
    "hf_energy_delta",
    "peak_fluct_cv",
    "cycle_nmse",
    "suspicious_run_energy",
    "delta_hf_energy",
    "delta_flux",
    "v_sag_pct",
]
ARC_ALL_COMPUTED_FEATURES = list(ALL_COMPUTED_FEATURES)
CONTEXT_ALL_COMPUTED_FEATURES = list(ALL_COMPUTED_FEATURES)

DEVICE_FAMILY_CLASSES = [
    "resistive_linear",
    "inductive_motor",
    "rectifier_smps",
    "phase_angle_controlled",
    "brush_universal_motor",
    "other_mixed",
]
DEVICE_FAMILY_UNKNOWN_CODE = -1
DEVICE_FAMILY_CODE_MAP = {fam: idx for idx, fam in enumerate(DEVICE_FAMILY_CLASSES)}
DEVICE_FAMILY_NAME_FROM_CODE = {idx: fam for fam, idx in DEVICE_FAMILY_CODE_MAP.items()}
DEVICE_FAMILY_CODE_MAP_WITH_UNKNOWN = {"unknown": DEVICE_FAMILY_UNKNOWN_CODE, **DEVICE_FAMILY_CODE_MAP}

ARC_CONTEXT_FEATURES = [f"ctx_family_{fam}" for fam in DEVICE_FAMILY_CLASSES] + ["context_family_confidence"]
ARC_FEATURES = list(ARC_DEFAULT_BASE_FEATURES) + list(ARC_CONTEXT_FEATURES)
CONTEXT_FEATURES = list(CONTEXT_DEFAULT_FEATURES)
FEATURES = list(ALL_COMPUTED_FEATURES)
MODEL_INPUT_FEATURE_ORDER = list(LEGACY_COMPUTED_FEATURES) + list(ARC_CONTEXT_FEATURES) + list(EXTRA_COMPUTED_FEATURES)
MODEL_INPUT_FEATURE_ID_MAP = {name: idx for idx, name in enumerate(MODEL_INPUT_FEATURE_ORDER)}

TARGET = "label_arc"
CONTEXT_TARGET = "device_family_code"
GROUP_COL_CANDIDATES = ["trial_id", "session_id", "section_id", "session", "sid"]

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

SCAFFOLD_SOURCE_NAMES = {"scaffold", "synthetic", "augmented_scaffold"}
SCAFFOLD_MAX_SHARE_OF_REAL = 0.45
SCAFFOLD_MAX_BUCKET_WEIGHT_NO_REAL = 6.0
SCAFFOLD_MIN_ROW_WEIGHT = 0.05
SCAFFOLD_MIN_BUCKETS_FOR_DEVICE_CAP = 1

ARC_TOLERANCE_MODES = ("none", "expanded_positive", "soft_positive")


@dataclass(frozen=True)
class ArcToleranceConfig:
    mode: str = "soft_positive"
    pre_rows: int = 1
    post_rows: int = 3
    soft_neighbor_weight: float = 0.30
    expanded_neighbor_weight: float = 0.70
    hard_negative_ring: int = 2

    def normalized_mode(self) -> str:
        mode = str(self.mode or "none").strip().lower()
        if mode in ("expanded", "expanded_positive", "expanded-positive", "expand"):
            return "expanded_positive"
        if mode in ("soft", "soft_positive", "soft-positive", "soft_label"):
            return "soft_positive"
        return "none"

    def event_pre_rows(self) -> int:
        return max(0, int(self.pre_rows))

    def event_post_rows(self) -> int:
        return max(0, int(self.post_rows))


def normalize_arc_tolerance_config(
    tolerance: ArcToleranceConfig | dict | None = None,
    *,
    mode: str | None = None,
    pre_rows: int | None = None,
    post_rows: int | None = None,
    soft_neighbor_weight: float | None = None,
    expanded_neighbor_weight: float | None = None,
    hard_negative_ring: int | None = None,
) -> ArcToleranceConfig:
    if isinstance(tolerance, ArcToleranceConfig):
        base = tolerance
    elif isinstance(tolerance, dict):
        base = ArcToleranceConfig(
            mode=str(tolerance.get("mode", "soft_positive")),
            pre_rows=int(tolerance.get("pre_rows", tolerance.get("pre_arc_window", 1)) or 0),
            post_rows=int(tolerance.get("post_rows", tolerance.get("post_arc_window", 3)) or 0),
            soft_neighbor_weight=float(tolerance.get("soft_neighbor_weight", 0.30)),
            expanded_neighbor_weight=float(tolerance.get("expanded_neighbor_weight", 0.70)),
            hard_negative_ring=int(tolerance.get("hard_negative_ring", 2) or 0),
        )
    else:
        base = ArcToleranceConfig()
    cfg = ArcToleranceConfig(
        mode=base.mode if mode is None else mode,
        pre_rows=base.pre_rows if pre_rows is None else int(pre_rows),
        post_rows=base.post_rows if post_rows is None else int(post_rows),
        soft_neighbor_weight=base.soft_neighbor_weight if soft_neighbor_weight is None else float(soft_neighbor_weight),
        expanded_neighbor_weight=base.expanded_neighbor_weight if expanded_neighbor_weight is None else float(expanded_neighbor_weight),
        hard_negative_ring=base.hard_negative_ring if hard_negative_ring is None else int(hard_negative_ring),
    )
    return ArcToleranceConfig(
        mode=cfg.normalized_mode(),
        pre_rows=max(0, int(cfg.pre_rows)),
        post_rows=max(0, int(cfg.post_rows)),
        soft_neighbor_weight=float(min(1.0, max(0.02, cfg.soft_neighbor_weight))),
        expanded_neighbor_weight=float(min(1.25, max(0.10, cfg.expanded_neighbor_weight))),
        hard_negative_ring=max(0, int(cfg.hard_negative_ring)),
    )


def default_feature_names_for_target(target_col: str = TARGET) -> list[str]:
    return list(CONTEXT_FEATURES if str(target_col) == str(CONTEXT_TARGET) else ARC_FEATURES)


def validate_feature_subset(
    feature_names,
    *,
    allowed_features,
    role: str,
    allow_empty: bool = False,
) -> list[str]:
    allowed = list(allowed_features or [])
    allowed_set = set(allowed)
    cleaned = []
    invalid = []
    seen = set()
    for raw in (feature_names or []):
        name = str(raw).strip()
        if not name or name in seen:
            continue
        seen.add(name)
        if name not in allowed_set:
            invalid.append(name)
            continue
        cleaned.append(name)
    if invalid:
        raise ValueError(f"{role} feature list contains unsupported features: {invalid}")
    if not cleaned and not allow_empty:
        raise ValueError(f"{role} feature list must include at least one feature from: {allowed}")
    return cleaned


def feature_ids_for_names(feature_names) -> list[int]:
    ids = []
    for name in feature_names or []:
        key = str(name).strip()
        if key not in MODEL_INPUT_FEATURE_ID_MAP:
            raise ValueError(f"Unknown model input feature name: {key}")
        ids.append(int(MODEL_INPUT_FEATURE_ID_MAP[key]))
    return ids


def split_arc_feature_names(feature_names) -> tuple[list[str], list[str]]:
    names = [str(x).strip() for x in (feature_names or []) if str(x).strip()]
    base_names = [name for name in names if name not in ARC_CONTEXT_FEATURES]
    context_names = [name for name in names if name in ARC_CONTEXT_FEATURES]
    if context_names and context_names != list(ARC_CONTEXT_FEATURES):
        raise ValueError(
            "Arc model inputs must append the 7 fixed runtime context inputs in canonical order: "
            + ", ".join(ARC_CONTEXT_FEATURES)
        )
    return base_names, context_names


def describe_feature_subset(feature_names) -> dict:
    names = [str(x).strip() for x in (feature_names or []) if str(x).strip()]
    ids = feature_ids_for_names(names)
    return {
        "feature_names": names,
        "feature_ids": ids,
        "feature_count": len(names),
    }


def _series_from(values) -> pd.Series:
    if isinstance(values, pd.Series):
        return pd.to_numeric(values, errors="coerce")
    return pd.Series(pd.to_numeric(values, errors="coerce"), dtype=float)


def _series_from_default(index, values, default=0.0, dtype=float) -> pd.Series:
    if isinstance(values, pd.Series):
        return pd.to_numeric(values, errors="coerce")
    return pd.Series(default if values is None else values, index=index, dtype=dtype)


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


def _scaffold_mask(df: pd.DataFrame) -> pd.Series:
    source = df.get("source_kind", pd.Series("real", index=df.index)).astype(str).str.lower().str.strip()
    mask = source.isin(SCAFFOLD_SOURCE_NAMES) | source.str.contains("scaffold", na=False)
    if "is_scaffold" in df.columns:
        mask |= pd.to_numeric(df["is_scaffold"], errors="coerce").fillna(0).astype(int) == 1
    if "context_runtime_source" in df.columns:
        mask |= df["context_runtime_source"].astype(str).str.lower().str.contains("scaffold", na=False)
    return pd.Series(mask, index=df.index, dtype=bool)


def apply_scaffold_gap_fill_cap(
    df: pd.DataFrame,
    *,
    target_col: str = TARGET,
    weight_col: str = "sample_weight",
    trainable_col: str | None = "rf_train_row",
    bucket_cols: list[str] | None = None,
) -> pd.DataFrame:
    work = df.copy()
    if work.empty:
        return work

    if weight_col not in work.columns:
        work[weight_col] = 1.0
    work[weight_col] = pd.to_numeric(work[weight_col], errors="coerce").fillna(1.0).clip(SCAFFOLD_MIN_ROW_WEIGHT, 100.0)

    if trainable_col and trainable_col in work.columns:
        trainable_mask = pd.to_numeric(work[trainable_col], errors="coerce").fillna(0).astype(int) == 1
    else:
        trainable_mask = pd.Series(True, index=work.index, dtype=bool)

    scaffold_mask = _scaffold_mask(work)
    work["is_scaffold"] = scaffold_mask.astype(int)
    work["scaffold_weight_scale"] = 1.0
    work["scaffold_cap_reason"] = work.get("scaffold_cap_reason", pd.Series("", index=work.index, dtype=object)).astype(str)

    if bucket_cols is None:
        if target_col == TARGET:
            bucket_cols = [c for c in ["device_family", "device_name", target_col] if c in work.columns]
            if not bucket_cols:
                bucket_cols = [target_col] if target_col in work.columns else []
        else:
            bucket_cols = [c for c in ["device_family", target_col] if c in work.columns]
            if not bucket_cols:
                bucket_cols = [target_col] if target_col in work.columns else []
    if not bucket_cols:
        return work

    def _apply_caps(cols: list[str], tag: str):
        if not cols:
            return
        eligible = work.loc[trainable_mask].copy()
        if eligible.empty:
            return
        grouped = eligible.groupby(cols, dropna=False, sort=False)
        for _, g in grouped:
            idx = g.index.tolist()
            g = work.loc[idx]
            s_mask = scaffold_mask.loc[idx]
            if not bool(s_mask.any()):
                continue
            real_sum = float(pd.to_numeric(g.loc[~s_mask, weight_col], errors="coerce").fillna(0.0).sum())
            scaff_sum = float(pd.to_numeric(g.loc[s_mask, weight_col], errors="coerce").fillna(0.0).sum())
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
            s_idx = g.index[s_mask]
            work.loc[s_idx, weight_col] = np.maximum(SCAFFOLD_MIN_ROW_WEIGHT, pd.to_numeric(work.loc[s_idx, weight_col], errors="coerce").fillna(0.0) * scale)
            work.loc[s_idx, "scaffold_weight_scale"] = np.minimum(pd.to_numeric(work.loc[s_idx, "scaffold_weight_scale"], errors="coerce").fillna(1.0), scale)
            empty_reason = work.loc[s_idx, "scaffold_cap_reason"].astype(str).str.len() == 0
            work.loc[s_idx[empty_reason], "scaffold_cap_reason"] = reason

    family_cols = [c for c in bucket_cols if c != "device_name"]
    device_cols = bucket_cols if "device_name" in bucket_cols else []
    _apply_caps(family_cols, "family")
    if len(device_cols) >= SCAFFOLD_MIN_BUCKETS_FOR_DEVICE_CAP:
        _apply_caps(device_cols, "device")

    work[weight_col] = pd.to_numeric(work[weight_col], errors="coerce").fillna(1.0).clip(SCAFFOLD_MIN_ROW_WEIGHT, 100.0)
    return work


def augment_unknown_context_rows(df: pd.DataFrame, target_col: str = TARGET) -> pd.DataFrame:
    work = normalize_feature_names(df.copy())
    if work.empty:
        return work
    work["context_family_code_runtime"] = -1
    work["context_family_confidence"] = 0.0
    for fam in DEVICE_FAMILY_CLASSES:
        work[f"ctx_family_{fam}"] = 0.0
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
    work = apply_scaffold_gap_fill_cap(work, target_col=target_col, trainable_col=None, bucket_cols=[c for c in ["device_family", target_col] if c in work.columns])
    return work



def resolve_n_jobs(default: int = -1) -> int:
    raw = os.environ.get("TINYML_N_JOBS", "").strip()
    if not raw:
        return int(default)
    try:
        value = int(raw)
    except Exception:
        return int(default)
    if value == 0:
        return int(default)
    return int(value)


def estimator_n_jobs(default: int = -1):
    value = resolve_n_jobs(default)
    # sklearn/joblib can still allocate a worker pool when n_jobs==1 in some
    # Windows environments. Use None to force a truly serial fit in that case.
    return None if int(value) == 1 else int(value)


MODEL_CONFIGS = {
    "rf": {
        "pretty_name": "RandomForest",
        "builder": lambda: RandomForestClassifier(
            random_state=42,
            class_weight="balanced_subsample",
            n_jobs=estimator_n_jobs(-1),
            bootstrap=True,
            oob_score=False,
        ),
        "param_space": {
            "n_estimators": [80, 120, 160, 220, 280, 360],
            "max_depth": [4, 6, 8, 10, 12, 16, 20, None],
            "min_samples_leaf": [1, 2, 3, 4, 6, 8],
            "min_samples_split": [2, 4, 6, 8, 12, 16],
            "max_features": ["sqrt", "log2", 0.35, 0.5, 0.65, 0.8, 1.0],
            "bootstrap": [True, False],
            "class_weight": ["balanced", "balanced_subsample", None],
            "criterion": ["gini", "entropy"],
            "ccp_alpha": [0.0, 0.00001, 0.00005, 0.0001, 0.0002, 0.0005, 0.001],
            "max_leaf_nodes": [None, 32, 48, 64, 96, 128, 160],
            "min_impurity_decrease": [0.0, 0.000001, 0.00001, 0.00005, 0.0001],
        },
    },
    "et": {
        "pretty_name": "ExtraTrees",
        "builder": lambda: ExtraTreesClassifier(
            random_state=42,
            class_weight="balanced",
            n_jobs=estimator_n_jobs(-1),
            bootstrap=False,
        ),
        "param_space": {
            "n_estimators": [80, 120, 160, 220, 280, 360],
            "max_depth": [4, 6, 8, 10, 12, 16, 20, None],
            "min_samples_leaf": [1, 2, 3, 4, 6, 8],
            "min_samples_split": [2, 4, 6, 8, 12, 16],
            "max_features": ["sqrt", "log2", 0.35, 0.5, 0.65, 0.8, 1.0],
            "bootstrap": [False, True],
            "class_weight": ["balanced", None],
            "criterion": ["gini", "entropy"],
            "ccp_alpha": [0.0, 0.00001, 0.00005, 0.0001, 0.0002, 0.0005, 0.001],
            "max_leaf_nodes": [None, 32, 48, 64, 96, 128, 160],
            "min_impurity_decrease": [0.0, 0.000001, 0.00001, 0.00005, 0.0001],
        },
    },
}

MODEL_SEARCH_PROFILES = {
    "rf": {
        "full": dict(MODEL_CONFIGS["rf"]["param_space"]),
        "fast_subset": {
            "n_estimators": [48, 64, 80, 96, 128],
            "max_depth": [4, 6, 8, 10, 12],
            "min_samples_leaf": [2, 3, 4, 6, 8],
            "min_samples_split": [4, 6, 8, 12, 16],
            "max_features": ["sqrt", "log2", 0.35, 0.5, 0.65],
            "bootstrap": [True, False],
            "class_weight": ["balanced", "balanced_subsample"],
            "criterion": ["gini", "entropy"],
            "ccp_alpha": [0.0, 0.00001, 0.00005, 0.0001],
            "max_leaf_nodes": [32, 48, 64, 96, 128],
            "min_impurity_decrease": [0.0, 0.000001, 0.00001, 0.00005],
        },
        "final_rf": {
            "n_estimators": [160, 220, 280, 360],
            "max_depth": [8, 10, 12, 16, 20, None],
            "min_samples_leaf": [1, 2, 3, 4, 6],
            "min_samples_split": [2, 4, 6, 8, 12],
            "max_features": ["sqrt", "log2", 0.35, 0.5, 0.65, 0.8],
            "bootstrap": [True, False],
            "class_weight": ["balanced", "balanced_subsample", None],
            "criterion": ["gini", "entropy"],
            "ccp_alpha": [0.0, 0.00001, 0.00005, 0.0001, 0.0002],
            "max_leaf_nodes": [None, 48, 64, 96, 128, 160],
            "min_impurity_decrease": [0.0, 0.000001, 0.00001, 0.00005],
        },
    },
    "et": {
        "full": dict(MODEL_CONFIGS["et"]["param_space"]),
        "fast_subset": {
            "n_estimators": [48, 64, 80, 96, 128],
            "max_depth": [4, 6, 8, 10, 12],
            "min_samples_leaf": [2, 3, 4, 6, 8],
            "min_samples_split": [4, 6, 8, 12, 16],
            "max_features": ["sqrt", "log2", 0.35, 0.5, 0.65],
            "bootstrap": [False, True],
            "class_weight": ["balanced", None],
            "criterion": ["gini", "entropy"],
            "ccp_alpha": [0.0, 0.00001, 0.00005, 0.0001],
            "max_leaf_nodes": [32, 48, 64, 96, 128],
            "min_impurity_decrease": [0.0, 0.000001, 0.00001, 0.00005],
        },
        "final_et": {
            "n_estimators": [160, 220, 280, 360],
            "max_depth": [8, 10, 12, 16, 20, None],
            "min_samples_leaf": [1, 2, 3, 4, 6],
            "min_samples_split": [2, 4, 6, 8, 12],
            "max_features": ["sqrt", "log2", 0.35, 0.5, 0.65, 0.8],
            "bootstrap": [False, True],
            "class_weight": ["balanced", None],
            "criterion": ["gini", "entropy"],
            "ccp_alpha": [0.0, 0.00001, 0.00005, 0.0001, 0.0002],
            "max_leaf_nodes": [None, 48, 64, 96, 128, 160],
            "min_impurity_decrease": [0.0, 0.000001, 0.00001, 0.00005],
        },
    },
}


def resolve_model_search_profile(model_key: str, profile: str | None = None) -> dict:
    key = str(model_key).strip().lower()
    profile_key = str(profile or "full").strip().lower()
    profiles = MODEL_SEARCH_PROFILES.get(key, {})
    return dict(profiles.get(profile_key) or profiles.get("full") or MODEL_CONFIGS[key]["param_space"])



def ensure_dir(path: str) -> None:
    folder = os.path.dirname(path)
    if folder:
        os.makedirs(folder, exist_ok=True)


def _series_boolish_to_int(series, index, default: int = 0) -> pd.Series:
    if isinstance(series, pd.Series):
        raw = series.reindex(index)
    else:
        raw = pd.Series(default, index=index)
    if raw.dtype == bool:
        return raw.fillna(False).astype(int)
    text = raw.astype(str).str.strip().str.lower()
    truthy = text.isin({"1", "true", "t", "yes", "y", "verified", "human_verified"})
    falsy = text.isin({"0", "false", "f", "no", "n", "", "nan", "none"})
    numeric = pd.to_numeric(raw, errors="coerce")
    out = pd.Series(default, index=index, dtype=int)
    out.loc[numeric.notna()] = (numeric.loc[numeric.notna()] > 0).astype(int)
    out.loc[truthy] = 1
    out.loc[falsy] = 0
    return out.astype(int)


def normalize_training_generation(value: str | None, *, fallback: str = "legacy") -> str:
    try:
        normalized = normalize_generation_tag(value, allow_none=True)
    except Exception:
        normalized = None
    return str(normalized or fallback)


def annotate_generation_and_mismatch_columns(
    df: pd.DataFrame,
    *,
    current_training_generation: str | None = None,
) -> pd.DataFrame:
    work = df.copy()
    current_generation = normalize_training_generation(current_training_generation)

    score_alias = None
    for candidate in ("model_arc_score", "arc_model_score", "model_score", "arc_score"):
        if candidate in work.columns:
            score_alias = candidate
            break
    if score_alias is not None:
        work["model_arc_score"] = pd.to_numeric(work[score_alias], errors="coerce")
    else:
        work["model_arc_score"] = np.nan

    pred_alias = None
    for candidate in ("model_arc_pred", "model_pred", "arc_model_pred"):
        if candidate in work.columns:
            pred_alias = candidate
            break
    if pred_alias is not None:
        model_pred = pd.to_numeric(work[pred_alias], errors="coerce")
        valid_pred = model_pred.isin([0, 1])
        work["model_arc_pred"] = np.where(valid_pred, model_pred.astype(float), np.nan)
    else:
        work["model_arc_pred"] = np.nan

    work["label_verified"] = _series_boolish_to_int(work.get("label_verified"), work.index, default=0)

    if "current_training_generation" in work.columns and current_training_generation is None:
        current_series = work["current_training_generation"].astype(str).map(lambda v: normalize_training_generation(v, fallback=current_generation))
    else:
        current_series = pd.Series(current_generation, index=work.index, dtype=object)
    work["current_training_generation"] = current_series.astype(str)

    if "source_model_generation" in work.columns:
        raw_source_generation = work["source_model_generation"]
    elif "model_generation" in work.columns:
        raw_source_generation = work["model_generation"]
    else:
        raw_source_generation = pd.Series("", index=work.index, dtype=object)
    source_model_generation = raw_source_generation.astype(str).map(
        lambda v: normalize_training_generation(v, fallback="unknown") if str(v).strip() else "unknown"
    )
    work["source_model_generation"] = source_model_generation.astype(str)
    work["generation_tag"] = source_model_generation.where(source_model_generation.ne("unknown"), current_series.astype(str))

    if "label_truth_source" not in work.columns:
        truth_source = np.where(
            work["label_verified"].astype(int) == 1,
            "human_verified",
            "human_label",
        )
        work["label_truth_source"] = pd.Series(truth_source, index=work.index, dtype=object)
    else:
        work["label_truth_source"] = work["label_truth_source"].fillna("human_label").astype(str)

    labels = pd.to_numeric(work.get(TARGET, pd.Series(np.nan, index=work.index)), errors="coerce")
    preds = pd.to_numeric(work["model_arc_pred"], errors="coerce")
    mismatch_mask = labels.isin([0, 1]) & preds.isin([0, 1]) & labels.ne(preds)
    mismatch_type = pd.Series("", index=work.index, dtype=object)
    mismatch_type.loc[(preds == 1) & (labels == 0)] = "fp_override"
    mismatch_type.loc[(preds == 0) & (labels == 1)] = "fn_override"
    work["reality_predict_mismatch"] = mismatch_mask.astype(int)
    work["mismatch_type"] = mismatch_type.astype(str)
    return work


def build_mismatch_summary(df: pd.DataFrame, *, current_training_generation: str | None = None) -> dict:
    if not isinstance(df, pd.DataFrame) or df.empty:
        return {
            "rows": 0,
            "rows_with_model_output": 0,
            "mismatch_rows": 0,
            "mismatch_rate": 0.0,
            "fp_override_rows": 0,
            "fn_override_rows": 0,
            "verified_rows": 0,
            "verified_mismatch_rows": 0,
            "source_model_generation_counts": {},
            "per_generation_mismatch_counts": {},
            "per_family_mismatch_counts": {},
            "per_device_mismatch_counts": {},
        }

    work = annotate_generation_and_mismatch_columns(df, current_training_generation=current_training_generation)
    model_pred = pd.to_numeric(work.get("model_arc_pred", pd.Series(np.nan, index=work.index)), errors="coerce")
    has_model_output = model_pred.isin([0, 1]) | pd.to_numeric(work.get("model_arc_score", pd.Series(np.nan, index=work.index)), errors="coerce").notna()
    mismatch = pd.to_numeric(work.get("reality_predict_mismatch", pd.Series(0, index=work.index)), errors="coerce").fillna(0).astype(int) == 1
    mismatch_type = work.get("mismatch_type", pd.Series("", index=work.index)).astype(str)
    verified = _series_boolish_to_int(work.get("label_verified"), work.index, default=0) == 1

    def _counts(series: pd.Series, mask: pd.Series | None = None) -> dict[str, int]:
        src = series if mask is None else series.loc[mask]
        if src.empty:
            return {}
        src = src.fillna("unknown").astype(str).str.strip()
        src = src.replace("", "unknown")
        return {str(k): int(v) for k, v in src.value_counts().to_dict().items()}

    family_series = work.get("device_family", pd.Series("unknown", index=work.index)).astype(str)
    device_series = work.get("device_name", pd.Series("unknown_device", index=work.index)).astype(str)
    source_generation_series = work.get("source_model_generation", pd.Series("unknown", index=work.index)).astype(str)

    rows_with_model_output = int(has_model_output.sum())
    mismatch_rows = int(mismatch.sum())
    mismatch_rate = float(mismatch_rows / rows_with_model_output) if rows_with_model_output > 0 else 0.0
    return {
        "rows": int(len(work)),
        "rows_with_model_output": rows_with_model_output,
        "mismatch_rows": mismatch_rows,
        "mismatch_rate": mismatch_rate,
        "fp_override_rows": int(((mismatch_type == "fp_override") & mismatch).sum()),
        "fn_override_rows": int(((mismatch_type == "fn_override") & mismatch).sum()),
        "verified_rows": int(verified.sum()),
        "verified_mismatch_rows": int((verified & mismatch).sum()),
        "source_model_generation_counts": _counts(source_generation_series, has_model_output),
        "per_generation_mismatch_counts": _counts(source_generation_series, mismatch),
        "per_family_mismatch_counts": _counts(family_series, mismatch),
        "per_device_mismatch_counts": _counts(device_series, mismatch),
        "current_training_generation_counts": _counts(work.get("current_training_generation", pd.Series("legacy", index=work.index)).astype(str)),
    }


def build_holdout_mismatch_summary(
    meta_df: pd.DataFrame | None,
    y_true,
    y_score,
    threshold: float,
    current_training_generation: str | None = None,
) -> dict:
    if meta_df is None or not isinstance(meta_df, pd.DataFrame) or meta_df.empty:
        out = build_mismatch_summary(pd.DataFrame())
        out.update({
            "held_out_rows": 0,
            "threshold": float(threshold),
            "current_model_correct_rows": 0,
            "current_model_error_rows": 0,
            "current_model_correction_rate": 0.0,
        })
        return out

    work = annotate_generation_and_mismatch_columns(meta_df, current_training_generation=current_training_generation)
    summary = build_mismatch_summary(work)
    mismatch_mask = pd.to_numeric(work.get("reality_predict_mismatch", pd.Series(0, index=work.index)), errors="coerce").fillna(0).astype(int) == 1
    y_true_arr = np.asarray(y_true).astype(int)
    y_pred_arr = (np.asarray(y_score).astype(float) >= float(threshold)).astype(int)
    if len(y_true_arr) != len(work):
        summary.update({
            "held_out_rows": int(len(work)),
            "threshold": float(threshold),
            "current_model_correct_rows": 0,
            "current_model_error_rows": 0,
            "current_model_correction_rate": 0.0,
        })
        return summary
    if bool(mismatch_mask.any()):
        current_model_correct = int(np.sum(y_pred_arr[mismatch_mask.to_numpy()] == y_true_arr[mismatch_mask.to_numpy()]))
        current_model_error = int(np.sum(y_pred_arr[mismatch_mask.to_numpy()] != y_true_arr[mismatch_mask.to_numpy()]))
        correction_rate = float(current_model_correct / int(mismatch_mask.sum())) if int(mismatch_mask.sum()) > 0 else 0.0
    else:
        current_model_correct = 0
        current_model_error = 0
        correction_rate = 0.0
    summary.update({
        "held_out_rows": int(len(work)),
        "threshold": float(threshold),
        "current_model_correct_rows": current_model_correct,
        "current_model_error_rows": current_model_error,
        "current_model_correction_rate": correction_rate,
    })
    return summary


def assess_next_generation_readiness(
    result: dict,
    *,
    min_recall: float,
    min_precision: float,
    max_fpr: float,
) -> dict:
    validation_recall = float(result.get("holdout_validation_recall", result.get("validation_recall", 0.0)) or 0.0)
    validation_precision = float(result.get("holdout_validation_precision", result.get("validation_precision", 0.0)) or 0.0)
    validation_fpr = float(result.get("validation_fpr", 1.0) or 1.0)
    validation_event = result.get("validation_event_level_metrics") or {}
    missed_events = int(validation_event.get("missed_event_count", 0) or 0)
    false_alarms = float(validation_event.get("false_alarms_per_session", 0.0) or 0.0)
    justified = bool(
        validation_recall >= float(min_recall)
        and validation_precision >= float(min_precision)
        and validation_fpr <= float(max_fpr)
        and missed_events <= 1
    )
    reasons = []
    if validation_recall < float(min_recall):
        reasons.append(f"holdout recall {validation_recall:.4f} < {float(min_recall):.4f}")
    if validation_precision < float(min_precision):
        reasons.append(f"holdout precision {validation_precision:.4f} < {float(min_precision):.4f}")
    if validation_fpr > float(max_fpr):
        reasons.append(f"holdout FPR {validation_fpr:.4f} > {float(max_fpr):.4f}")
    if missed_events > 1:
        reasons.append(f"validation missed events {missed_events} > 1")
    if not reasons:
        reasons.append("Held-out precision/recall/FPR and event-miss checks are within the configured generation gate.")
    return {
        "justified": justified,
        "basis": "Held-out validation metrics only; training-set agreement is not used as the advancement criterion.",
        "criteria": {
            "min_recall": float(min_recall),
            "min_precision": float(min_precision),
            "max_fpr": float(max_fpr),
            "max_validation_event_misses": 1,
        },
        "held_out_snapshot": {
            "validation_recall": validation_recall,
            "validation_precision": validation_precision,
            "validation_fpr": validation_fpr,
            "validation_missed_event_count": missed_events,
            "validation_false_alarms_per_session": false_alarms,
        },
        "reason": "; ".join(reasons),
    }


def pick_group_column(df: pd.DataFrame) -> str | None:
    for col in GROUP_COL_CANDIDATES:
        if col in df.columns:
            return col
    return None


def normalize_feature_names(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()
    df.columns = [str(c).strip() for c in df.columns]

    aliases = {
        "i_rms": "i_rms",
        "v_rms": "v_rms",
        "temp_c": "temp_c",
        "label_arc": "label_arc",
        "trial_id": "trial_id",
        "section_id": "section_id",
        "session_id": "session_id",
        "epoch_ms": "epoch_ms",
        "adc_fs_hz": "adc_fs_hz",
        "feat_valid": "feat_valid",
        "current_valid": "current_valid",
        "rf_train_row": "rf_train_row",
        "sample_weight": "sample_weight",
        "spectral_flux_midhf": "spectral_flux_midhf",
        "spectral_flux": "spectral_flux_midhf",
        "midhf_flux": "spectral_flux_midhf",
        "residual_crest_factor": "residual_crest_factor",
        "resid_crest_factor": "residual_crest_factor",
        "edge_spike_ratio": "edge_spike_ratio",
        "pre_dip_spike_ratio": "edge_spike_ratio",
        "midband_residual_ratio": "midband_residual_ratio",
        "cycle_nmse": "cycle_nmse",
        "peak_fluct_cv": "peak_fluct_cv",
        "thd_i": "thd_i",
        "hf_energy_delta": "hf_energy_delta",
        "zcv": "zcv",
        "abs_irms_zscore_vs_baseline": "abs_irms_zscore_vs_baseline",
        "frame_start_uptime_ms": "frame_start_uptime_ms",
        "frame_end_uptime_ms": "frame_end_uptime_ms",
        "frame_dt_ms": "frame_dt_ms",
        "compute_time_ms": "compute_time_ms",
        "queue_drop_count": "queue_drop_count",
        "fs_err_hz": "fs_err_hz",
        "sampling_quality_bad": "sampling_quality_bad",
        "invalid_loaded_flag": "invalid_loaded_flag",
        "invalid_off_flag": "invalid_off_flag",
        "relay_blank_active": "relay_blank_active",
        "turnon_blank_active": "turnon_blank_active",
        "transient_blank_active": "transient_blank_active",
        "suspicious_run_len": "suspicious_run_len",
        "suspicious_run_energy": "suspicious_run_energy",
        "invalid_loaded_run_len": "invalid_loaded_run_len",
        "delta_irms_abs": "delta_irms_abs",
        "delta_hf_energy": "delta_hf_energy",
        "delta_flux": "delta_flux",
        "v_sag_pct": "v_sag_pct",
        "restrike_count_short": "restrike_count_short",
        "halfcycle_asymmetry": "halfcycle_asymmetry",
        "rolling_baseline_irms": "rolling_baseline_irms",
        "rolling_baseline_irms_mean": "rolling_baseline_irms_mean",
        "rolling_baseline_irms_std": "rolling_baseline_irms_std",
        "rolling_baseline_hf_band_energy_ratio": "rolling_baseline_hf_band_energy_ratio",
        "hf_band_energy_ratio": "hf_band_energy_ratio",
        "midband_residual_rms": "midband_residual_rms",
        "irms_drop_vs_baseline": "irms_drop_vs_baseline",
        "pulse_count_per_cycle": "pulse_count_per_cycle",
        **DB_ALIAS_MAP,
    }

    rename_map = {
        c: aliases[str(c).strip()]
        for c in df.columns
        if str(c).strip() in aliases
    }
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

    fam_series = df.get("device_family", pd.Series("unknown", index=df.index)).astype(str).map(_normalize_device_family_name)
    fam_series = pd.Series(fam_series, index=df.index, dtype=object)
    if "device_family_code" in df.columns:
        fam_code = pd.to_numeric(df["device_family_code"], errors="coerce").apply(_coerce_device_family_code).astype(int)
        fam_from_code = fam_code.map(lambda c: DEVICE_FAMILY_NAME_FROM_CODE.get(int(c), "unknown"))
        known_from_text = fam_series.map(lambda name: DEVICE_FAMILY_CODE_MAP.get(str(name), DEVICE_FAMILY_UNKNOWN_CODE)).astype(int)
        repair_mask = (fam_code == DEVICE_FAMILY_UNKNOWN_CODE) & (known_from_text >= 0)
        if repair_mask.any():
            fam_code.loc[repair_mask] = known_from_text.loc[repair_mask]
        fam_from_code = fam_code.map(lambda c: DEVICE_FAMILY_NAME_FROM_CODE.get(int(c), "unknown"))
        fam_series = fam_series.where(fam_series != "unknown", fam_from_code)
        df["device_family_code"] = fam_code
    else:
        df["device_family_code"] = fam_series.map(lambda name: DEVICE_FAMILY_CODE_MAP.get(str(name), DEVICE_FAMILY_UNKNOWN_CODE)).astype(int)

    df["device_family"] = pd.Series(fam_series, index=df.index).astype(str)
    if "device_name" in df.columns:
        df["device_name"] = df["device_name"].astype(str).fillna("unknown_device")
    else:
        df["device_name"] = "unknown_device"

    if "context_family_code_runtime" in df.columns:
        runtime_codes = pd.to_numeric(df["context_family_code_runtime"], errors="coerce").apply(_coerce_device_family_code).astype(int)
    else:
        runtime_codes = df["device_family_code"].astype(int)
    df["context_family_code_runtime"] = runtime_codes

    for fam in DEVICE_FAMILY_CLASSES:
        fam_idx = DEVICE_FAMILY_CODE_MAP[fam]
        col = f"ctx_family_{fam}"
        if col not in df.columns:
            df[col] = (runtime_codes == fam_idx).astype(float)
        else:
            df[col] = pd.to_numeric(df[col], errors="coerce").fillna(0.0)
    if "context_family_confidence" not in df.columns:
        df["context_family_confidence"] = np.where(runtime_codes >= 0, 1.0, 0.0)
    else:
        df["context_family_confidence"] = pd.to_numeric(df["context_family_confidence"], errors="coerce").fillna(0.0).clip(0.0, 1.0)

    return df


def clean_df(df: pd.DataFrame, include_invalid: bool = False, feature_names=None, target_col: str = TARGET) -> pd.DataFrame:
    df = normalize_feature_names(df.copy())
    feature_names = list(feature_names or default_feature_names_for_target(target_col))

    missing = [c for c in feature_names + [target_col] if c not in df.columns]
    if missing:
        raise ValueError(f"Missing required columns: {missing}")

    df[target_col] = pd.to_numeric(df[target_col], errors="coerce")
    if target_col == TARGET:
        df = df[df[target_col].isin([0, 1])].copy()
    else:
        df = df[df[target_col].notna()].copy()

    for c in feature_names:
        df[c] = pd.to_numeric(df[c], errors="coerce")

    if include_invalid:
        x = df[feature_names].replace([np.inf, -np.inf], np.nan)
        df = df.loc[x.notna().all(axis=1)].copy()
    else:
        if "rf_train_row" in df.columns and target_col == TARGET:
            df = df[df["rf_train_row"] == 1].copy()
        elif "feat_valid" in df.columns and target_col == TARGET:
            curv = pd.to_numeric(_series_from_default(df.index, df.get("current_valid", None), default=1), errors="coerce").fillna(1)
            df = df[(pd.to_numeric(df["feat_valid"], errors="coerce") == 1) & (curv == 1)].copy()

        x = df[feature_names].replace([np.inf, -np.inf], np.nan)
        df = df.loc[x.notna().all(axis=1)].copy()

    for c, (lo, hi) in FEATURE_CLIP_BOUNDS.items():
        if c in df.columns:
            df[c] = df[c].clip(lo, hi)

    if "sample_weight" in df.columns:
        df["sample_weight"] = pd.to_numeric(df["sample_weight"], errors="coerce").fillna(1.0)
        df["sample_weight"] = df["sample_weight"].clip(0.05, 100.0)
    else:
        df["sample_weight"] = 1.0

    if target_col == TARGET:
        df[target_col] = df[target_col].astype(int)
    return df


def select_threshold_cost(
    y_true: np.ndarray,
    y_proba: np.ndarray,
    fn_weight: float = 80.0,
    fp_weight: float = 4.0,
    min_recall: float = 0.97,
    min_precision: float = 0.0,
    max_fpr: float = 1.0,
    min_threshold: float = 0.05,
    meta_df: pd.DataFrame | None = None,
    arc_tolerance: ArcToleranceConfig | dict | None = None,
) -> dict:
    start_thr = max(0.01, float(min_threshold))
    thresholds = np.arange(start_thr, 0.96, 0.01)
    best = None

    y_true = np.asarray(y_true).astype(int)
    y_proba = np.asarray(y_proba).astype(float)
    tolerance_cfg = normalize_arc_tolerance_config(arc_tolerance)

    def _pack(thr, tn, fp, fn, tp, selection_reason):
        recall = tp / max(1, tp + fn)
        precision = tp / max(1, tp + fp)
        specificity = tn / max(1, tn + fp)
        fpr = fp / max(1, fp + tn)
        threshold_ok = float(thr) >= float(min_threshold)
        recall_ok = recall >= float(min_recall)
        precision_ok = precision >= float(min_precision)
        fpr_ok = fpr <= float(max_fpr)
        row = {
            "thr": float(thr),
            "cost": float(fn_weight * fn + fp_weight * fp),
            "tn": int(tn),
            "fp": int(fp),
            "fn": int(fn),
            "tp": int(tp),
            "recall": float(recall),
            "precision": float(precision),
            "specificity": float(specificity),
            "fpr": float(fpr),
            "threshold_ok": bool(threshold_ok),
            "recall_ok": bool(recall_ok),
            "precision_ok": bool(precision_ok),
            "fpr_ok": bool(fpr_ok),
            "constraints_met": bool(threshold_ok and recall_ok and precision_ok and fpr_ok),
            "selection_reason": selection_reason,
        }
        if meta_df is not None and len(meta_df) == len(y_true):
            event_report = build_event_level_report(
                meta_df,
                y_true,
                y_proba,
                float(thr),
                tolerance=tolerance_cfg,
            ) or {}
            row["event_true_count"] = int(event_report.get("true_event_count", 0))
            row["event_tp"] = int(event_report.get("detected_event_count", 0))
            row["event_fn"] = int(event_report.get("missed_event_count", 0))
            row["event_recall"] = float(event_report.get("event_recall", 0.0))
            row["event_precision"] = float(event_report.get("event_precision", 0.0))
            row["false_alarm_events"] = int(event_report.get("false_alarm_event_count", 0))
            row["false_alarm_sessions"] = int(event_report.get("false_alarm_session_count", 0))
            row["false_alarms_per_session"] = float(event_report.get("false_alarms_per_session", 0.0))
        else:
            row["event_true_count"] = 0
            row["event_tp"] = 0
            row["event_fn"] = 0
            row["event_recall"] = 0.0
            row["event_precision"] = 0.0
            row["false_alarm_events"] = 0
            row["false_alarm_sessions"] = 0
            row["false_alarms_per_session"] = 0.0
        return row

    feasible = []
    fallback = []
    for thr in thresholds:
        y_pred = (y_proba >= thr).astype(int)
        tn, fp, fn, tp = confusion_matrix(
            y_true,
            y_pred,
            labels=[0, 1],
        ).ravel()
        row = _pack(thr, tn, fp, fn, tp, "constraint_satisfying_threshold")
        if row["constraints_met"]:
            feasible.append(row)
        miss_recall = max(0.0, float(min_recall) - row["recall"])
        miss_precision = max(0.0, float(min_precision) - row["precision"])
        over_fpr = max(0.0, row["fpr"] - float(max_fpr))
        event_fn_penalty = max(0.0, float(row.get("event_fn", 0)))
        false_alarm_penalty = max(0.0, float(row.get("false_alarms_per_session", 0.0)))
        penalty = (
            row["cost"]
            + (event_fn_penalty * max(1500.0, float(fn_weight) * 35.0))
            + (false_alarm_penalty * max(120.0, float(fp_weight) * 80.0))
            + (miss_recall * max(1.0, float(fn_weight)) * 800.0)
            + (miss_precision * max(1.0, max(float(fp_weight) * 12.0, float(fn_weight) * 0.45)) * 1200.0)
            + (over_fpr * max(1.0, max(float(fp_weight) * 18.0, float(fn_weight) * 0.35)) * 1400.0)
        )
        row["fallback_penalty"] = float(penalty)
        fallback.append(row)

    if feasible:
        best = sorted(
            feasible,
            key=lambda r: (
                r.get("event_fn", 10**9),
                r["cost"],
                r.get("false_alarms_per_session", 10**9),
                r["fpr"],
                r["fp"],
                r["fn"],
                -r.get("event_precision", 0.0),
                -r["precision"],
                -r["recall"],
                -r["thr"],
            ),
        )[0]
        best["selection_reason"] = "constraint_satisfying_threshold"
        return best

    if fallback:
        best = sorted(
            fallback,
            key=lambda r: (
                r.get("event_fn", 10**9),
                r["fallback_penalty"],
                r.get("false_alarms_per_session", 10**9),
                r["fpr"],
                r["fp"],
                r["fn"],
                -r.get("event_precision", 0.0),
                -r["precision"],
                -r["recall"],
                -r["thr"],
            ),
        )[0]
        best["selection_reason"] = "best_fallback_under_constraints"
        return best

    return {
        "thr": max(0.5, float(min_threshold)),
        "cost": float("inf"),
        "tn": 0,
        "fp": 0,
        "fn": 0,
        "tp": 0,
        "recall": 0.0,
        "precision": 0.0,
        "specificity": 0.0,
        "fpr": 1.0,
        "threshold_ok": True,
        "recall_ok": False,
        "precision_ok": False,
        "fpr_ok": False,
        "constraints_met": False,
        "selection_reason": "no_thresholds_available",
        "event_true_count": 0,
        "event_tp": 0,
        "event_fn": 0,
        "event_recall": 0.0,
        "event_precision": 0.0,
        "false_alarm_events": 0,
        "false_alarm_sessions": 0,
        "false_alarms_per_session": 0.0,
    }


def model_size_estimate(model) -> int:
    estimators = getattr(model, "estimators_", None)
    if estimators is None:
        return 0

    flat = []
    if isinstance(estimators, np.ndarray):
        flat = list(estimators.ravel())
    elif isinstance(estimators, (list, tuple)):
        for item in estimators:
            if isinstance(item, (list, tuple, np.ndarray)):
                flat.extend(list(np.asarray(item, dtype=object).ravel()))
            else:
                flat.append(item)
    else:
        flat = [estimators]

    total = 0
    for est in flat:
        tree = getattr(est, "tree_", None)
        if tree is not None and hasattr(tree, "node_count"):
            total += int(tree.node_count)
    return int(total)




def _safe_float(value, default: float = 0.0) -> float:
    try:
        out = float(value)
        if np.isnan(out) or np.isinf(out):
            return float(default)
        return out
    except Exception:
        return float(default)


def _normalize_device_family_name(value, default: str = "unknown") -> str:
    raw = str(value or "").strip().lower()
    raw = raw.replace("-", "_").replace(" ", "_")
    alias = {
        "": default,
        "unknown": "unknown",
        "resistive": "resistive_linear",
        "resistive_linear": "resistive_linear",
        "heater": "resistive_linear",
        "heating": "resistive_linear",
        "inductive": "inductive_motor",
        "motor": "inductive_motor",
        "fan": "inductive_motor",
        "inductive_motor": "inductive_motor",
        "smps": "rectifier_smps",
        "rectifier": "rectifier_smps",
        "rectifier_smps": "rectifier_smps",
        "charger": "rectifier_smps",
        "adapter": "rectifier_smps",
        "phase": "phase_angle_controlled",
        "dimmer": "phase_angle_controlled",
        "dimmer_phase": "phase_angle_controlled",
        "phase_angle": "phase_angle_controlled",
        "phase_angle_controlled": "phase_angle_controlled",
        "universal": "brush_universal_motor",
        "universal_motor": "brush_universal_motor",
        "brush": "brush_universal_motor",
        "brush_universal_motor": "brush_universal_motor",
        "vacuum": "brush_universal_motor",
        "mixed": "other_mixed",
        "mixed_unknown": "other_mixed",
        "other": "other_mixed",
        "other_mixed": "other_mixed",
    }
    out = alias.get(raw, raw)
    if out in DEVICE_FAMILY_CODE_MAP:
        return out
    return default


def _coerce_device_family_code(value) -> int:
    try:
        if value is None or (isinstance(value, float) and np.isnan(value)):
            return DEVICE_FAMILY_UNKNOWN_CODE
        iv = int(value)
        if iv in DEVICE_FAMILY_NAME_FROM_CODE or iv == DEVICE_FAMILY_UNKNOWN_CODE:
            return iv
    except Exception:
        pass
    return DEVICE_FAMILY_UNKNOWN_CODE


def c_float_literal(value: float, digits: int = 9) -> str:
    fv = _safe_float(value, 0.0)
    s = f"{fv:.{digits}g}"
    if "e" in s or "E" in s:
        mantissa, exponent = re.split(r"[eE]", s, maxsplit=1)
        if "." not in mantissa:
            mantissa += ".0"
        return f"{mantissa}e{exponent}f"
    if "." not in s:
        s += ".0"
    return s + "f"



def _safe_rate(num: float, den: float) -> float:
    return float(num) / max(1.0, float(den))



def _safe_average_precision(y_true, y_score) -> float:
    y_true = np.asarray(y_true).astype(int)
    y_score = np.asarray(y_score).astype(float)
    if y_true.size == 0 or np.unique(y_true).size < 2:
        return 0.0
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        try:
            out = float(average_precision_score(y_true, y_score))
            if np.isnan(out) or np.isinf(out):
                return 0.0
            return out
        except Exception:
            return 0.0


def _safe_roc_auc(y_true, y_score) -> float:
    y_true = np.asarray(y_true).astype(int)
    y_score = np.asarray(y_score).astype(float)
    if y_true.size == 0 or np.unique(y_true).size < 2:
        return 0.5
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        try:
            out = float(roc_auc_score(y_true, y_score))
            if np.isnan(out) or np.isinf(out):
                return 0.5
            return out
        except Exception:
            return 0.5


def summarize_scores(y_score: np.ndarray) -> dict:
    y_score = np.asarray(y_score).astype(float)
    if y_score.size == 0:
        return {
            "min": 0.0,
            "max": 0.0,
            "mean": 0.0,
            "median": 0.0,
            "std": 0.0,
        }
    return {
        "min": float(np.min(y_score)),
        "max": float(np.max(y_score)),
        "mean": float(np.mean(y_score)),
        "median": float(np.median(y_score)),
        "std": float(np.std(y_score)),
    }




def summarize_label_scores(y_true, y_score) -> dict:
    y_true = np.asarray(y_true).astype(int)
    y_score = np.asarray(y_score).astype(float)
    pos = y_score[y_true == 1]
    neg = y_score[y_true == 0]
    return {
        "mean_score": float(np.mean(y_score)) if y_score.size else 0.0,
        "median_score": float(np.median(y_score)) if y_score.size else 0.0,
        "min_score": float(np.min(y_score)) if y_score.size else 0.0,
        "max_score": float(np.max(y_score)) if y_score.size else 0.0,
        "std_score": float(np.std(y_score)) if y_score.size else 0.0,
        "mean_score_pos": float(np.mean(pos)) if pos.size else 0.0,
        "mean_score_neg": float(np.mean(neg)) if neg.size else 0.0,
    }

def evaluate_binary_scores(y_true, y_score, threshold: float) -> dict:
    y_true = np.asarray(y_true).astype(int)
    y_score = np.asarray(y_score).astype(float)
    y_pred = (y_score >= float(threshold)).astype(int)

    tn, fp, fn, tp = confusion_matrix(
        y_true,
        y_pred,
        labels=[0, 1],
    ).ravel()

    try:
        mcc = float(matthews_corrcoef(y_true, y_pred))
        if np.isnan(mcc) or np.isinf(mcc):
            mcc = 0.0
    except Exception:
        mcc = 0.0

    accuracy = _safe_rate(tp + tn, tp + tn + fp + fn)
    recall = _safe_rate(tp, tp + fn)
    specificity = _safe_rate(tn, tn + fp)
    balanced_accuracy = float((recall + specificity) * 0.5)

    out = {
        "threshold": float(threshold),
        "accuracy": float(accuracy),
        "balanced_accuracy": float(balanced_accuracy),
        "average_precision": _safe_average_precision(y_true, y_score),
        "roc_auc": _safe_roc_auc(y_true, y_score),
        "precision": float(precision_score(y_true, y_pred, zero_division=0)),
        "recall": float(recall),
        "f1": float(f1_score(y_true, y_pred, zero_division=0)),
        "specificity": float(specificity),
        "npv": _safe_rate(tn, tn + fn),
        "fpr": _safe_rate(fp, fp + tn),
        "fnr": _safe_rate(fn, fn + tp),
        "mcc": mcc,
        "confusion_matrix": {
            "tn": int(tn),
            "fp": int(fp),
            "fn": int(fn),
            "tp": int(tp),
        },
        "classification_report": classification_report(
            y_true,
            y_pred,
            digits=4,
            labels=[0, 1],
            output_dict=True,
            zero_division=0,
        ),
        "positive_count": int(np.sum(y_true == 1)),
        "negative_count": int(np.sum(y_true == 0)),
        "score_summary": summarize_scores(y_score),
        "label_score_summary": summarize_label_scores(y_true, y_score),
        "class_balance_ok": bool(np.unique(y_true).size >= 2),
    }
    return out


def _selection_policy_meta(
    winner_mode: str,
    fn_weight: float,
    fp_weight: float,
) -> dict:
    if winner_mode == "legacy_cv_ap":
        return {
            "mode": winner_mode,
            "description": (
                "Legacy rule: higher CV average precision, then fewer validation FP, "
                "then fewer validation FN, then smaller node count."
            ),
            "uses_test_metrics_for_selection": False,
            "tie_break_order": [
                "cv_best_average_precision",
                "validation_fp",
                "validation_fn",
                "estimated_node_count",
            ],
        }

    if winner_mode == "arc_guard":
        return {
            "mode": winner_mode,
            "description": (
                "Arc-first winner rule: prefer models that satisfy threshold constraints, "
                "then minimize validation event misses, then minimize row false negatives, "
                "then minimize false positives and false alarms."
            ),
            "uses_test_metrics_for_selection": False,
            "tie_break_order": [
                "threshold_constraints_met",
                "validation_event_fn",
                "validation_fn",
                "validation_fp",
                "validation_false_alarms_per_session",
                "validation_recall",
                "validation_precision",
                "cv_best_average_precision",
                "estimated_node_count",
            ],
        }

    return {
        "mode": winner_mode,
        "description": (
            "Composite winner score using event-aware validation first, then CV AP plus "
            "validation-set precision, recall, balanced accuracy, specificity, ROC AUC, "
            "MCC, validation threshold cost penalty, false-alarm penalty, and model size penalty. "
            "Test metrics are intentionally excluded from winner selection."
        ),
        "uses_test_metrics_for_selection": False,
        "weights": {
            "validation_event_recall": 0.24,
            "validation_event_fn_penalty": -0.22,
            "cv_best_average_precision": 0.18,
            "validation_average_precision": 0.14,
            "validation_recall": 0.16,
            "validation_balanced_accuracy": 0.15,
            "validation_f1": 0.08,
            "validation_specificity": 0.08,
            "validation_roc_auc": 0.04,
            "validation_mcc_norm": 0.03,
            "validation_cost_penalty": -0.03,
            "validation_false_alarm_penalty": -0.09,
            "node_penalty": -0.01,
        },
        "cost_context": {
            "fn_weight": float(fn_weight),
            "fp_weight": float(fp_weight),
            "cost_unit": float(max(1.0, fn_weight + (5.0 * fp_weight))),
        },
        "tie_break_order": [
            "winner_score",
            "validation_event_recall",
            "validation_event_fn",
            "validation_recall",
            "validation_balanced_accuracy",
            "validation_average_precision",
            "cv_best_average_precision",
            "validation_false_alarms_per_session",
            "validation_specificity",
            "estimated_node_count",
        ],
    }



def _build_selection_meta(
    result: dict,
    winner_mode: str,
    fn_weight: float,
    fp_weight: float,
) -> dict:
    val = result.get("validation_metrics", {})
    val_event = result.get("validation_event_level_metrics", {}) or {}

    if winner_mode == "legacy_cv_ap":
        score = _safe_float(result.get("cv_best_average_precision"))
        reason = (
            "Legacy winner rule selected this model by CV average precision first, "
            "with validation FP/FN and node count only as tie-breakers."
        )
        return {
            "winner_score": float(score),
            "winner_score_components": {
                "cv_best_average_precision": float(score),
            },
            "winner_reason": reason,
        }

    cv_ap = _safe_float(result.get("cv_best_average_precision"))
    val_ap = _safe_float(val.get("average_precision"))
    val_recall = _safe_float(val.get("recall"))
    val_precision = _safe_float(val.get("precision"))
    val_bal_acc = _safe_float(val.get("balanced_accuracy"))
    val_f1 = _safe_float(val.get("f1"))
    val_spec = _safe_float(val.get("specificity"))
    val_roc_auc = _safe_float(val.get("roc_auc"))
    val_mcc_norm = (_safe_float(val.get("mcc")) + 1.0) / 2.0
    val_cost = _safe_float(result.get("validation_threshold_result", {}).get("cost"))
    val_fn = _safe_float(result.get("validation_threshold_result", {}).get("fn"), 1e18)
    val_fp = _safe_float(result.get("validation_threshold_result", {}).get("fp"), 1e18)
    val_event_fn = _safe_float(val_event.get("missed_event_count"), 1e18)
    val_event_recall = _safe_float(val_event.get("event_recall"))
    val_false_alarm_rate = _safe_float(val_event.get("false_alarms_per_session"))
    node_count = _safe_float(result.get("estimated_node_count"))
    constraints_met = bool(result.get("threshold_constraints_met", False))

    if winner_mode == "arc_guard":
        components = {
            "constraints_met": 4.0 * float(1.0 if constraints_met else 0.0),
            "zero_validation_event_fn": 3.5 * float(1.0 if val_event_fn == 0 else 0.0),
            "zero_validation_fn": 3.0 * float(1.0 if val_fn == 0 else 0.0),
            "zero_validation_fp": 1.0 * float(1.0 if val_fp == 0 else 0.0),
            "validation_event_recall": 2.8 * val_event_recall,
            "validation_recall": 2.5 * val_recall,
            "validation_precision": 1.5 * val_precision,
            "validation_specificity": 1.0 * val_spec,
            "cv_best_average_precision": 0.5 * cv_ap,
            "validation_event_fn_penalty": -0.65 * min(12.0, val_event_fn),
            "validation_fn_penalty": -0.35 * min(20.0, val_fn),
            "validation_fp_penalty": -0.05 * min(50.0, val_fp),
            "validation_false_alarm_penalty": -0.10 * min(20.0, val_false_alarm_rate),
            "node_penalty": -0.01 * min(1.0, node_count / 20000.0),
        }
        score = float(sum(components.values()))
        reason = (
            "Arc-first winner rule selected this model by prioritizing threshold-constraint "
            "compliance, zero or low validation false negatives, then low validation false positives."
        )
        return {
            "winner_score": score,
            "winner_score_components": components,
            "winner_reason": reason,
        }

    cost_unit = max(1.0, fn_weight + (5.0 * fp_weight))
    val_cost_penalty = float(np.tanh(val_cost / cost_unit))
    node_penalty = min(1.0, node_count / 20000.0)

    components = {
        "validation_event_recall": 0.24 * val_event_recall,
        "validation_event_fn_penalty": -0.22 * min(1.0, val_event_fn / 6.0),
        "cv_best_average_precision": 0.18 * cv_ap,
        "validation_average_precision": 0.14 * val_ap,
        "validation_recall": 0.16 * val_recall,
        "validation_balanced_accuracy": 0.15 * val_bal_acc,
        "validation_f1": 0.08 * val_f1,
        "validation_specificity": 0.08 * val_spec,
        "validation_roc_auc": 0.04 * val_roc_auc,
        "validation_mcc_norm": 0.03 * val_mcc_norm,
        "validation_cost_penalty": -0.03 * val_cost_penalty,
        "validation_false_alarm_penalty": -0.09 * min(1.0, val_false_alarm_rate),
        "node_penalty": -0.01 * node_penalty,
    }
    score = float(sum(components.values()))
    reason = (
        "Composite winner score selected this model using CV AP and validation metrics "
        "together. Test metrics were not used for model selection to avoid test leakage."
    )
    return {
        "winner_score": score,
        "winner_score_components": components,
        "winner_reason": reason,
    }


def postprocess_m2c_header(c_code: str) -> str:
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
        r'\1static inline void set_output2(double *dst, double a, double b) {\n'
        r'    dst[0] = a;\n'
        r'    dst[1] = b;\n'
        r'}\n'
        r'static inline void arc_rf_predict(double * input, double * output) {',
        c_code,
        flags=re.MULTILINE,
    )

    memcpy_pat = re.compile(
        r'(?m)^(?P<indent>\s*)memcpy\(\s*(?P<dst>[A-Za-z_]\w*)\s*,\s*\(\s*double\s*\[\s*\]\s*\)\s*\{\s*(?P<a>[^,{}]+?)\s*,\s*(?P<b>[^{}]+?)\s*\}\s*,\s*2\s*\*\s*sizeof\s*\(\s*double\s*\)\s*\)\s*;\s*$'
    )

    def repl(match):
        return "%sset_output2(%s, %s, %s);" % (
            match.group("indent"),
            match.group("dst").strip(),
            match.group("a").strip(),
            match.group("b").strip(),
        )

    c_code = memcpy_pat.sub(repl, c_code)
    c_code = re.sub(r"(?<![\w.])(-?\d+)f\b", lambda m: f"{m.group(1)}.0f", c_code)
    return c_code


def _arc_time_sort_column(df: pd.DataFrame) -> str | None:
    for col in ("frame_start_uptime_ms", "frame_end_uptime_ms", "uptime_ms", "epoch_ms"):
        if col in df.columns:
            return col
    return None


def apply_arc_tolerance_policy(
    df: pd.DataFrame,
    *,
    target_col: str = TARGET,
    tolerance: ArcToleranceConfig | dict | None = None,
) -> pd.DataFrame:
    work = df.copy()
    cfg = normalize_arc_tolerance_config(tolerance)

    if target_col != TARGET:
        return work

    raw_target = pd.to_numeric(work.get(target_col, pd.Series(0, index=work.index)), errors="coerce").fillna(0).astype(int)
    work["label_arc_raw"] = raw_target.astype(int)
    work["label_arc_exact"] = raw_target.astype(int)
    work["label_arc_tolerant"] = raw_target.astype(int)
    work["arc_tolerance_positive"] = raw_target.astype(int)
    work["arc_tolerance_center"] = raw_target.astype(int)
    work["arc_neighbor_distance"] = np.where(raw_target.to_numpy(dtype=int) == 1, 0, 9999).astype(int)
    work["arc_event_window_positive"] = raw_target.astype(int)
    work["arc_event_id"] = -1
    work["near_arc_hard_negative"] = 0
    work["arc_tolerance_mode"] = cfg.normalized_mode()

    mode = cfg.normalized_mode()
    if mode == "none" or (cfg.pre_rows <= 0 and cfg.post_rows <= 0):
        return work

    group_col = pick_group_column(work)
    if group_col is None:
        work["_group"] = np.arange(len(work)) // 200
        group_col = "_group"

    time_col = _arc_time_sort_column(work)
    sort_cols = [group_col] + ([time_col] if time_col is not None else [])
    work = work.sort_values(sort_cols, kind="stable").copy()

    positive_weight_ref = float(
        pd.to_numeric(work.loc[work["label_arc_raw"] == 1, "sample_weight"], errors="coerce").fillna(1.0).median()
        if "sample_weight" in work.columns and (work["label_arc_raw"] == 1).any()
        else 1.0
    )
    if not np.isfinite(positive_weight_ref) or positive_weight_ref <= 0.0:
        positive_weight_ref = 1.0

    event_serial = 0
    for _, group in work.groupby(group_col, sort=False):
        idxs = group.index.to_numpy()
        labels = work.loc[idxs, "label_arc_raw"].to_numpy(dtype=int)
        starts = []
        ends = []
        run_start = None
        for i, lab in enumerate(labels):
            if lab == 1 and run_start is None:
                run_start = i
            end_run = (lab == 0 and run_start is not None)
            final_row = (i == len(labels) - 1 and run_start is not None and lab == 1)
            if end_run or final_row:
                run_end = i if final_row else i - 1
                starts.append(int(run_start))
                ends.append(int(run_end))
                run_start = None
        for run_start, run_end in zip(starts, ends):
            event_serial += 1
            win_lo = max(0, int(run_start) - int(cfg.pre_rows))
            win_hi = min(len(idxs) - 1, int(run_end) + int(cfg.post_rows))
            ring_lo = max(0, int(win_lo) - int(cfg.hard_negative_ring))
            ring_hi = min(len(idxs) - 1, int(win_hi) + int(cfg.hard_negative_ring))

            window_rows = idxs[win_lo:win_hi + 1]
            center_rows = idxs[run_start:run_end + 1]
            ring_rows = idxs[ring_lo:ring_hi + 1]

            work.loc[window_rows, "arc_event_window_positive"] = 1
            work.loc[window_rows, "arc_tolerance_positive"] = 1
            work.loc[center_rows, "arc_tolerance_center"] = 1
            work.loc[window_rows, "arc_event_id"] = np.maximum(
                pd.to_numeric(work.loc[window_rows, "arc_event_id"], errors="coerce").fillna(-1).astype(int),
                int(event_serial),
            )

            for rel_i, abs_idx in enumerate(window_rows, start=win_lo):
                if work.at[abs_idx, "label_arc_raw"] == 1:
                    dist = 0
                elif rel_i < run_start:
                    dist = int(run_start - rel_i)
                elif rel_i > run_end:
                    dist = int(rel_i - run_end)
                else:
                    dist = 0
                work.at[abs_idx, "arc_neighbor_distance"] = min(int(work.at[abs_idx, "arc_neighbor_distance"]), int(dist))

            if mode == "expanded_positive":
                work.loc[window_rows, "label_arc_tolerant"] = 1
                neighbor_rows = [idx for idx in window_rows if idx not in set(center_rows.tolist())]
                if neighbor_rows and "sample_weight" in work.columns:
                    current = pd.to_numeric(work.loc[neighbor_rows, "sample_weight"], errors="coerce").fillna(0.0)
                    promoted = max(0.10, positive_weight_ref * float(cfg.expanded_neighbor_weight))
                    work.loc[neighbor_rows, "sample_weight"] = np.maximum(current, promoted)
            elif mode == "soft_positive":
                neighbor_mask = (work.loc[window_rows, "label_arc_raw"].astype(int) == 0)
                neighbor_rows = work.loc[window_rows].index[neighbor_mask].tolist()
                if neighbor_rows and "sample_weight" in work.columns:
                    current = pd.to_numeric(work.loc[neighbor_rows, "sample_weight"], errors="coerce").fillna(1.0)
                    work.loc[neighbor_rows, "sample_weight"] = np.maximum(
                        SCAFFOLD_MIN_ROW_WEIGHT,
                        current * float(cfg.soft_neighbor_weight),
                    )
                    if "label_trust" in work.columns:
                        trust = pd.to_numeric(work.loc[neighbor_rows, "label_trust"], errors="coerce").fillna(1.0)
                        work.loc[neighbor_rows, "label_trust"] = np.minimum(trust, float(cfg.soft_neighbor_weight))

            ring_only_rows = [idx for idx in ring_rows if idx not in set(window_rows.tolist())]
            if ring_only_rows:
                ring_only_rows = [idx for idx in ring_only_rows if int(work.at[idx, "label_arc_raw"]) == 0]
            if ring_only_rows:
                work.loc[ring_only_rows, "near_arc_hard_negative"] = 1
                if "hard_negative" in work.columns:
                    work.loc[ring_only_rows, "hard_negative"] = 1
                if "sample_weight" in work.columns:
                    current = pd.to_numeric(work.loc[ring_only_rows, "sample_weight"], errors="coerce").fillna(1.0)
                    work.loc[ring_only_rows, "sample_weight"] = np.maximum(current, current * 1.6)

    if mode == "expanded_positive":
        work[target_col] = pd.to_numeric(work["label_arc_tolerant"], errors="coerce").fillna(0).astype(int)

    if "clean_quality" in work.columns:
        soft_rows = (work["arc_tolerance_positive"].astype(int) == 1) & (work["label_arc_raw"].astype(int) == 0)
        work.loc[soft_rows & (work["clean_quality"].astype(str) == "trainable"), "clean_quality"] = (
            "expanded_positive_neighbor" if mode == "expanded_positive" else "soft_positive_neighbor"
        )
    return work.sort_index().copy()


def load_clean_dataset(
    csv_path: str,
    include_invalid: bool = False,
    feature_names=None,
    target_col: str = TARGET,
    augment_unknown_context: bool = True,
    arc_tolerance: ArcToleranceConfig | dict | None = None,
    current_training_generation: str | None = None,
    mismatch_fp_boost: float = 1.0,
    mismatch_fn_boost: float = 1.0,
    mismatch_focus_ratio: float = 0.0,
    mismatch_verified_only: bool = False,
):
    if not os.path.isfile(csv_path):
        raise ValueError(f"Merged dataset not found: {csv_path}")

    raw_df = pd.read_csv(csv_path)
    raw_df = normalize_feature_names(raw_df)
    raw_df = annotate_generation_and_mismatch_columns(raw_df, current_training_generation=current_training_generation)
    feature_names = list(feature_names or default_feature_names_for_target(target_col))
    df = clean_df(raw_df, include_invalid=include_invalid, feature_names=feature_names, target_col=target_col)
    df = apply_scaffold_gap_fill_cap(df, target_col=target_col)
    if target_col == TARGET:
        df = apply_arc_tolerance_policy(df, target_col=target_col, tolerance=arc_tolerance)
        df = annotate_generation_and_mismatch_columns(df, current_training_generation=current_training_generation)
        eligible_mismatch = pd.to_numeric(
            df.get("reality_predict_mismatch", pd.Series(0, index=df.index)),
            errors="coerce",
        ).fillna(0).astype(int) == 1
        if bool(mismatch_verified_only):
            eligible_mismatch &= _series_boolish_to_int(df.get("label_verified"), df.index, default=0) == 1
        mismatch_type = df.get("mismatch_type", pd.Series("", index=df.index)).astype(str)
        fp_multiplier = float(np.clip(float(mismatch_fp_boost), 0.10, 20.0))
        fn_multiplier = float(np.clip(float(mismatch_fn_boost), 0.10, 20.0))
        df["mismatch_weight_multiplier"] = 1.0
        df.loc[eligible_mismatch & mismatch_type.eq("fp_override"), "mismatch_weight_multiplier"] = fp_multiplier
        df.loc[eligible_mismatch & mismatch_type.eq("fn_override"), "mismatch_weight_multiplier"] = fn_multiplier
        if "sample_weight" in df.columns:
            df["sample_weight"] = (
                pd.to_numeric(df["sample_weight"], errors="coerce").fillna(1.0)
                * pd.to_numeric(df["mismatch_weight_multiplier"], errors="coerce").fillna(1.0)
            )
        df["mismatch_focus_ratio"] = float(np.clip(float(mismatch_focus_ratio), 0.0, 1.0))
        df["mismatch_verified_only"] = 1 if bool(mismatch_verified_only) else 0

    if augment_unknown_context and target_col == TARGET and any(str(name).startswith("ctx_family_") for name in feature_names):
        already_augmented = pd.Series(False, index=df.index)
        if "context_augmented" in df.columns:
            already_augmented = pd.to_numeric(df["context_augmented"], errors="coerce").fillna(0).astype(int) == 1
        base_aug = df.loc[~already_augmented].copy()
        if not base_aug.empty:
            df = pd.concat([df, augment_unknown_context_rows(base_aug, target_col=target_col)], ignore_index=True, sort=False)
            df = apply_scaffold_gap_fill_cap(df, target_col=target_col)

    if df.empty or df[target_col].nunique() < 2:
        raise ValueError(f"Training requires at least 2 classes in {target_col} from cleaned_data.csv.")

    group_col = pick_group_column(df)
    if group_col is None:
        df["_group"] = np.arange(len(df)) // 200
        group_col = "_group"

    groups = df[group_col].astype(str)
    df["_group_col_used"] = group_col
    X = df[feature_names].astype(float)
    y = df[target_col].astype(int)
    w = df["sample_weight"].astype(float).to_numpy()
    return df, X, y, groups, w


def _group_summary_frame(groups, y) -> pd.DataFrame:
    frame = pd.DataFrame({
        "group": pd.Series(groups).astype(str).to_numpy(),
        "label": pd.Series(y).astype(int).to_numpy(),
    })
    return frame.groupby("group", sort=False).agg(
        group_label=("label", "max"),
        row_count=("label", "size"),
        positive_rows=("label", "sum"),
    ).reset_index()


def _split_has_both_classes(summary: pd.DataFrame, selected_groups: set[str]) -> bool:
    if summary.empty or not selected_groups:
        return False
    labels = summary.loc[summary["group"].isin(selected_groups), "group_label"].astype(int)
    return labels.nunique() >= 2


def _class_balanced_group_kfold_splits(X, y, groups, n_splits: int):
    if StratifiedGroupKFold is not None:
        cv = StratifiedGroupKFold(n_splits=n_splits, shuffle=True, random_state=42)
        return list(cv.split(X, y, groups=groups))

    summary = _group_summary_frame(groups, y)
    pos = summary[summary["group_label"] == 1].sample(frac=1.0, random_state=42).reset_index(drop=True)
    neg = summary[summary["group_label"] == 0].sample(frac=1.0, random_state=43).reset_index(drop=True)
    folds = [{"groups": [], "rows": 0, "pos": 0, "neg": 0} for _ in range(n_splits)]

    def _place(group_row, is_pos: bool):
        target = min(
            range(n_splits),
            key=lambda i: (
                folds[i]["pos"] if is_pos else folds[i]["neg"],
                folds[i]["rows"],
                len(folds[i]["groups"]),
            ),
        )
        folds[target]["groups"].append(str(group_row["group"]))
        folds[target]["rows"] += int(group_row["row_count"])
        folds[target]["pos"] += int(is_pos)
        folds[target]["neg"] += int(not is_pos)

    for _, row in pos.iterrows():
        _place(row, True)
    for _, row in neg.iterrows():
        _place(row, False)

    all_groups = pd.Series(groups).astype(str).to_numpy()
    all_idx = np.arange(len(all_groups))
    out = []
    for fold in folds:
        val_groups = set(fold["groups"])
        val_mask = np.isin(all_groups, list(val_groups))
        val_idx = all_idx[val_mask]
        fit_idx = all_idx[~val_mask]
        if len(val_idx) and len(fit_idx):
            out.append((fit_idx, val_idx))
    return out


def _stratified_group_shuffle_indices(groups, y, test_size: float, random_state: int):
    summary = _group_summary_frame(groups, y)
    if len(summary) < 2:
        raise ValueError("Need at least 2 groups for grouped splitting.")

    pos_groups = int((summary["group_label"] == 1).sum())
    neg_groups = int((summary["group_label"] == 0).sum())
    y_arr = np.asarray(y).astype(int)
    groups_arr = pd.Series(groups).astype(str).to_numpy()
    all_idx = np.arange(len(groups_arr))

    n_group_classes = int(summary["group_label"].nunique())
    if pos_groups >= 2 and neg_groups >= 2 and len(summary) >= (2 * n_group_classes):
        group_names = summary["group"].astype(str).to_numpy()
        group_labels = summary["group_label"].astype(int).to_numpy()
        test_groups = int(round(len(summary) * float(test_size))) if isinstance(test_size, float) else int(test_size)
        test_groups = max(n_group_classes, test_groups)
        test_groups = min(test_groups, len(summary) - n_group_classes)
        if test_groups >= n_group_classes and (len(summary) - test_groups) >= n_group_classes:
            splitter = StratifiedShuffleSplit(
                n_splits=24,
                test_size=test_groups,
                random_state=random_state,
            )
            for train_gi, test_gi in splitter.split(group_names, group_labels):
                train_groups = set(group_names[train_gi].tolist())
                test_groups_set = set(group_names[test_gi].tolist())
                if _split_has_both_classes(summary, train_groups) and _split_has_both_classes(summary, test_groups_set):
                    train_mask = np.isin(groups_arr, list(train_groups))
                    test_mask = np.isin(groups_arr, list(test_groups_set))
                    return all_idx[train_mask], all_idx[test_mask]

    gss = GroupShuffleSplit(n_splits=24, test_size=test_size, random_state=random_state)
    best = None
    best_penalty = None
    for train_idx, test_idx in gss.split(X=np.zeros(len(groups_arr)), y=y_arr, groups=groups_arr):
        penalty = abs(np.unique(y_arr[train_idx]).size - 2) + abs(np.unique(y_arr[test_idx]).size - 2)
        if best is None or penalty < best_penalty:
            best = (train_idx, test_idx)
            best_penalty = penalty
            if penalty == 0:
                break
    if best is None:
        raise ValueError("Failed to build grouped train/test split.")
    return best


def make_group_splits(X, y, groups, test_size=0.20):
    train_full_idx, test_idx = _stratified_group_shuffle_indices(
        groups=groups,
        y=y,
        test_size=test_size,
        random_state=42,
    )

    X_train_full = X.iloc[train_full_idx]
    y_train_full = y.iloc[train_full_idx]
    groups_train_full = groups.iloc[train_full_idx]

    X_test = X.iloc[test_idx]
    y_test = y.iloc[test_idx]

    try:
        train_idx, val_idx = _stratified_group_shuffle_indices(
            groups=groups_train_full,
            y=y_train_full,
            test_size=0.20,
            random_state=123,
        )
    except Exception:
        train_idx = np.arange(len(X_train_full))
        val_idx = np.arange(len(X_train_full))

    train_summary = _group_summary_frame(groups_train_full, y_train_full)
    pos_group_count = int((train_summary["group_label"] == 1).sum())
    neg_group_count = int((train_summary["group_label"] == 0).sum())
    recommended_cv_splits = int(min(5, len(train_summary), pos_group_count, neg_group_count)) if pos_group_count > 0 and neg_group_count > 0 else 0

    return {
        "train_full_idx": train_full_idx,
        "test_idx": test_idx,
        "train_idx": train_idx,
        "val_idx": val_idx,
        "X_train_full": X_train_full,
        "y_train_full": y_train_full,
        "groups_train_full": groups_train_full,
        "X_test": X_test,
        "y_test": y_test,
        "cv_group_summary": {
            "train_group_count": int(len(train_summary)),
            "positive_group_count": pos_group_count,
            "negative_group_count": neg_group_count,
            "recommended_cv_splits": int(recommended_cv_splits),
            "stratified_group_cv": bool(StratifiedGroupKFold is not None and recommended_cv_splits >= 2),
            "grouping_note": "Grouping prefers trial_id over session_id so start/steady/arc sections from one trial stay together.",
        },
    }




def rebalance_family_class_weights(
    meta_df: pd.DataFrame | None,
    y,
    sample_weight,
    *,
    family_col: str = "device_family",
    max_scale: float = 3.0,
    min_scale: float = 0.33,
    include_unknown: bool = False,
) -> tuple[np.ndarray, dict]:
    weights = np.asarray(sample_weight, dtype=float).copy()
    info = {
        "enabled": False,
        "family_col": family_col,
        "class_targets": {},
        "family_class_weight_before": {},
        "family_class_weight_after": {},
        "family_class_scale": {},
    }
    if meta_df is None or not isinstance(meta_df, pd.DataFrame) or len(meta_df) != len(weights):
        return weights, info

    frame = meta_df.copy()
    labels = pd.Series(np.asarray(y).astype(int), index=frame.index)
    fam = frame.get(family_col, pd.Series("unknown", index=frame.index)).astype(str).map(_normalize_device_family_name)
    fam = pd.Series(fam, index=frame.index, dtype=object)
    known_mask = fam.ne("unknown") if not bool(include_unknown) else pd.Series(True, index=frame.index, dtype=bool)
    if int(known_mask.sum()) <= 0:
        return weights, info

    info["enabled"] = True
    weight_ser = pd.Series(weights, index=frame.index, dtype=float)
    scale_ser = pd.Series(1.0, index=frame.index, dtype=float)

    for label in (0, 1):
        mask = known_mask & labels.eq(int(label))
        if int(mask.sum()) <= 0:
            continue
        grouped = weight_ser.loc[mask].groupby(fam.loc[mask], sort=False).sum()
        grouped = grouped[grouped > 0.0]
        if len(grouped) <= 1:
            continue
        target = float(np.median(grouped.to_numpy(dtype=float)))
        info["class_targets"][str(int(label))] = target
        for family_name, total_weight in grouped.items():
            total_weight = float(total_weight)
            if total_weight <= 0.0:
                continue
            scale = float(target / total_weight)
            scale = float(np.clip(scale, float(min_scale), float(max_scale)))
            fam_mask = mask & fam.eq(str(family_name))
            scale_ser.loc[fam_mask] *= scale
            key = f"{family_name}|{int(label)}"
            info["family_class_weight_before"][key] = total_weight
            info["family_class_scale"][key] = scale

    balanced = np.clip(weight_ser.to_numpy(dtype=float) * scale_ser.to_numpy(dtype=float), 0.05, 100.0)
    balanced_ser = pd.Series(balanced, index=frame.index, dtype=float)
    for label in (0, 1):
        mask = known_mask & labels.eq(int(label))
        if int(mask.sum()) <= 0:
            continue
        grouped = balanced_ser.loc[mask].groupby(fam.loc[mask], sort=False).sum()
        for family_name, total_weight in grouped.items():
            key = f"{family_name}|{int(label)}"
            info["family_class_weight_after"][key] = float(total_weight)
    return balanced, info


def compose_arc_training_view(
    X,
    y,
    groups,
    w,
    *,
    meta_df: pd.DataFrame | None = None,
    negative_ratio: float = 0.0,
    positive_oversample: float = 1.0,
    random_state: int = 42,
    min_negative_rows: int = 256,
    mismatch_focus_ratio: float = 0.0,
    mismatch_verified_only: bool = False,
    family_balance: bool = True,
    family_balance_max_scale: float = 3.0,
    family_balance_min_scale: float = 0.33,
) -> tuple[pd.DataFrame, pd.Series, pd.Series, np.ndarray, pd.DataFrame | None, dict]:
    X_df = X.copy()
    y_ser = pd.Series(np.asarray(y).astype(int), index=X_df.index)
    g_ser = pd.Series(groups).astype(str).reset_index(drop=True)
    g_ser.index = X_df.index
    w_arr = np.asarray(w).astype(float)
    meta = meta_df.copy() if isinstance(meta_df, pd.DataFrame) else None

    pos_mask = y_ser.astype(int) == 1
    neg_mask = ~pos_mask
    pos_idx = X_df.index[pos_mask].to_numpy()
    neg_idx = X_df.index[neg_mask].to_numpy()

    info = {
        "positive_rows": int(pos_mask.sum()),
        "negative_rows": int(neg_mask.sum()),
        "negative_ratio_requested": float(negative_ratio),
        "positive_oversample": float(positive_oversample),
        "mismatch_focus_ratio": float(np.clip(float(mismatch_focus_ratio), 0.0, 1.0)),
        "hard_negative_kept": 0,
        "transition_negative_kept": 0,
        "near_arc_negative_kept": 0,
        "mismatch_fp_negatives_kept": 0,
        "mismatch_fn_positives": 0,
        "family_balance_enabled": bool(family_balance),
        "family_balance_max_scale": float(family_balance_max_scale),
        "family_balance_min_scale": float(family_balance_min_scale),
    }

    selected_neg_idx = neg_idx
    if float(negative_ratio) > 0.0 and len(pos_idx) > 0 and len(neg_idx) > 0:
        target_neg = int(max(int(min_negative_rows), round(len(pos_idx) * float(negative_ratio))))
        target_neg = min(target_neg, len(neg_idx))
        if target_neg < len(neg_idx):
            work_meta = meta if meta is not None else pd.DataFrame(index=X_df.index)
            arc_like = pd.to_numeric(work_meta.get("arc_like_score", pd.Series(0.0, index=X_df.index)), errors="coerce").fillna(0.0)
            hard_negative = pd.to_numeric(work_meta.get("hard_negative", pd.Series(0, index=X_df.index)), errors="coerce").fillna(0).astype(int)
            transition_normal = pd.to_numeric(work_meta.get("transition_normal", pd.Series(0, index=X_df.index)), errors="coerce").fillna(0).astype(int)
            near_arc_negative = pd.to_numeric(work_meta.get("near_arc_hard_negative", pd.Series(0, index=X_df.index)), errors="coerce").fillna(0).astype(int)
            trusted_normal = pd.to_numeric(work_meta.get("trusted_normal_session", pd.Series(0, index=X_df.index)), errors="coerce").fillna(0).astype(int)
            mismatch_type = work_meta.get("mismatch_type", pd.Series("", index=X_df.index)).astype(str)
            verified_mask = _series_boolish_to_int(work_meta.get("label_verified"), X_df.index, default=0) == 1
            mismatch_fp = mismatch_type.eq("fp_override")
            if bool(mismatch_verified_only):
                mismatch_fp &= verified_mask
            start_like = pd.Series(False, index=X_df.index, dtype=bool)
            if "division_tag" in work_meta.columns:
                start_like = work_meta["division_tag"].astype(str).str.lower().str.contains("start", na=False)

            hardness = (
                1.0
                + (2.8 * hard_negative.to_numpy(dtype=float))
                + (1.9 * transition_normal.to_numpy(dtype=float))
                + (1.6 * near_arc_negative.to_numpy(dtype=float))
                + np.clip(arc_like.to_numpy(dtype=float), 0.0, 3.0)
                + (0.9 * start_like.to_numpy(dtype=float))
                - (0.45 * trusted_normal.to_numpy(dtype=float))
                + (4.2 * mismatch_fp.to_numpy(dtype=float))
            )
            hardness = np.clip(hardness, 0.05, None)
            neg_positions = np.where(neg_mask.to_numpy(dtype=bool))[0]
            neg_probs = hardness[neg_positions] * np.clip(w_arr[neg_positions], 0.05, None)
            neg_probs = np.clip(neg_probs, 1e-6, None)
            neg_probs = neg_probs / neg_probs.sum()
            rng = np.random.default_rng(int(random_state))
            reserve_count = 0
            reserved_positions = np.asarray([], dtype=int)
            focus_ratio = float(np.clip(float(mismatch_focus_ratio), 0.0, 1.0))
            if focus_ratio > 0.0:
                eligible_reserved = np.where((neg_mask.to_numpy(dtype=bool)) & mismatch_fp.to_numpy(dtype=bool))[0]
                if len(eligible_reserved) > 0:
                    reserve_count = min(len(eligible_reserved), int(round(target_neg * focus_ratio)))
                    if reserve_count > 0:
                        reserve_scores = hardness[eligible_reserved] * np.clip(w_arr[eligible_reserved], 0.05, None)
                        order = np.argsort(-reserve_scores)
                        reserved_positions = np.sort(eligible_reserved[order[:reserve_count]])
            remaining_target = max(0, int(target_neg) - int(len(reserved_positions)))
            remaining_positions = np.array([pos for pos in neg_positions if pos not in set(reserved_positions.tolist())], dtype=int)
            if remaining_target > 0 and len(remaining_positions) > 0:
                remaining_probs = hardness[remaining_positions] * np.clip(w_arr[remaining_positions], 0.05, None)
                remaining_probs = np.clip(remaining_probs, 1e-6, None)
                remaining_probs = remaining_probs / remaining_probs.sum()
                chosen_pos = rng.choice(remaining_positions, size=min(remaining_target, len(remaining_positions)), replace=False, p=remaining_probs)
                combined_positions = np.unique(np.concatenate([reserved_positions, np.sort(chosen_pos)]))
            else:
                combined_positions = np.unique(reserved_positions)
            selected_neg_idx = X_df.index[np.sort(combined_positions)]
            info["negative_rows_selected"] = int(target_neg)
            info["mismatch_fp_negatives_kept"] = int(len(reserved_positions))
        else:
            info["negative_rows_selected"] = int(len(neg_idx))
    else:
        info["negative_rows_selected"] = int(len(neg_idx))

    selected_idx = np.concatenate([pos_idx, np.asarray(selected_neg_idx)])
    selected_idx = np.unique(selected_idx)
    X_sel = X_df.loc[selected_idx].copy()
    y_sel = y_ser.loc[selected_idx].copy()
    g_sel = g_ser.loc[selected_idx].copy()
    w_sel = np.asarray(w_arr[[X_df.index.get_loc(idx) for idx in selected_idx]], dtype=float)
    meta_sel = meta.loc[selected_idx].copy() if meta is not None else None

    if meta_sel is not None and not meta_sel.empty:
        info["hard_negative_kept"] = int(pd.to_numeric(meta_sel.get("hard_negative", pd.Series(0, index=meta_sel.index)), errors="coerce").fillna(0).astype(int).sum())
        info["transition_negative_kept"] = int(pd.to_numeric(meta_sel.get("transition_normal", pd.Series(0, index=meta_sel.index)), errors="coerce").fillna(0).astype(int).sum())
        info["near_arc_negative_kept"] = int(pd.to_numeric(meta_sel.get("near_arc_hard_negative", pd.Series(0, index=meta_sel.index)), errors="coerce").fillna(0).astype(int).sum())
        mismatch_type_meta = meta_sel.get("mismatch_type", pd.Series("", index=meta_sel.index)).astype(str)
        verified_meta = _series_boolish_to_int(meta_sel.get("label_verified"), meta_sel.index, default=0) == 1
        mismatch_fn_mask = mismatch_type_meta.eq("fn_override")
        if bool(mismatch_verified_only):
            mismatch_fn_mask &= verified_meta
        info["mismatch_fn_positives"] = int(mismatch_fn_mask.sum())

    oversample = max(1.0, float(positive_oversample))
    extra_repeats = int(np.floor(oversample) - 1)
    fractional = float(oversample - np.floor(oversample))
    if len(pos_idx) > 0 and (extra_repeats > 0 or fractional > 1e-9):
        extra_idx = []
        if extra_repeats > 0:
            for _ in range(extra_repeats):
                extra_idx.extend(pos_idx.tolist())
        if fractional > 1e-9:
            rng = np.random.default_rng(int(random_state) + 17)
            take = int(round(len(pos_idx) * fractional))
            if take > 0:
                picked = rng.choice(pos_idx, size=min(len(pos_idx), take), replace=False)
                extra_idx.extend(list(picked))
        if extra_idx:
            extra_idx = list(extra_idx)
            X_extra = X_df.loc[extra_idx].copy()
            y_extra = y_ser.loc[extra_idx].copy()
            g_extra = g_ser.loc[extra_idx].copy()
            w_extra = np.asarray(w_arr[[X_df.index.get_loc(idx) for idx in extra_idx]], dtype=float)
            X_sel = pd.concat([X_sel, X_extra], axis=0)
            y_sel = pd.concat([y_sel, y_extra], axis=0)
            g_sel = pd.concat([g_sel, g_extra], axis=0)
            w_sel = np.concatenate([w_sel, w_extra])
            if meta_sel is not None:
                meta_sel = pd.concat([meta_sel, meta.loc[extra_idx].copy()], axis=0)
            info["positive_rows_oversampled_extra"] = int(len(extra_idx))
        else:
            info["positive_rows_oversampled_extra"] = 0
    else:
        info["positive_rows_oversampled_extra"] = 0

    family_balance_info = {
        "enabled": False,
        "family_col": "device_family",
        "class_targets": {},
        "family_class_weight_before": {},
        "family_class_weight_after": {},
        "family_class_scale": {},
    }
    if bool(family_balance) and meta_sel is not None and not meta_sel.empty:
        w_sel, family_balance_info = rebalance_family_class_weights(
            meta_sel,
            y_sel,
            w_sel,
            family_col="device_family",
            max_scale=float(family_balance_max_scale),
            min_scale=float(family_balance_min_scale),
            include_unknown=False,
        )
    info["family_balance"] = family_balance_info

    info["final_rows"] = int(len(X_sel))
    info["final_positive_rows"] = int(np.sum(np.asarray(y_sel).astype(int) == 1))
    info["final_negative_rows"] = int(np.sum(np.asarray(y_sel).astype(int) == 0))
    return X_sel, y_sel, g_sel, np.asarray(w_sel).astype(float), meta_sel, info


def estimate_search_plan(n_iter: int) -> dict:
    base = max(1, int(n_iter))
    if base <= 1:
        refine = 0
        finalists = 1
        robustness_seeds = [42]
    elif base <= 3:
        refine = 1
        finalists = min(2, base)
        robustness_seeds = [42]
    elif base <= 6:
        refine = max(1, int(round(base * 0.50)))
        finalists = min(3, max(2, int(round(np.sqrt(base)))))
        robustness_seeds = [42, 314]
    else:
        refine = max(2, int(round(base * 0.75)))
        finalists = min(8, max(3, int(round(np.sqrt(base)))))
        robustness_seeds = [42, 314, 2024, 7331]
        if base >= 96:
            robustness_seeds.append(9001)
    total_steps = base + refine + (finalists * len(robustness_seeds)) + 1
    return {
        "broad_candidates": int(base),
        "refine_candidates": int(refine),
        "robust_finalists": int(finalists),
        "robustness_seeds": robustness_seeds,
        "progress_steps": int(total_steps),
    }


def _coerce_jsonable(value):
    if isinstance(value, (np.integer,)):
        return int(value)
    if isinstance(value, (np.floating,)):
        return float(value)
    return value


def _param_signature(params: dict) -> tuple:
    return tuple(sorted((str(k), repr(_coerce_jsonable(v))) for k, v in params.items()))


def _sample_param_set(space: dict, rng: np.random.Generator) -> dict:
    params = {}
    for name, values in space.items():
        vals = list(values)
        params[name] = vals[int(rng.integers(0, len(vals)))]
    return params


def _dedupe_candidates(candidates: list[dict]) -> list[dict]:
    seen = set()
    out = []
    for params in candidates:
        sig = _param_signature(params)
        if sig in seen:
            continue
        seen.add(sig)
        out.append(params)
    return out


def _build_broad_candidates(space: dict, count: int, seed: int = 42) -> list[dict]:
    rng = np.random.default_rng(seed)
    candidates = []
    attempts = 0
    max_attempts = max(200, count * 50)
    while len(candidates) < count and attempts < max_attempts:
        attempts += 1
        candidates.append(_sample_param_set(space, rng))
        candidates = _dedupe_candidates(candidates)
    return candidates[:count]


def _local_candidate_values(values: list, base_value):
    vals = list(values)
    if len(vals) <= 1:
        return vals
    if base_value in vals:
        i = vals.index(base_value)
        lo = max(0, i - 2)
        hi = min(len(vals), i + 3)
        window = vals[lo:hi]
        if base_value not in window:
            window.insert(0, base_value)
        return window
    return vals


def _build_refine_candidates(space: dict, seeds: list[dict], count: int, seed: int = 123) -> list[dict]:
    rng = np.random.default_rng(seed)
    if not seeds:
        return _build_broad_candidates(space, count=count, seed=seed)

    candidates = []
    attempts = 0
    max_attempts = max(400, count * 60)
    while len(candidates) < count and attempts < max_attempts:
        attempts += 1
        seed_params = seeds[int(rng.integers(0, len(seeds)))]
        params = {}
        for name, values in space.items():
            values = list(values)
            base_value = seed_params.get(name, values[0])
            if rng.random() < 0.68:
                pool = _local_candidate_values(values, base_value)
            else:
                pool = values
            params[name] = pool[int(rng.integers(0, len(pool)))]
        candidates.append(params)
        candidates = _dedupe_candidates(candidates)
    return candidates[:count]


def _fit_estimator(builder, params: dict, random_state_override: int | None = None):
    est = clone(builder())
    est.set_params(**params)
    if random_state_override is not None:
        try:
            est.set_params(random_state=int(random_state_override))
        except Exception:
            pass
    return est


def _cv_candidate_metrics(
    builder,
    params: dict,
    X_train,
    y_train,
    groups_train,
    w_train,
    n_splits: int,
    meta_train: pd.DataFrame | None = None,
    random_state_override: int | None = None,
    fn_weight: float = 80.0,
    fp_weight: float = 4.0,
    min_recall: float = 0.97,
    min_precision: float = 0.0,
    max_fpr: float = 1.0,
    min_threshold: float = 0.05,
    arc_tolerance: ArcToleranceConfig | dict | None = None,
) -> dict:
    fold_splits = _class_balanced_group_kfold_splits(
        X_train,
        y_train,
        groups_train,
        n_splits=n_splits,
    )
    ap_scores = []
    roc_scores = []
    bal_scores = []
    recall_scores = []
    precision_scores = []
    f1_scores = []
    specificity_scores = []
    fpr_scores = []
    threshold_scores = []
    cost_scores = []
    constraint_scores = []
    event_recall_scores = []
    event_fn_scores = []
    false_alarm_scores = []
    missing_class_folds = 0

    y_train_np = np.asarray(y_train).astype(int)
    w_train_np = np.asarray(w_train).astype(float)

    for fold_idx, (fit_idx, val_idx) in enumerate(fold_splits, start=1):
        est = _fit_estimator(builder, params, random_state_override=random_state_override)
        try:
            est.fit(
                X_train.iloc[fit_idx],
                y_train.iloc[fit_idx],
                sample_weight=w_train_np[fit_idx],
            )
            y_val_fold = y_train_np[val_idx]
            proba = est.predict_proba(X_train.iloc[val_idx])[:, 1]
            threshold_result = select_threshold_cost(
                y_val_fold,
                proba,
                fn_weight=fn_weight,
                fp_weight=fp_weight,
                min_recall=min_recall,
                min_precision=min_precision,
                max_fpr=max_fpr,
                min_threshold=min_threshold,
                meta_df=(meta_train.iloc[val_idx].reset_index(drop=True) if isinstance(meta_train, pd.DataFrame) else None),
                arc_tolerance=arc_tolerance,
            )
            metrics = evaluate_binary_scores(y_val_fold, proba, threshold_result["thr"])
            if np.unique(y_val_fold).size < 2:
                missing_class_folds += 1
        except Exception as exc:
            return {
                "params": dict(params),
                "folds": int(fold_idx - 1),
                "cv_average_precision": 0.0,
                "cv_average_precision_std": 1.0,
                "cv_roc_auc": 0.5,
                "cv_balanced_accuracy": 0.0,
                "cv_recall": 0.0,
                "cv_precision": 0.0,
                "cv_specificity": 0.0,
                "cv_fpr": 1.0,
                "cv_threshold_mean": float(min_threshold),
                "cv_threshold_cost": float("inf"),
                "cv_constraints_met_rate": 0.0,
                "cv_f1": 0.0,
                "missing_class_folds": int(missing_class_folds),
                "missing_class_rate": 1.0,
                "search_score": -1.0,
                "cv_error": str(exc),
            }

        ap_scores.append(_safe_float(metrics["average_precision"]))
        roc_scores.append(_safe_float(metrics["roc_auc"], 0.5))
        bal_scores.append(_safe_float(metrics["balanced_accuracy"]))
        recall_scores.append(_safe_float(metrics["recall"]))
        precision_scores.append(_safe_float(metrics["precision"]))
        f1_scores.append(_safe_float(metrics["f1"]))
        specificity_scores.append(_safe_float(metrics["specificity"]))
        fpr_scores.append(_safe_float(metrics["fpr"]))
        threshold_scores.append(_safe_float(threshold_result.get("thr"), min_threshold))
        cost_scores.append(_safe_float(threshold_result.get("cost"), 1e18))
        constraint_scores.append(1.0 if threshold_result.get("constraints_met", False) else 0.0)
        event_recall_scores.append(_safe_float(threshold_result.get("event_recall"), 0.0))
        event_fn_scores.append(_safe_float(threshold_result.get("event_fn"), 0.0))
        false_alarm_scores.append(_safe_float(threshold_result.get("false_alarms_per_session"), 0.0))

    ap_mean = float(np.mean(ap_scores)) if ap_scores else 0.0
    ap_std = float(np.std(ap_scores)) if ap_scores else 0.0
    roc_mean = float(np.mean(roc_scores)) if roc_scores else 0.5
    bal_mean = float(np.mean(bal_scores)) if bal_scores else 0.0
    recall_mean = float(np.mean(recall_scores)) if recall_scores else 0.0
    precision_mean = float(np.mean(precision_scores)) if precision_scores else 0.0
    f1_mean = float(np.mean(f1_scores)) if f1_scores else 0.0
    specificity_mean = float(np.mean(specificity_scores)) if specificity_scores else 0.0
    fpr_mean = float(np.mean(fpr_scores)) if fpr_scores else 1.0
    threshold_mean = float(np.mean(threshold_scores)) if threshold_scores else float(min_threshold)
    raw_cost_mean = float(np.mean(cost_scores)) if cost_scores else 1e18
    constraints_met_rate = float(np.mean(constraint_scores)) if constraint_scores else 0.0
    missing_class_rate = float(missing_class_folds / max(1, len(ap_scores)))
    event_recall_mean = float(np.mean(event_recall_scores)) if event_recall_scores else 0.0
    event_fn_mean = float(np.mean(event_fn_scores)) if event_fn_scores else 1e18
    false_alarm_mean = float(np.mean(false_alarm_scores)) if false_alarm_scores else 0.0

    cost_unit = max(1.0, (0.65 * float(fn_weight)) + (12.0 * float(fp_weight)))
    cost_penalty = float(np.tanh(raw_cost_mean / cost_unit)) if np.isfinite(raw_cost_mean) else 1.0
    fpr_penalty = max(0.0, fpr_mean - float(max_fpr))
    recall_shortfall = max(0.0, float(min_recall) - recall_mean)
    precision_shortfall = max(0.0, float(min_precision) - precision_mean)
    event_fn_penalty = 0.0 if not np.isfinite(event_fn_mean) else min(1.0, float(event_fn_mean) / max(1.0, float(np.sum(np.asarray(y_train_np) == 1))))

    search_score = (
        (0.12 * ap_mean)
        + (0.22 * event_recall_mean)
        + (0.10 * recall_mean)
        + (0.16 * precision_mean)
        + (0.18 * bal_mean)
        + (0.16 * specificity_mean)
        + (0.05 * f1_mean)
        + (0.05 * roc_mean)
        + (0.14 * constraints_met_rate)
        - (0.03 * ap_std)
        - (0.08 * cost_penalty)
        - (0.24 * fpr_penalty)
        - (0.22 * event_fn_penalty)
        - (0.12 * min(1.0, false_alarm_mean))
        - (0.10 * recall_shortfall)
        - (0.16 * precision_shortfall)
        - (0.08 * missing_class_rate)
    )

    return {
        "params": dict(params),
        "folds": int(len(ap_scores)),
        "cv_average_precision": float(ap_mean),
        "cv_average_precision_std": float(ap_std),
        "cv_roc_auc": float(roc_mean),
        "cv_balanced_accuracy": float(bal_mean),
        "cv_recall": float(recall_mean),
        "cv_precision": float(precision_mean),
        "cv_specificity": float(specificity_mean),
        "cv_fpr": float(fpr_mean),
        "cv_threshold_mean": float(threshold_mean),
        "cv_threshold_cost": float(raw_cost_mean),
        "cv_constraints_met_rate": float(constraints_met_rate),
        "cv_f1": float(f1_mean),
        "cv_event_recall": float(event_recall_mean),
        "cv_event_fn": float(event_fn_mean if np.isfinite(event_fn_mean) else 0.0),
        "cv_false_alarms_per_session": float(false_alarm_mean),
        "missing_class_folds": int(missing_class_folds),
        "missing_class_rate": float(missing_class_rate),
        "search_score": float(search_score),
    }


def _report_search_progress(progress_callback, current: int, total: int, payload: dict) -> None:
    if progress_callback is None:
        return
    progress_callback(int(current), int(total), payload)




def train_one_model(
    model_key: str,
    X_train,
    y_train,
    groups_train,
    w_train,
    X_val,
    y_val,
    X_test,
    y_test,
    fn_weight: float,
    fp_weight: float,
    min_recall: float,
    n_iter: int,
    min_precision: float = 0.0,
    max_fpr: float = 1.0,
    min_threshold: float = 0.05,
    meta_train: pd.DataFrame | None = None,
    meta_val: pd.DataFrame | None = None,
    meta_test: pd.DataFrame | None = None,
    arc_tolerance: ArcToleranceConfig | dict | None = None,
    search_profile: str = "full",
    progress_callback=None,
) -> dict:
    if model_key not in MODEL_CONFIGS:
        raise ValueError(f"Unsupported model: {model_key}")

    cfg = MODEL_CONFIGS[model_key]
    pretty = cfg["pretty_name"]
    builder = cfg["builder"]
    param_space = resolve_model_search_profile(model_key, search_profile)

    group_summary = _group_summary_frame(groups_train, y_train)
    pos_group_count = int((group_summary["group_label"] == 1).sum())
    neg_group_count = int((group_summary["group_label"] == 0).sum())
    n_splits = int(min(5, len(group_summary), pos_group_count, neg_group_count)) if pos_group_count > 0 and neg_group_count > 0 else 0
    if n_splits < 2:
        raise ValueError(
            "Need at least 2 positive sessions and 2 normal sessions for class-balanced grouped CV. "
            f"Found {pos_group_count} positive groups and {neg_group_count} normal groups."
        )

    plan = estimate_search_plan(n_iter)
    total_steps = plan["progress_steps"]
    broad_candidates = _build_broad_candidates(
        param_space,
        count=plan["broad_candidates"],
        seed=42,
    )
    search_rows = []
    progress_step = 0

    _report_search_progress(
        progress_callback,
        progress_step,
        total_steps,
        {
            "model_key": model_key,
            "model_name": pretty,
            "stage": "search_init",
            "message": f"Preparing {plan['broad_candidates']} broad candidates",
        },
    )

    for idx, params in enumerate(broad_candidates, start=1):
        row = _cv_candidate_metrics(
            builder=builder,
            params=params,
            X_train=X_train,
            y_train=y_train,
            groups_train=groups_train,
            w_train=w_train,
            n_splits=n_splits,
            meta_train=meta_train,
            fn_weight=fn_weight,
            fp_weight=fp_weight,
            min_recall=min_recall,
            min_precision=min_precision,
            max_fpr=max_fpr,
            min_threshold=min_threshold,
            arc_tolerance=arc_tolerance,
        )
        row["search_stage"] = "broad"
        row["candidate_index"] = int(idx)
        search_rows.append(row)
        progress_step += 1
        _report_search_progress(
            progress_callback,
            progress_step,
            total_steps,
            {
                "model_key": model_key,
                "model_name": pretty,
                "stage": "broad_search",
                "message": (
                    f"Broad {idx}/{plan['broad_candidates']} | "
                    f"AP={row['cv_average_precision']:.4f} | Recall={row.get('cv_recall', 0.0):.4f} | "
                    f"Score={row['search_score']:.4f} | Constraints={row.get('cv_constraints_met_rate', 0.0):.2f}"
                ),
                "cv_average_precision": row["cv_average_precision"],
                "search_score": row["search_score"],
                "missing_class_folds": int(row.get("missing_class_folds", 0)),
                "cv_constraints_met_rate": float(row.get("cv_constraints_met_rate", 0.0)),
            },
        )

    prelim = sorted(
        search_rows,
        key=lambda r: (
            -_safe_float(r.get("search_score")),
            -_safe_float(r.get("cv_average_precision")),
            _safe_float(r.get("cv_average_precision_std"), 1e18),
        ),
    )
    seed_params = [r["params"] for r in prelim[: max(3, plan["robust_finalists"])]]
    refine_candidates = _build_refine_candidates(
        param_space,
        seeds=seed_params,
        count=plan["refine_candidates"],
        seed=123,
    )

    used_signatures = {_param_signature(r["params"]) for r in search_rows}
    kept_refine = []
    for params in refine_candidates:
        sig = _param_signature(params)
        if sig in used_signatures:
            continue
        used_signatures.add(sig)
        kept_refine.append(params)

    if len(kept_refine) < plan["refine_candidates"]:
        extras = _build_broad_candidates(
            param_space,
            count=plan["refine_candidates"] * 2,
            seed=777,
        )
        for params in extras:
            sig = _param_signature(params)
            if sig in used_signatures:
                continue
            used_signatures.add(sig)
            kept_refine.append(params)
            if len(kept_refine) >= plan["refine_candidates"]:
                break

    kept_refine = kept_refine[: plan["refine_candidates"]]

    for idx, params in enumerate(kept_refine, start=1):
        row = _cv_candidate_metrics(
            builder=builder,
            params=params,
            X_train=X_train,
            y_train=y_train,
            groups_train=groups_train,
            w_train=w_train,
            n_splits=n_splits,
            meta_train=meta_train,
            fn_weight=fn_weight,
            fp_weight=fp_weight,
            min_recall=min_recall,
            min_precision=min_precision,
            max_fpr=max_fpr,
            min_threshold=min_threshold,
            arc_tolerance=arc_tolerance,
        )
        row["search_stage"] = "refine"
        row["candidate_index"] = int(idx)
        search_rows.append(row)
        progress_step += 1
        _report_search_progress(
            progress_callback,
            progress_step,
            total_steps,
            {
                "model_key": model_key,
                "model_name": pretty,
                "stage": "refine_search",
                "message": (
                    f"Refine {idx}/{len(kept_refine)} | "
                    f"AP={row['cv_average_precision']:.4f} | Recall={row.get('cv_recall', 0.0):.4f} | "
                    f"Score={row['search_score']:.4f} | Constraints={row.get('cv_constraints_met_rate', 0.0):.2f}"
                ),
                "cv_average_precision": row["cv_average_precision"],
                "search_score": row["search_score"],
                "missing_class_folds": int(row.get("missing_class_folds", 0)),
                "cv_constraints_met_rate": float(row.get("cv_constraints_met_rate", 0.0)),
            },
        )

    ranked_search = sorted(
        search_rows,
        key=lambda r: (
            -_safe_float(r.get("search_score")),
            -_safe_float(r.get("cv_average_precision")),
            _safe_float(r.get("cv_average_precision_std"), 1e18),
        ),
    )

    finalists = ranked_search[: plan["robust_finalists"]]
    robust_rows = []
    for finalist_idx, row in enumerate(finalists, start=1):
        seed_rows = []
        for seed in plan["robustness_seeds"]:
            robust = _cv_candidate_metrics(
                builder=builder,
                params=row["params"],
                X_train=X_train,
                y_train=y_train,
                groups_train=groups_train,
                w_train=w_train,
                n_splits=n_splits,
                meta_train=meta_train,
                random_state_override=seed,
                fn_weight=fn_weight,
                fp_weight=fp_weight,
                min_recall=min_recall,
                min_precision=min_precision,
                max_fpr=max_fpr,
                min_threshold=min_threshold,
                arc_tolerance=arc_tolerance,
            )
            seed_rows.append(robust)
            progress_step += 1
            _report_search_progress(
                progress_callback,
                progress_step,
                total_steps,
                {
                    "model_key": model_key,
                    "model_name": pretty,
                    "stage": "robustness_check",
                    "message": (
                        f"Stability {finalist_idx}/{len(finalists)} | seed={seed} | "
                        f"AP={robust['cv_average_precision']:.4f} | Recall={robust.get('cv_recall', 0.0):.4f} | "
                        f"Score={robust['search_score']:.4f}"
                    ),
                    "cv_average_precision": robust["cv_average_precision"],
                    "search_score": robust["search_score"],
                    "missing_class_folds": int(robust.get("missing_class_folds", 0)),
                },
            )

        robust_score = float(np.mean([r["search_score"] for r in seed_rows])) if seed_rows else -1.0
        robust_ap = float(np.mean([r["cv_average_precision"] for r in seed_rows])) if seed_rows else 0.0
        robust_ap_std = float(np.std([r["cv_average_precision"] for r in seed_rows])) if seed_rows else 1.0
        robust_rows.append({
            "params": dict(row["params"]),
            "robust_search_score": robust_score,
            "robust_cv_average_precision": robust_ap,
            "robust_cv_average_precision_std": robust_ap_std,
            "robust_cv_recall": float(np.mean([r.get("cv_recall", 0.0) for r in seed_rows])) if seed_rows else 0.0,
            "robust_cv_precision": float(np.mean([r.get("cv_precision", 0.0) for r in seed_rows])) if seed_rows else 0.0,
            "robust_cv_constraints_met_rate": float(np.mean([r.get("cv_constraints_met_rate", 0.0) for r in seed_rows])) if seed_rows else 0.0,
            "base_search_score": row["search_score"],
            "base_cv_average_precision": row["cv_average_precision"],
            "base_cv_recall": row.get("cv_recall", 0.0),
            "base_cv_precision": row.get("cv_precision", 0.0),
            "base_cv_constraints_met_rate": row.get("cv_constraints_met_rate", 0.0),
            "seed_breakdown": seed_rows,
        })

    robust_ranked = sorted(
        robust_rows,
        key=lambda r: (
            -_safe_float(r.get("robust_search_score")),
            -_safe_float(r.get("robust_cv_average_precision")),
            _safe_float(r.get("robust_cv_average_precision_std"), 1e18),
        ),
    )

    if robust_ranked:
        final_choice = robust_ranked[0]
        best_params = dict(final_choice["params"])
        search_meta = {
            "strategy": "broad_random_then_local_refine_then_seed_robustness",
            "plan": plan,
            "best_search_score": float(final_choice["robust_search_score"]),
            "best_cv_average_precision": float(final_choice["robust_cv_average_precision"]),
            "best_cv_average_precision_std": float(final_choice["robust_cv_average_precision_std"]),
            "top_candidates": robust_ranked,
            "evaluated_candidates": ranked_search,
        }
    else:
        best_params = dict(ranked_search[0]["params"])
        search_meta = {
            "strategy": "broad_random_then_local_refine",
            "plan": plan,
            "best_search_score": float(ranked_search[0]["search_score"]),
            "best_cv_average_precision": float(ranked_search[0]["cv_average_precision"]),
            "best_cv_average_precision_std": float(ranked_search[0]["cv_average_precision_std"]),
            "top_candidates": ranked_search[: plan["robust_finalists"]],
            "evaluated_candidates": ranked_search,
        }

    progress_step = total_steps - 1
    _report_search_progress(
        progress_callback,
        progress_step,
        total_steps,
        {
            "model_key": model_key,
            "model_name": pretty,
            "stage": "final_fit",
            "message": "Fitting final model and validating threshold",
        },
    )

    best = _fit_estimator(builder, best_params, random_state_override=42)
    best.fit(X_train, y_train, sample_weight=np.asarray(w_train).astype(float))
    val_proba = best.predict_proba(X_val)[:, 1]
    best_thr = select_threshold_cost(
        y_val.to_numpy(),
        val_proba,
        fn_weight=fn_weight,
        fp_weight=fp_weight,
        min_recall=min_recall,
        min_precision=min_precision,
        max_fpr=max_fpr,
        min_threshold=min_threshold,
        meta_df=(meta_val.reset_index(drop=True) if isinstance(meta_val, pd.DataFrame) else None),
        arc_tolerance=arc_tolerance,
    )
    thr = float(best_thr["thr"])

    val_metrics = evaluate_binary_scores(y_val.to_numpy(), val_proba, thr)
    y_score = best.predict_proba(X_test)[:, 1]
    test_metrics = evaluate_binary_scores(y_test, y_score, thr)
    val_event_metrics = build_event_level_report(
        meta_val.reset_index(drop=True) if isinstance(meta_val, pd.DataFrame) else None,
        y_val.to_numpy(),
        val_proba,
        thr,
        tolerance=arc_tolerance,
    ) if isinstance(meta_val, pd.DataFrame) else {}
    test_event_metrics = build_event_level_report(
        meta_test.reset_index(drop=True) if isinstance(meta_test, pd.DataFrame) else None,
        np.asarray(y_test).astype(int),
        y_score,
        thr,
        tolerance=arc_tolerance,
    ) if isinstance(meta_test, pd.DataFrame) else {}

    fi = pd.Series(
        best.feature_importances_,
        index=list(X_train.columns),
    ).sort_values(ascending=False)

    progress_step = total_steps
    _report_search_progress(
        progress_callback,
        progress_step,
        total_steps,
        {
            "model_key": model_key,
            "model_name": pretty,
            "stage": "done",
            "message": f"Done | CV AP={search_meta['best_cv_average_precision']:.4f} | Test AP={test_metrics['average_precision']:.4f}",
            "cv_average_precision": search_meta["best_cv_average_precision"],
            "test_average_precision": test_metrics["average_precision"],
        },
    )

    result = {
        "model_key": model_key,
        "model_name": pretty,
        "best_params": best_params,
        "cv_best_average_precision": float(search_meta["best_cv_average_precision"]),
        "cv_best_average_precision_std": float(search_meta.get("best_cv_average_precision_std", 0.0)),
        "cv_search_best_score": float(search_meta["best_search_score"]),
        "cv_best_recall": float(search_meta.get("top_candidates", [{}])[0].get("robust_cv_recall", search_meta.get("top_candidates", [{}])[0].get("cv_recall", 0.0)) if search_meta.get("top_candidates") else 0.0),
        "search_summary": search_meta,
        "estimated_node_count": model_size_estimate(best),
        "threshold": thr,
        "threshold_source": best_thr.get("selection_reason", "validation_cost"),
        "threshold_constraints_met": bool(best_thr.get("constraints_met", False)),
        "validation_threshold_result": best_thr,
        "validation_precision": float(val_metrics.get("precision", 0.0)),
        "holdout_validation_precision": float(val_metrics.get("precision", 0.0)),
        "holdout_validation_recall": float(val_metrics.get("recall", 0.0)),
        "validation_metrics": val_metrics,
        "validation_accuracy": val_metrics["accuracy"],
        "validation_average_precision": val_metrics["average_precision"],
        "validation_roc_auc": val_metrics["roc_auc"],
        "validation_precision": val_metrics["precision"],
        "validation_recall": val_metrics["recall"],
        "validation_f1": val_metrics["f1"],
        "validation_balanced_accuracy": val_metrics["balanced_accuracy"],
        "validation_specificity": val_metrics["specificity"],
        "validation_npv": val_metrics["npv"],
        "validation_fpr": val_metrics["fpr"],
        "validation_fnr": val_metrics["fnr"],
        "validation_mcc": val_metrics["mcc"],
        "validation_confusion_matrix": val_metrics["confusion_matrix"],
        "validation_classification_report": val_metrics["classification_report"],
        "validation_event_level_metrics": val_event_metrics,
        "test_metrics": test_metrics,
        "test_accuracy": test_metrics["accuracy"],
        "test_average_precision": test_metrics["average_precision"],
        "test_roc_auc": test_metrics["roc_auc"],
        "test_precision": test_metrics["precision"],
        "test_recall": test_metrics["recall"],
        "test_f1": test_metrics["f1"],
        "test_balanced_accuracy": test_metrics["balanced_accuracy"],
        "test_specificity": test_metrics["specificity"],
        "test_npv": test_metrics["npv"],
        "test_fpr": test_metrics["fpr"],
        "test_fnr": test_metrics["fnr"],
        "test_mcc": test_metrics["mcc"],
        "test_confusion_matrix": test_metrics["confusion_matrix"],
        "test_classification_report": test_metrics["classification_report"],
        "test_event_level_metrics": test_event_metrics,
        "split_sizes": {
            "train_rows": int(len(X_train)),
            "validation_rows": int(len(X_val)),
            "val_rows": int(len(X_val)),
            "test_rows": int(len(X_test)),
            "train_groups": int(groups_train.nunique()),
            "cv_splits": int(n_splits),
            "positive_groups": int(pos_group_count),
            "negative_groups": int(neg_group_count),
            "train_positive": int(np.sum(np.asarray(y_train) == 1)),
            "validation_positive": int(np.sum(np.asarray(y_val) == 1)),
            "test_positive": int(np.sum(np.asarray(y_test) == 1)),
        },
        "training_semantics": {
            "arc_tolerance_mode": normalize_arc_tolerance_config(arc_tolerance).normalized_mode(),
            "pre_arc_window": int(normalize_arc_tolerance_config(arc_tolerance).pre_rows),
            "post_arc_window": int(normalize_arc_tolerance_config(arc_tolerance).post_rows),
            "search_profile": str(search_profile),
        },
        "feature_importances": {
            str(k): float(v) for k, v in fi.items()
        },
        "validation_score_summary": val_metrics["label_score_summary"],
        "test_score_summary": test_metrics["label_score_summary"],
        "score_summary": {
            "validation": val_metrics["score_summary"],
            "test": test_metrics["score_summary"],
        },
        "estimator": best,
    }
    return result


def _preferred_meta_col(df: pd.DataFrame, candidates: list[str]) -> str | None:
    for col in candidates:
        if col in df.columns:
            return col
    return None




def family_label_from_code(code: int) -> str:
    return DEVICE_FAMILY_NAME_FROM_CODE.get(int(code), "unknown")


def pick_runtime_family_code(meta_df: pd.DataFrame, row_idx: int, runtime_col: str = "context_family_code_runtime") -> int:
    if meta_df is None or runtime_col not in meta_df.columns:
        return DEVICE_FAMILY_UNKNOWN_CODE
    try:
        return _coerce_device_family_code(meta_df.iloc[int(row_idx)][runtime_col])
    except Exception:
        return DEVICE_FAMILY_UNKNOWN_CODE


def pick_context_confidence(meta_df: pd.DataFrame, row_idx: int, confidence_col: str = "context_family_confidence") -> float:
    if meta_df is None or confidence_col not in meta_df.columns:
        return 0.0
    try:
        return float(pd.to_numeric(pd.Series([meta_df.iloc[int(row_idx)][confidence_col]]), errors="coerce").fillna(0.0).iloc[0])
    except Exception:
        return 0.0


def calibrate_family_threshold_policy(
    val_meta: pd.DataFrame,
    y_true,
    y_score,
    *,
    base_threshold: float,
    fn_weight: float = 80.0,
    fp_weight: float = 4.0,
    min_recall: float = 0.97,
    min_precision: float = 0.90,
    max_fpr: float = 0.03,
    min_threshold: float = 0.08,
    known_confidence_min: float = 0.45,
    family_min_rows: int = 12,
) -> dict:
    y_true = np.asarray(y_true).astype(int)
    y_score = np.asarray(y_score).astype(float)
    work = (val_meta.copy() if val_meta is not None else pd.DataFrame(index=np.arange(len(y_true))))
    if len(work) != len(y_true):
        work = work.reset_index(drop=True).iloc[:len(y_true)].copy()
    work["_y_true"] = y_true
    work["_y_score"] = y_score

    if "device_family_code" in work.columns:
        truth_family_code = pd.to_numeric(work["device_family_code"], errors="coerce").fillna(-1).astype(int)
    elif "context_family_code_runtime" in work.columns:
        truth_family_code = pd.to_numeric(work["context_family_code_runtime"], errors="coerce").fillna(-1).astype(int)
    else:
        truth_family_code = pd.Series(DEVICE_FAMILY_UNKNOWN_CODE, index=work.index, dtype=int)
    work["device_family_code"] = truth_family_code

    if "context_family_code_runtime" not in work.columns:
        work["context_family_code_runtime"] = truth_family_code.astype(int)
    else:
        work["context_family_code_runtime"] = pd.to_numeric(work["context_family_code_runtime"], errors="coerce").fillna(-1).astype(int)

    if "context_family_confidence" not in work.columns:
        work["context_family_confidence"] = np.where(work["context_family_code_runtime"] >= 0, 1.0, 0.0)
    else:
        work["context_family_confidence"] = pd.to_numeric(work["context_family_confidence"], errors="coerce").fillna(0.0).clip(0.0, 1.0)

    truth_known_rows = work[work["device_family_code"] >= 0].copy()
    runtime_known_rows = work[
        (work["context_family_code_runtime"] >= 0)
        & (work["context_family_confidence"] >= float(known_confidence_min))
    ].copy()

    per_family = {}
    pending_unknown_fallback = []
    family_assignment_source = "device_family_code"
    for fam_idx, fam_name in DEVICE_FAMILY_NAME_FROM_CODE.items():
        truth_group = truth_known_rows[truth_known_rows["device_family_code"] == int(fam_idx)].copy()
        runtime_group = runtime_known_rows[runtime_known_rows["context_family_code_runtime"] == int(fam_idx)].copy()
        calibration_group = truth_group if len(truth_group) else runtime_group
        source = "fallback_unknown"
        details = {
            "family": fam_name,
            "family_code": int(fam_idx),
            "rows": int(len(calibration_group)),
            "positive_rows": int(calibration_group["_y_true"].sum()) if len(calibration_group) else 0,
            "negative_rows": int((calibration_group["_y_true"] == 0).sum()) if len(calibration_group) else 0,
            "truth_family_rows": int(len(truth_group)),
            "runtime_known_rows": int(len(runtime_group)),
            "calibration_family_source": "device_family_code" if len(truth_group) else ("context_family_runtime" if len(runtime_group) else "none"),
        }
        thr = None
        thr_meta = None
        if len(calibration_group) >= family_min_rows and calibration_group["_y_true"].nunique() >= 2:
            thr_meta = select_threshold_cost(
                calibration_group["_y_true"].to_numpy(),
                calibration_group["_y_score"].to_numpy(),
                fn_weight=fn_weight,
                fp_weight=fp_weight,
                min_recall=min_recall,
                min_precision=min_precision,
                max_fpr=max_fpr,
                min_threshold=min_threshold,
            )
            thr = float(thr_meta["thr"])
            source = "family_validation"
        elif len(calibration_group) >= family_min_rows and int(calibration_group["_y_true"].sum()) == 0:
            neg = np.sort(calibration_group["_y_score"].to_numpy())
            if neg.size:
                thr = float(min(0.995, max(base_threshold, np.quantile(neg, 0.995) + 0.01)))
                source = "family_negative_only"
        elif len(calibration_group) >= family_min_rows and int((calibration_group["_y_true"] == 1).sum()) == len(calibration_group):
            pos = np.sort(calibration_group["_y_score"].to_numpy())
            if pos.size:
                # Positive-only family support is weaker than mixed validation, but still better than
                # silently treating a present family as unknown. Keep the threshold conservative.
                thr = float(max(min_threshold, min(base_threshold, np.quantile(pos, 0.20))))
                source = "family_positive_only"
        else:
            pending_unknown_fallback.append(str(int(fam_idx)))
        per_family[str(int(fam_idx))] = {
            "threshold": None if thr is None else float(thr),
            "source": source,
            "selection": thr_meta,
            **details,
        }

    unknown_group = work[(work["context_family_code_runtime"] < 0) | (work["context_family_confidence"] < float(known_confidence_min))].copy()
    unknown_threshold = float(max(min_threshold, base_threshold + 0.12, 0.55))
    unknown_source = "strict_fallback"
    unknown_sel = None
    if len(unknown_group) >= max(10, family_min_rows) and unknown_group["_y_true"].nunique() >= 2:
        unknown_sel = select_threshold_cost(
            unknown_group["_y_true"].to_numpy(),
            unknown_group["_y_score"].to_numpy(),
            fn_weight=max(fn_weight, 120.0),
            fp_weight=max(fp_weight * 3.0, 12.0),
            min_recall=max(0.75, min_recall - 0.15),
            min_precision=max(0.98, min_precision),
            max_fpr=min(max_fpr, 0.005),
            min_threshold=max(min_threshold, base_threshold + 0.10, 0.50),
        )
        unknown_threshold = float(max(unknown_threshold, unknown_sel["thr"]))
        unknown_source = "unknown_validation"
    elif len(unknown_group) >= max(10, family_min_rows) and int(unknown_group["_y_true"].sum()) == 0:
        neg = np.sort(unknown_group["_y_score"].to_numpy())
        if neg.size:
            unknown_threshold = float(min(0.999, max(unknown_threshold, np.quantile(neg, 0.999) + 0.01)))
            unknown_source = "unknown_negative_only"

    for fam_key in pending_unknown_fallback:
        if fam_key in per_family:
            per_family[fam_key]["threshold"] = float(unknown_threshold)
            per_family[fam_key]["source"] = "fallback_unknown"

    return {
        "policy_type": "family_conditioned_thresholds",
        "base_threshold": float(base_threshold),
        "known_confidence_min": float(known_confidence_min),
        "family_assignment_source": family_assignment_source,
        "family_min_rows": int(family_min_rows),
        "family_thresholds": per_family,
        "unknown_threshold": float(unknown_threshold),
        "unknown_policy": {
            "threshold": float(unknown_threshold),
            "source": unknown_source,
            "selection": unknown_sel,
            "rows": int(len(unknown_group)),
            "positive_rows": int(unknown_group["_y_true"].sum()) if len(unknown_group) else 0,
            "negative_rows": int((unknown_group["_y_true"] == 0).sum()) if len(unknown_group) else 0,
            "min_positive_feature_votes": 3,
            "description": "Unknown or low-confidence context requires a stricter threshold and corroborating arc-like features.",
        },
    }


def threshold_for_policy(policy: dict | None, family_code: int, confidence: float) -> float:
    if not isinstance(policy, dict):
        return 0.5
    base = float(policy.get("base_threshold", 0.5))
    if int(family_code) >= 0 and float(confidence) >= float(policy.get("known_confidence_min", 0.45)):
        fam = (policy.get("family_thresholds") or {}).get(str(int(family_code))) or {}
        return float(fam.get("threshold", base))
    return float((policy.get("unknown_policy") or {}).get("threshold", policy.get("unknown_threshold", max(base, 0.65))))


def apply_threshold_policy(meta_df: pd.DataFrame, y_score, policy: dict | None) -> tuple[np.ndarray, np.ndarray]:
    y_score = np.asarray(y_score).astype(float)
    n = len(y_score)
    meta = meta_df.reset_index(drop=True) if meta_df is not None else pd.DataFrame(index=np.arange(n))
    thresholds = np.zeros(n, dtype=float)
    y_pred = np.zeros(n, dtype=int)
    for i in range(n):
        fam = pick_runtime_family_code(meta, i)
        conf = pick_context_confidence(meta, i)
        thr = threshold_for_policy(policy, fam, conf)
        thresholds[i] = float(thr)
        y_pred[i] = int(y_score[i] >= thr)
    return y_pred, thresholds


def evaluate_threshold_policy(meta_df: pd.DataFrame, y_true, y_score, policy: dict | None) -> dict:
    y_true = np.asarray(y_true).astype(int)
    y_score = np.asarray(y_score).astype(float)
    y_pred, thresholds = apply_threshold_policy(meta_df, y_score, policy)
    tn, fp, fn, tp = confusion_matrix(y_true, y_pred, labels=[0, 1]).ravel()
    recall = _safe_rate(tp, tp + fn)
    specificity = _safe_rate(tn, tn + fp)
    out = {
        "accuracy": float(_safe_rate(tp + tn, tp + tn + fp + fn)),
        "balanced_accuracy": float((recall + specificity) * 0.5),
        "average_precision": _safe_average_precision(y_true, y_score),
        "roc_auc": _safe_roc_auc(y_true, y_score),
        "precision": float(precision_score(y_true, y_pred, zero_division=0)),
        "recall": float(recall),
        "f1": float(f1_score(y_true, y_pred, zero_division=0)),
        "specificity": float(specificity),
        "npv": _safe_rate(tn, tn + fn),
        "fpr": _safe_rate(fp, fp + tn),
        "fnr": _safe_rate(fn, fn + tp),
        "confusion_matrix": {"tn": int(tn), "fp": int(fp), "fn": int(fn), "tp": int(tp)},
        "threshold_mean": float(np.mean(thresholds)) if thresholds.size else 0.0,
        "threshold_min": float(np.min(thresholds)) if thresholds.size else 0.0,
        "threshold_max": float(np.max(thresholds)) if thresholds.size else 0.0,
        "row_count": int(len(y_true)),
        "positive_count": int(np.sum(y_true == 1)),
        "negative_count": int(np.sum(y_true == 0)),
    }
    return out


def build_policy_subset_metrics_report(meta_df: pd.DataFrame, y_true, y_score, policy: dict | None, group_col: str, min_rows: int = 6) -> list[dict]:
    if meta_df is None or group_col not in meta_df.columns:
        return []
    frame = meta_df.copy().reset_index(drop=True)
    frame["_y_true"] = np.asarray(y_true).astype(int)
    frame["_y_score"] = np.asarray(y_score).astype(float)
    pred, thr = apply_threshold_policy(frame, frame["_y_score"].to_numpy(), policy)
    frame["_y_pred"] = pred
    frame["_thr"] = thr
    rows = []
    for value, group in frame.groupby(group_col, dropna=False, sort=True):
        if len(group) < min_rows:
            continue
        tn, fp, fn, tp = confusion_matrix(group["_y_true"].to_numpy(), group["_y_pred"].to_numpy(), labels=[0, 1]).ravel()
        recall = _safe_rate(tp, tp + fn)
        specificity = _safe_rate(tn, tn + fp)
        rows.append({
            "group": "<blank>" if pd.isna(value) else str(value),
            "rows": int(len(group)),
            "positive_rows": int(group["_y_true"].sum()),
            "threshold_mean": float(group["_thr"].mean()),
            "threshold_min": float(group["_thr"].min()),
            "threshold_max": float(group["_thr"].max()),
            "average_precision": _safe_average_precision(group["_y_true"].to_numpy(), group["_y_score"].to_numpy()),
            "precision": float(precision_score(group["_y_true"].to_numpy(), group["_y_pred"].to_numpy(), zero_division=0)),
            "recall": float(recall),
            "fpr": float(_safe_rate(fp, fp + tn)),
            "specificity": float(specificity),
            "confusion_matrix": {"tn": int(tn), "fp": int(fp), "fn": int(fn), "tp": int(tp)},
        })
    rows.sort(key=lambda r: (-r["positive_rows"], -r["rows"], r["group"]))
    return rows

def build_subset_metrics_report(meta_df: pd.DataFrame, y_true, y_score, thr: float, group_col: str, min_rows: int = 6) -> list[dict]:
    if meta_df is None or group_col not in meta_df.columns:
        return []
    frame = meta_df.copy()
    frame["_y_true"] = np.asarray(y_true).astype(int)
    frame["_y_score"] = np.asarray(y_score).astype(float)
    rows = []
    for value, group in frame.groupby(group_col, dropna=False, sort=True):
        if len(group) < min_rows:
            continue
        metrics = evaluate_binary_scores(group["_y_true"].to_numpy(), group["_y_score"].to_numpy(), thr)
        rows.append({
            "group": "<blank>" if pd.isna(value) else str(value),
            "rows": int(len(group)),
            "positive_rows": int(group["_y_true"].sum()),
            "average_precision": float(metrics.get("average_precision", 0.0)),
            "precision": float(metrics.get("precision", 0.0)),
            "recall": float(metrics.get("recall", 0.0)),
            "fpr": float(metrics.get("fpr", 0.0)),
            "confusion_matrix": metrics.get("confusion_matrix", {}),
        })
    rows.sort(key=lambda r: (-r["positive_rows"], -r["rows"], r["group"]))
    return rows


def build_group_inventory_report(df: pd.DataFrame, group_col: str) -> list[dict]:
    if df is None or group_col not in df.columns or TARGET not in df.columns:
        return []
    inv = (
        df.groupby(group_col, dropna=False)[TARGET]
        .agg(rows="size", positive_rows="sum")
        .reset_index()
        .rename(columns={group_col: "group"})
    )
    inv["negative_rows"] = inv["rows"] - inv["positive_rows"]
    inv["eligible_loto"] = ((inv["positive_rows"] > 0) & (inv["negative_rows"] > 0)).astype(int)
    inv["group"] = inv["group"].astype(str)
    return inv.sort_values(["eligible_loto", "positive_rows", "rows", "group"], ascending=[False, False, False, True]).to_dict("records")


def build_short_burst_report(meta_df: pd.DataFrame, y_true, y_score, thr: float, max_positive_run_rows: int = 3) -> dict:
    if meta_df is None or len(meta_df) == 0:
        return {}
    frame = meta_df.copy().reset_index(drop=True)
    frame["_y_true"] = np.asarray(y_true).astype(int)
    frame["_y_score"] = np.asarray(y_score).astype(float)
    session_col = _preferred_meta_col(frame, ["session_id", "section_id", "trial_id"])
    time_col = _preferred_meta_col(frame, ["frame_start_uptime_ms", "frame_end_uptime_ms", "uptime_ms", "epoch_ms"])
    if session_col is None:
        return {}
    if time_col is not None:
        frame = frame.sort_values([session_col, time_col]).reset_index(drop=True)
    short_mask = np.zeros(len(frame), dtype=bool)
    for _, group in frame.groupby(session_col, sort=False):
        labels = group["_y_true"].astype(int).to_numpy()
        if labels.size == 0:
            continue
        start = None
        idxs = group.index.to_numpy()
        for i, lab in enumerate(labels):
            if lab == 1 and start is None:
                start = i
            end_run = (lab == 0 and start is not None)
            final_row = (i == len(labels) - 1 and start is not None and lab == 1)
            if end_run or final_row:
                end = i if final_row else i - 1
                run_len = end - start + 1
                if run_len <= max_positive_run_rows:
                    short_mask[idxs[start:end + 1]] = True
                start = None
    if not short_mask.any():
        return {
            "positive_rows": 0,
            "row_count": 0,
            "max_positive_run_rows": int(max_positive_run_rows),
        }
    subset = frame.loc[short_mask]
    metrics = evaluate_binary_scores(subset["_y_true"].to_numpy(), subset["_y_score"].to_numpy(), thr)
    return {
        "row_count": int(len(subset)),
        "positive_rows": int(subset["_y_true"].sum()),
        "max_positive_run_rows": int(max_positive_run_rows),
        "average_precision": float(metrics.get("average_precision", 0.0)),
        "precision": float(metrics.get("precision", 0.0)),
        "recall": float(metrics.get("recall", 0.0)),
        "fpr": float(metrics.get("fpr", 0.0)),
        "confusion_matrix": metrics.get("confusion_matrix", {}),
    }


def _prepare_meta_binary_frame(meta_df: pd.DataFrame, y_true, y_score) -> tuple[pd.DataFrame, str, str | None]:
    frame = (meta_df.copy() if meta_df is not None else pd.DataFrame()).reset_index(drop=True)
    frame["_y_true"] = np.asarray(y_true).astype(int)
    frame["_y_score"] = np.asarray(y_score).astype(float)
    session_col = _preferred_meta_col(frame, ["session_id", "section_id", "trial_id"])
    if session_col is None:
        session_col = "_session_tmp"
        frame[session_col] = "all"
    time_col = _preferred_meta_col(frame, ["frame_start_uptime_ms", "frame_end_uptime_ms", "uptime_ms", "epoch_ms"])
    if time_col is not None:
        frame[time_col] = pd.to_numeric(frame[time_col], errors="coerce")
        frame = frame.sort_values([session_col, time_col], kind="stable").reset_index(drop=True)
    return frame, session_col, time_col


def _positive_runs(labels: np.ndarray) -> list[tuple[int, int]]:
    runs = []
    start = None
    for i, lab in enumerate(np.asarray(labels).astype(int)):
        if lab == 1 and start is None:
            start = i
        end_run = (lab == 0 and start is not None)
        final_row = (i == len(labels) - 1 and start is not None and lab == 1)
        if end_run or final_row:
            end = i if final_row else i - 1
            runs.append((int(start), int(end)))
            start = None
    return runs


def _event_windows_for_group(group: pd.DataFrame, tolerance: ArcToleranceConfig | dict | None = None) -> list[tuple[int, int]]:
    cfg = normalize_arc_tolerance_config(tolerance)
    if "arc_event_window_positive" in group.columns:
        labels = pd.to_numeric(group["arc_event_window_positive"], errors="coerce").fillna(0).astype(int).to_numpy()
        runs = _positive_runs(labels)
        if runs:
            return runs
    if "label_arc_raw" in group.columns:
        raw_labels = pd.to_numeric(group["label_arc_raw"], errors="coerce").fillna(0).astype(int).to_numpy()
    else:
        raw_labels = group["_y_true"].astype(int).to_numpy()
    base_runs = _positive_runs(raw_labels)
    if not base_runs:
        return []
    windows = []
    for start, end in base_runs:
        lo = max(0, int(start) - int(cfg.event_pre_rows()))
        hi = min(len(group) - 1, int(end) + int(cfg.event_post_rows()))
        if windows and lo <= (windows[-1][1] + 1):
            windows[-1] = (windows[-1][0], max(windows[-1][1], hi))
        else:
            windows.append((lo, hi))
    return windows


def _subset_threshold_report(frame: pd.DataFrame, mask, thr: float, *, policy: dict | None = None, extra: dict | None = None) -> dict:
    mask = np.asarray(mask).astype(bool)
    if frame is None or len(frame) == 0 or mask.size == 0 or not mask.any():
        out = {
            "row_count": 0,
            "positive_rows": 0,
            "average_precision": 0.0,
            "precision": 0.0,
            "recall": 0.0,
            "fpr": 0.0,
            "confusion_matrix": {"tn": 0, "fp": 0, "fn": 0, "tp": 0},
        }
        if extra:
            out.update(extra)
        return out
    subset = frame.loc[mask].copy()
    metrics = evaluate_binary_scores(subset["_y_true"].to_numpy(), subset["_y_score"].to_numpy(), thr)
    out = {
        "row_count": int(len(subset)),
        "positive_rows": int(subset["_y_true"].sum()),
        "average_precision": float(metrics.get("average_precision", 0.0)),
        "precision": float(metrics.get("precision", 0.0)),
        "recall": float(metrics.get("recall", 0.0)),
        "fpr": float(metrics.get("fpr", 0.0)),
        "confusion_matrix": metrics.get("confusion_matrix", {}),
    }
    if policy is not None:
        out["policy_metrics"] = evaluate_threshold_policy(subset, subset["_y_true"].to_numpy(), subset["_y_score"].to_numpy(), policy)
    if extra:
        out.update(extra)
    return out


def build_short_burst_sweep_report(meta_df: pd.DataFrame, y_true, y_score, thr: float, run_lengths=(1, 2, 3), policy: dict | None = None) -> dict:
    report = {}
    for run_len in run_lengths:
        key = f"run_le_{int(run_len)}"
        report[key] = build_short_burst_report(meta_df, y_true, y_score, thr, max_positive_run_rows=int(run_len))
        if policy is not None and report[key].get("row_count", 0) > 0:
            frame, _, _ = _prepare_meta_binary_frame(meta_df, y_true, y_score)
            burst_mask = np.zeros(len(frame), dtype=bool)
            session_col = _preferred_meta_col(frame, ["session_id", "section_id", "trial_id"]) or "_session_tmp"
            if session_col not in frame.columns:
                frame[session_col] = "all"
            time_col = _preferred_meta_col(frame, ["frame_start_uptime_ms", "frame_end_uptime_ms", "uptime_ms", "epoch_ms"])
            if time_col is not None:
                frame = frame.sort_values([session_col, time_col], kind="stable").reset_index(drop=True)
            for _, group in frame.groupby(session_col, sort=False):
                labels = group["_y_true"].astype(int).to_numpy()
                idxs = group.index.to_numpy()
                for start, end in _positive_runs(labels):
                    if (end - start + 1) <= int(run_len):
                        burst_mask[idxs[start:end + 1]] = True
            report[key]["policy_metrics"] = evaluate_threshold_policy(frame.loc[burst_mask], frame.loc[burst_mask, "_y_true"].to_numpy(), frame.loc[burst_mask, "_y_score"].to_numpy(), policy) if burst_mask.any() else {
                "accuracy": 0.0,
                "balanced_accuracy": 0.0,
                "average_precision": 0.0,
                "roc_auc": 0.0,
                "precision": 0.0,
                "recall": 0.0,
                "f1": 0.0,
                "specificity": 0.0,
                "npv": 0.0,
                "fpr": 0.0,
                "fnr": 0.0,
                "confusion_matrix": {"tn": 0, "fp": 0, "fn": 0, "tp": 0},
                "threshold_mean": 0.0,
                "threshold_min": 0.0,
                "threshold_max": 0.0,
                "row_count": 0,
                "positive_count": 0,
                "negative_count": 0,
            }
    return report


def build_transition_window_report(
    meta_df: pd.DataFrame,
    y_true,
    y_score,
    thr: float,
    *,
    policy: dict | None = None,
    edge_rows: int = 1,
    min_delta_irms: float = 0.08,
    min_delta_hf: float = 0.40,
    min_delta_flux: float = 2.0,
) -> dict:
    frame, session_col, time_col = _prepare_meta_binary_frame(meta_df, y_true, y_score)
    if len(frame) == 0:
        return {}
    i_rms = pd.to_numeric(frame.get("i_rms", pd.Series(0.0, index=frame.index)), errors="coerce").fillna(0.0)
    frame["_i_rms"] = i_rms
    frame["_delta_hf_energy"] = pd.to_numeric(frame.get("delta_hf_energy", pd.Series(0.0, index=frame.index)), errors="coerce").fillna(0.0)
    frame["_delta_flux"] = pd.to_numeric(frame.get("delta_flux", pd.Series(0.0, index=frame.index)), errors="coerce").fillna(0.0)

    sep_mask = np.zeros(len(frame), dtype=bool)
    rec_mask = np.zeros(len(frame), dtype=bool)
    comb_mask = np.zeros(len(frame), dtype=bool)

    for _, group in frame.groupby(session_col, sort=False):
        idxs = group.index.to_numpy()
        g_irms = group["_i_rms"].to_numpy(dtype=float)
        g_diff = np.diff(g_irms, prepend=g_irms[0])
        g_hf = group["_delta_hf_energy"].to_numpy(dtype=float)
        g_flux = group["_delta_flux"].to_numpy(dtype=float)

        sep_hits = np.where(g_diff <= -float(min_delta_irms))[0]
        rec_hits = np.where((g_diff >= float(min_delta_irms)) | (g_hf >= float(min_delta_hf)) | (g_flux >= float(min_delta_flux)))[0]

        for hits, mask in ((sep_hits, sep_mask), (rec_hits, rec_mask)):
            for hit in hits:
                lo = max(0, int(hit) - int(edge_rows))
                hi = min(len(idxs), int(hit) + int(edge_rows) + 1)
                mask[idxs[lo:hi]] = True
                comb_mask[idxs[lo:hi]] = True

    return {
        "edge_rows": int(edge_rows),
        "separation": _subset_threshold_report(frame, sep_mask, thr, policy=policy, extra={"subset": "separation"}),
        "recontact": _subset_threshold_report(frame, rec_mask, thr, policy=policy, extra={"subset": "recontact"}),
        "combined": _subset_threshold_report(frame, comb_mask, thr, policy=policy, extra={"subset": "combined"}),
    }


def build_unknown_context_startup_report(
    meta_df: pd.DataFrame,
    y_true,
    y_score,
    thr: float,
    *,
    policy: dict | None = None,
    startup_rows: int = 50,
    known_confidence_min: float = 0.45,
) -> dict:
    frame, session_col, time_col = _prepare_meta_binary_frame(meta_df, y_true, y_score)
    if len(frame) == 0:
        return {}
    runtime_codes = pd.to_numeric(frame.get("context_family_code_runtime", pd.Series(-1, index=frame.index)), errors="coerce").fillna(-1).astype(int)
    runtime_conf = pd.to_numeric(frame.get("context_family_confidence", pd.Series(0.0, index=frame.index)), errors="coerce").fillna(0.0).clip(0.0, 1.0)
    startup_mask = np.zeros(len(frame), dtype=bool)
    unknown_mask = np.zeros(len(frame), dtype=bool)
    for _, group in frame.groupby(session_col, sort=False):
        idxs = group.index.to_numpy()
        take = idxs[: max(1, int(startup_rows))]
        startup_mask[take] = True
        take_codes = runtime_codes.iloc[take].to_numpy(dtype=int)
        take_conf = runtime_conf.iloc[take].to_numpy(dtype=float)
        unknown_mask[take] = (take_codes < 0) | (take_conf < float(known_confidence_min))
    return {
        "startup_rows": int(startup_rows),
        "startup_all": _subset_threshold_report(frame, startup_mask, thr, policy=policy, extra={"subset": "startup_all"}),
        "unknown_only": _subset_threshold_report(frame, unknown_mask, thr, policy=policy, extra={"subset": "unknown_only", "known_confidence_min": float(known_confidence_min)}),
    }


def build_event_level_report(
    meta_df: pd.DataFrame,
    y_true,
    y_score,
    thr: float,
    *,
    policy: dict | None = None,
    tolerance: ArcToleranceConfig | dict | None = None,
) -> dict:
    frame, session_col, time_col = _prepare_meta_binary_frame(meta_df, y_true, y_score)
    if len(frame) == 0:
        return {}
    if policy is not None:
        y_pred, _ = apply_threshold_policy(frame, frame["_y_score"].to_numpy(), policy)
    else:
        y_pred = (frame["_y_score"].to_numpy() >= float(thr)).astype(int)

    true_events = 0
    detected_events = 0
    predicted_events = 0
    predicted_overlap = 0
    false_alarm_events = 0
    false_alarm_session_count = 0
    session_count = 0

    for _, group in frame.groupby(session_col, sort=False):
        session_count += 1
        gp = np.asarray(y_pred[group.index.to_numpy()], dtype=int)
        true_runs = _event_windows_for_group(group, tolerance=tolerance)
        pred_runs = _positive_runs(gp)
        true_events += len(true_runs)
        predicted_events += len(pred_runs)
        session_false_alarms = 0
        for ts, te in true_runs:
            if any(not (pe < ts or ps > te) for ps, pe in pred_runs):
                detected_events += 1
        for ps, pe in pred_runs:
            if any(not (te < ps or ts > pe) for ts, te in true_runs):
                predicted_overlap += 1
            else:
                false_alarm_events += 1
                session_false_alarms += 1
        if session_false_alarms > 0:
            false_alarm_session_count += 1

    event_recall = _safe_rate(detected_events, true_events)
    event_precision = _safe_rate(predicted_overlap, predicted_events)
    return {
        "true_event_count": int(true_events),
        "detected_event_count": int(detected_events),
        "missed_event_count": int(max(0, true_events - detected_events)),
        "predicted_event_count": int(predicted_events),
        "overlapping_predicted_event_count": int(predicted_overlap),
        "false_alarm_event_count": int(false_alarm_events),
        "false_alarm_session_count": int(false_alarm_session_count),
        "false_alarms_per_session": float(_safe_rate(false_alarm_events, max(1, session_count))),
        "event_recall": float(event_recall),
        "event_precision": float(event_precision),
        "event_f1": float(0.0 if (event_precision + event_recall) <= 0 else (2.0 * event_precision * event_recall / (event_precision + event_recall))),
    }


def strip_estimator(result: dict) -> dict:
    out = dict(result)
    out.pop("estimator", None)
    return out


def pick_winner(
    results: List[dict],
    winner_mode: str = "safety_composite",
    fn_weight: float = 80.0,
    fp_weight: float = 4.0,
):
    if not results:
        raise ValueError("No benchmark results provided.")

    policy = _selection_policy_meta(winner_mode, fn_weight, fp_weight)
    annotated = []
    for result in results:
        enriched = dict(result)
        enriched.update(
            _build_selection_meta(
                enriched,
                winner_mode=winner_mode,
                fn_weight=fn_weight,
                fp_weight=fp_weight,
            )
        )
        enriched["winner_mode"] = winner_mode
        annotated.append(enriched)

    if winner_mode == "legacy_cv_ap":
        ordered = sorted(
            annotated,
            key=lambda r: (
                -_safe_float(r.get("cv_best_average_precision")),
                _safe_float((r.get("validation_event_level_metrics") or {}).get("missed_event_count"), 1e18),
                _safe_float(r.get("validation_threshold_result", {}).get("fp"), 1e18),
                _safe_float(r.get("validation_threshold_result", {}).get("fn"), 1e18),
                _safe_float(r.get("estimated_node_count"), 1e18),
            ),
        )
    elif winner_mode == "arc_guard":
        ordered = sorted(
            annotated,
            key=lambda r: (
                -float(1.0 if r.get("threshold_constraints_met", False) else 0.0),
                _safe_float((r.get("validation_event_level_metrics") or {}).get("missed_event_count"), 1e18),
                _safe_float(r.get("validation_threshold_result", {}).get("fn"), 1e18),
                _safe_float(r.get("validation_threshold_result", {}).get("fp"), 1e18),
                _safe_float((r.get("validation_event_level_metrics") or {}).get("false_alarms_per_session"), 1e18),
                -_safe_float((r.get("validation_event_level_metrics") or {}).get("event_recall")),
                -_safe_float(r.get("holdout_validation_recall", r.get("validation_recall"))),
                -_safe_float(r.get("holdout_validation_precision", r.get("validation_precision"))),
                -_safe_float(r.get("validation_specificity")),
                -_safe_float(r.get("cv_best_average_precision")),
                _safe_float(r.get("estimated_node_count"), 1e18),
            ),
        )
    else:
        ordered = sorted(
            annotated,
            key=lambda r: (
                -_safe_float(r.get("winner_score")),
                -_safe_float((r.get("validation_event_level_metrics") or {}).get("event_recall")),
                _safe_float((r.get("validation_event_level_metrics") or {}).get("missed_event_count"), 1e18),
                -_safe_float(r.get("validation_recall")),
                -_safe_float(r.get("validation_balanced_accuracy")),
                -_safe_float(r.get("validation_average_precision")),
                -_safe_float(r.get("cv_best_average_precision")),
                _safe_float((r.get("validation_event_level_metrics") or {}).get("false_alarms_per_session"), 1e18),
                -_safe_float(r.get("validation_specificity")),
                _safe_float(r.get("estimated_node_count"), 1e18),
            ),
        )

    ranked = []
    for rank, result in enumerate(ordered, start=1):
        enriched = dict(result)
        enriched["winner_rank"] = int(rank)
        ranked.append(enriched)

    return ranked[0], ranked, policy




def _iter_ensemble_estimators(model):
    estimators = getattr(model, "estimators_", None)
    if estimators is None:
        raise ValueError("Model has no fitted estimators_ to export.")
    if isinstance(estimators, np.ndarray):
        flat = list(estimators.ravel())
    elif isinstance(estimators, (list, tuple)):
        flat = []
        for item in estimators:
            if isinstance(item, (list, tuple, np.ndarray)):
                flat.extend(list(np.asarray(item, dtype=object).ravel()))
            else:
                flat.append(item)
    else:
        flat = [estimators]
    return [est for est in flat if est is not None]



def _export_tree_ensemble_to_c(model, function_name: str = "arc_rf_predict") -> str:
    classes = np.asarray(getattr(model, "classes_", []))
    if classes.size != 2:
        raise ValueError(f"Only binary classifiers are supported for C export; got classes={classes!r}")
    pos_idx = int(np.where(classes == 1)[0][0]) if np.any(classes == 1) else int(classes.size - 1)
    neg_idx = 1 - pos_idx

    estimators = _iter_ensemble_estimators(model)
    lines = []
    lines.append("#include <stdint.h>")
    lines.append("")
    lines.append("static inline float arc_tree_predict_proba_pos(")
    lines.append("    const int16_t *left,")
    lines.append("    const int16_t *right,")
    lines.append("    const int16_t *feature,")
    lines.append("    const float *threshold,")
    lines.append("    const float *leaf_pos,")
    lines.append("    const double *input")
    lines.append(") {")
    lines.append("    int node = 0;")
    lines.append("    while (left[node] != -1) {")
    lines.append("        const int feat = feature[node];")
    lines.append("        node = (((float)input[feat]) <= threshold[node]) ? left[node] : right[node];")
    lines.append("    }")
    lines.append("    return leaf_pos[node];")
    lines.append("}")
    lines.append("")

    tree_calls = []
    for tree_idx, est in enumerate(estimators):
        tree = getattr(est, 'tree_', None)
        if tree is None:
            raise ValueError(f'Estimator {tree_idx} has no tree_ attribute.')
        left = tree.children_left.astype(int).tolist()
        right = tree.children_right.astype(int).tolist()
        feature = tree.feature.astype(int).tolist()
        threshold = tree.threshold.astype(float).tolist()
        value = np.asarray(tree.value)
        if value.ndim != 3 or value.shape[1] != 1 or value.shape[2] < 2:
            raise ValueError(f'Unexpected tree.value shape for estimator {tree_idx}: {value.shape!r}')
        leaf_pos = []
        for node_idx in range(tree.node_count):
            counts = value[node_idx, 0]
            total = float(np.sum(counts))
            if left[node_idx] == -1:
                pos = float(counts[pos_idx]) / total if total > 0.0 else 0.0
            else:
                pos = 0.0
            leaf_pos.append(pos)

        def fmt_int_list(vals):
            return ", ".join(str(int(v)) for v in vals)
        def fmt_float_list(vals):
            return ", ".join(c_float_literal(v) for v in vals)

        prefix = f"tree_{tree_idx}"
        lines.append(f"static const int16_t {prefix}_left[{len(left)}] = {{{fmt_int_list(left)}}};")
        lines.append(f"static const int16_t {prefix}_right[{len(right)}] = {{{fmt_int_list(right)}}};")
        lines.append(f"static const int16_t {prefix}_feature[{len(feature)}] = {{{fmt_int_list(feature)}}};")
        lines.append(f"static const float {prefix}_threshold[{len(threshold)}] = {{{fmt_float_list(threshold)}}};")
        lines.append(f"static const float {prefix}_leaf_pos[{len(leaf_pos)}] = {{{fmt_float_list(leaf_pos)}}};")
        lines.append("")
        tree_calls.append(
            f"    sum_pos += arc_tree_predict_proba_pos({prefix}_left, {prefix}_right, {prefix}_feature, {prefix}_threshold, {prefix}_leaf_pos, input);"
        )

    lines.append(f"static inline void {function_name}(double * input, double * output) {{")
    lines.append("    float sum_pos = 0.0f;")
    lines.extend(tree_calls)
    lines.append(f"    const float pos = sum_pos / {max(1, len(estimators))}.0f;")
    lines.append("    output[0] = (double)(1.0f - pos);")
    lines.append("    output[1] = (double)pos;")
    lines.append("}")
    lines.append("")
    return "\n".join(lines)


def _write_model_header_compat_shims(out_header: str) -> None:
    norm_out = out_header.replace("\\", "/")
    marker = "/tinyml/model/"
    if marker not in norm_out:
        return

    project_root = norm_out.split(marker, 1)[0]
    header_name = os.path.basename(out_header)
    compat_targets = {
        os.path.join(project_root, "tinyml", header_name): f"#include \"model/{header_name}\"\n",
        os.path.join(project_root, "include", header_name): f"#include \"../tinyml/model/{header_name}\"\n",
    }
    for compat_path, include_line in compat_targets.items():
        if os.path.abspath(compat_path) == os.path.abspath(out_header):
            continue
        ensure_dir(compat_path)
        with open(compat_path, "w", encoding="utf-8") as shim:
            shim.write("#pragma once\n")
            shim.write(f"// Compatibility shim that forwards to tinyml/model/{header_name}\n")
            shim.write(include_line)


def export_context_header(
    *,
    out_header: str,
    feature_names,
    class_labels,
    means,
    stds,
    centroids,
    active_class_mask=None,
    unknown_confidence_threshold: float = 0.45,
    model_name: str = "ContextPrototype",
    function_name: str = "context_family_predict",
) -> None:
    ensure_dir(out_header)
    feature_names = validate_feature_subset(
        feature_names or CONTEXT_FEATURES,
        allowed_features=CONTEXT_ALL_COMPUTED_FEATURES,
        role="context model",
    )
    feature_ids = feature_ids_for_names(feature_names)
    class_labels = [str(x) for x in class_labels]
    means = [float(x) for x in means]
    stds = [max(1e-9, float(x)) for x in stds]
    centroids = {str(k): [float(v) for v in vals] for k, vals in dict(centroids).items()}
    if active_class_mask is None:
        active_class_mask = [1] * len(class_labels)
    active_class_mask = [1 if bool(v) else 0 for v in list(active_class_mask)[:len(class_labels)]]
    if len(active_class_mask) < len(class_labels):
        active_class_mask.extend([0] * (len(class_labels) - len(active_class_mask)))

    missing_centroid_labels = [label for label in class_labels if label not in centroids]
    near_zero_centroid_labels = []
    for label in class_labels:
        arr = centroids.get(label, [0.0] * len(feature_names))
        if len(arr) == len(feature_names) and float(np.max(np.abs(np.asarray(arr, dtype=float)))) <= 1e-9:
            near_zero_centroid_labels.append(label)
    if missing_centroid_labels:
        warnings.warn(
            "Context header export is filling missing centroids with zeros for: " + ", ".join(missing_centroid_labels),
            RuntimeWarning,
        )
    warned_zero = [label for label in near_zero_centroid_labels if label not in missing_centroid_labels]
    if warned_zero:
        warnings.warn(
            "Context header export found near-zero centroids for: " + ", ".join(warned_zero),
            RuntimeWarning,
        )

    with open(out_header, "w", encoding="utf-8") as f:
        f.write("#pragma once\n")
        f.write(f"// Auto-generated context family prototype model: {model_name}\n")
        f.write("#define CONTEXT_MODEL_METADATA_VERSION 1\n")
        f.write(f"#define CONTEXT_MODEL_INPUT_DIM {len(feature_names)}\n")
        f.write(f"#define CONTEXT_MODEL_FAMILY_COUNT {len(class_labels)}\n")
        f.write(f"#define CONTEXT_UNKNOWN_CONFIDENCE {float(unknown_confidence_threshold):.4f}f\n\n")
        f.write("#define CONTEXT_FAMILY_UNKNOWN -1\n")
        for idx, label in enumerate(class_labels):
            macro = re.sub(r"[^A-Z0-9]+", "_", label.upper()).strip("_")
            f.write(f"#define CONTEXT_FAMILY_{macro} {idx}\n")
        f.write("\n// Input Feature Order:\n")
        for i, name in enumerate(feature_names):
            f.write(f"// [{i}] {name}\n")
        f.write("\n#include <math.h>\n\n")
        if missing_centroid_labels or near_zero_centroid_labels:
            f.write("// WARNING: this exported context model has incomplete class coverage.\n")
            if missing_centroid_labels:
                f.write("// Missing centroids filled with zeros: " + ", ".join(missing_centroid_labels) + "\n")
            if near_zero_centroid_labels:
                f.write("// Near-zero centroids: " + ", ".join(near_zero_centroid_labels) + "\n")
            f.write("\n")
        f.write(
            "static const int context_model_input_feature_ids[CONTEXT_MODEL_INPUT_DIM] = {"
            + ", ".join(str(int(v)) for v in feature_ids)
            + "};\n"
        )
        f.write(f"static const float context_means[{len(feature_names)}] = {{{', '.join(c_float_literal(v) for v in means)}}};\n")
        f.write(f"static const float context_stds[{len(feature_names)}] = {{{', '.join(c_float_literal(v) for v in stds)}}};\n")
        f.write("static const int context_class_active[CONTEXT_MODEL_FAMILY_COUNT] = {" + ", ".join(str(int(v)) for v in active_class_mask) + "};\n")
        centroid_symbols = []
        for label in class_labels:
            arr = centroids.get(label, [0.0] * len(feature_names))
            if len(arr) != len(feature_names):
                raise ValueError(f"Centroid length mismatch for {label}: expected {len(feature_names)}, got {len(arr)}")
            safe_label = re.sub(r"[^a-z0-9]+", "_", label.lower()).strip("_")
            symbol = f"context_centroid_{safe_label}"
            centroid_symbols.append(symbol)
            f.write(f"static const float {symbol}[{len(feature_names)}] = {{{', '.join(c_float_literal(v) for v in arr)}}};\n")
        f.write("\n")
        f.write(f"static inline void {function_name}(double *input, double *output) {{\n")
        f.write("    const float *centroids[] = {" + ", ".join(centroid_symbols) + "};\n")
        f.write(f"    float logits[{len(class_labels)}];\n")
        f.write("    float max_logit = -1e30f;\n")
        f.write(f"    for (int c = 0; c < {len(class_labels)}; ++c) {{\n")
        f.write("        if (!context_class_active[c]) { logits[c] = -1.0e9f; continue; }\n")
        f.write("        float d2 = 0.0f;\n")
        f.write(f"        for (int i = 0; i < {len(feature_names)}; ++i) {{\n")
        f.write("            const float z = (((float)input[i]) - context_means[i]) / context_stds[i];\n")
        f.write("            const float d = z - centroids[c][i];\n")
        f.write("            d2 += d * d;\n")
        f.write("        }\n")
        f.write("        logits[c] = -d2;\n")
        f.write("        if (logits[c] > max_logit) max_logit = logits[c];\n")
        f.write("    }\n")
        f.write("    float sum = 0.0f;\n")
        f.write(f"    for (int c = 0; c < {len(class_labels)}; ++c) {{\n")
        f.write("        logits[c] = expf(logits[c] - max_logit);\n")
        f.write("        sum += logits[c];\n")
        f.write("    }\n")
        f.write("    if (sum <= 1e-12f) sum = 1.0f;\n")
        f.write(f"    for (int c = 0; c < {len(class_labels)}; ++c) output[c] = (double)(logits[c] / sum);\n")
        f.write("}\n\n")
        f.write("static inline int context_family_best(double *output, double *confidence) {\n")
        f.write("    int best = 0;\n")
        f.write("    double best_v = output[0];\n")
        f.write(f"    for (int c = 1; c < {len(class_labels)}; ++c) {{ if (output[c] > best_v) {{ best_v = output[c]; best = c; }} }}\n")
        f.write("    if (confidence) *confidence = best_v;\n")
        f.write(f"    return (best_v < {float(unknown_confidence_threshold):.4f}) ? CONTEXT_FAMILY_UNKNOWN : best;\n")
        f.write("}\n")

    _write_model_header_compat_shims(out_header)


def save_context_bundle(
    *,
    report: dict,
    out_header: str,
    out_joblib: str,
    out_report: str,
    settings: dict,
) -> None:
    ensure_dir(out_joblib)
    ensure_dir(out_report)
    estimator = report.get("estimator")
    if estimator is None:
        raise ValueError("Context report must include estimator for save_context_bundle().")
    joblib.dump(estimator, out_joblib)
    export_context_header(
        out_header=out_header,
        feature_names=report.get("feature_names", CONTEXT_FEATURES),
        class_labels=report.get("class_labels", DEVICE_FAMILY_CLASSES),
        means=report.get("means", [0.0] * len(report.get("feature_names", CONTEXT_FEATURES))),
        stds=report.get("stds", [1.0] * len(report.get("feature_names", CONTEXT_FEATURES))),
        centroids=report.get("centroids", {}),
        active_class_mask=report.get("active_class_mask", None),
        unknown_confidence_threshold=float(report.get("unknown_confidence_threshold", 0.45)),
        model_name=str(report.get("model_name", "ContextPrototype")),
    )
    payload = dict(report)
    payload.pop("estimator", None)
    payload["feature_names"] = validate_feature_subset(
        payload.get("feature_names", CONTEXT_FEATURES),
        allowed_features=CONTEXT_ALL_COMPUTED_FEATURES,
        role="context model",
    )
    payload["model_input_feature_ids"] = feature_ids_for_names(payload["feature_names"])
    payload["settings"] = settings
    with open(out_report, "w", encoding="utf-8") as f:
        json.dump(payload, f, indent=2)


def export_header(
    model,
    threshold: float,
    out_header: str,
    model_name: str,
    feature_names=None,
    function_name: str = "arc_rf_predict",
    threshold_policy: dict | None = None,
) -> None:
    ensure_dir(out_header)
    export_backend = "native_tree_ensemble"
    feature_names = list(feature_names or ARC_FEATURES)
    function_name = str(function_name or "arc_rf_predict")
    arc_base_feature_names, arc_context_feature_names = split_arc_feature_names(feature_names)
    if arc_context_feature_names:
        arc_base_feature_names = validate_feature_subset(
            arc_base_feature_names,
            allowed_features=ARC_ALL_COMPUTED_FEATURES,
            role="arc base",
        )
    arc_input_feature_ids = feature_ids_for_names(feature_names)
    arc_base_feature_ids = feature_ids_for_names(arc_base_feature_names)

    model_label = str(model_name or "").strip().lower()
    use_native_export = isinstance(model, (ExtraTreesClassifier, RandomForestClassifier))

    if use_native_export:
        c_code = _export_tree_ensemble_to_c(model, function_name=function_name)
        if model_label == "extratrees":
            default_threshold = max(float(threshold), 0.26)
            default_unknown_threshold = max(default_threshold + 0.10, 0.65)
        elif model_label == "randomforest":
            default_threshold = max(float(threshold), 0.17)
            default_unknown_threshold = max(default_threshold + 0.10, 0.65)
        else:
            default_threshold = float(threshold)
            default_unknown_threshold = max(default_threshold, 0.65)
    else:
        if m2c is None:
            raise RuntimeError("m2cgen is not installed and native export is unavailable for this model.")
        raw_code = m2c.export_to_c(model)
        c_code = postprocess_m2c_header(raw_code)
        default_threshold = float(threshold)
        default_unknown_threshold = max(default_threshold, 0.65)

    family_thresholds = []
    if isinstance(threshold_policy, dict):
        fam_map = threshold_policy.get("family_thresholds") or {}
        for fam_idx in range(len(DEVICE_FAMILY_CLASSES)):
            fam = fam_map.get(str(int(fam_idx))) or {}
            family_thresholds.append(float(fam.get("threshold", default_threshold)))
    else:
        family_thresholds = [float(default_threshold)] * len(DEVICE_FAMILY_CLASSES)
    unknown_threshold = float(
        ((threshold_policy or {}).get("unknown_policy") or {}).get(
            "threshold",
            (threshold_policy or {}).get("unknown_threshold", default_unknown_threshold),
        )
    )
    known_confidence_min = float((threshold_policy or {}).get("known_confidence_min", 0.45))
    unknown_min_votes = int(((threshold_policy or {}).get("unknown_policy") or {}).get("min_positive_feature_votes", 3))

    with open(out_header, "w", encoding="utf-8") as f:
        f.write("#pragma once\n")
        f.write(f"// Auto-generated C header from scikit-learn {model_name} ({export_backend})\n")
        feature_version = 6 if arc_context_feature_names else 4
        f.write(f"#define ARC_MODEL_FEATURE_VERSION {feature_version}\n")
        f.write(f"#define ARC_MODEL_INPUT_DIM {len(feature_names)}\n")
        if arc_context_feature_names:
            f.write(f"#define ARC_MODEL_BASE_FEATURE_COUNT {len(arc_base_feature_names)}\n")
            f.write(f"#define ARC_MODEL_CONTEXT_FEATURE_COUNT {len(arc_context_feature_names)}\n")
            f.write(f"#define ARC_MODEL_CONTEXT_INPUT_OFFSET {len(arc_base_feature_names)}\n")
        f.write(f"#define ARC_THRESHOLD {float(default_threshold):.4f}f\n")
        f.write(f"#define ARC_CONTEXT_CONFIDENCE_MIN {known_confidence_min:.4f}f\n")
        f.write(f"#define ARC_THRESHOLD_UNKNOWN {unknown_threshold:.4f}f\n")
        f.write(f"#define ARC_UNKNOWN_MIN_FEATURE_VOTES {unknown_min_votes}\n\n")
        f.write("// Input Feature Order:\n")
        for i, name in enumerate(feature_names):
            f.write(f"// [{i}] {name}\n")
        f.write("\n#include <string.h>\n")
        if arc_context_feature_names:
            f.write(
                "static const int arc_model_input_feature_ids[ARC_MODEL_INPUT_DIM] = {"
                + ", ".join(str(int(v)) for v in arc_input_feature_ids)
                + "};\n"
            )
            f.write(
                "static const int arc_model_base_feature_ids[ARC_MODEL_BASE_FEATURE_COUNT] = {"
                + ", ".join(str(int(v)) for v in arc_base_feature_ids)
                + "};\n"
            )
        f.write(f"static const float arc_family_thresholds[{len(family_thresholds)}] = {{{', '.join(c_float_literal(v) for v in family_thresholds)}}};\n")
        f.write("static inline float arc_context_threshold_for_family(int family, float confidence) {\n")
        f.write("    if (family >= 0 && family < (int)(sizeof(arc_family_thresholds)/sizeof(arc_family_thresholds[0])) && confidence >= ARC_CONTEXT_CONFIDENCE_MIN) return arc_family_thresholds[family];\n")
        f.write("    return ARC_THRESHOLD_UNKNOWN;\n")
        f.write("}\n\n")
        f.write(c_code)

    _write_model_header_compat_shims(out_header)


def save_model_bundle(

    result: dict,
    out_header: str,
    out_joblib: str,
    out_report: str,
    settings: dict,
    feature_names=None,
    function_name: str = "arc_rf_predict",
) -> None:
    ensure_dir(out_joblib)
    ensure_dir(out_report)

    joblib.dump(result["estimator"], out_joblib)
    export_header(
        model=result["estimator"],
        threshold=float(result["threshold"]),
        out_header=out_header,
        model_name=result["model_name"],
        feature_names=feature_names,
        function_name=function_name,
        threshold_policy=result.get("threshold_policy"),
    )

    report = strip_estimator(result)
    report["feature_names"] = list(feature_names or ARC_FEATURES)
    report["arc_base_feature_names"], report["arc_context_feature_names"] = split_arc_feature_names(report["feature_names"])
    report["model_input_feature_ids"] = feature_ids_for_names(report["feature_names"])
    report["arc_base_feature_ids"] = feature_ids_for_names(report["arc_base_feature_names"])
    report["settings"] = settings
    with open(out_report, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)


def save_duel_bundle(
    winner: dict,
    results: List[dict],
    out_header: str,
    out_joblib: str,
    out_report: str,
    settings: dict,
    winner_policy: dict | None = None,
    feature_names=None,
    function_name: str = "arc_rf_predict",
    report_overrides: dict | None = None,
) -> None:
    ensure_dir(out_joblib)
    ensure_dir(out_report)

    joblib.dump(winner["estimator"], out_joblib)
    export_header(
        model=winner["estimator"],
        threshold=float(winner["threshold"]),
        out_header=out_header,
        model_name=winner["model_name"],
        feature_names=feature_names,
        function_name=function_name,
        threshold_policy=winner.get("threshold_policy"),
    )

    winner_feature_names = list(feature_names or ARC_FEATURES)
    winner_base_feature_names, winner_context_feature_names = split_arc_feature_names(winner_feature_names)
    report = {
        "winner": strip_estimator(winner),
        "all_results": [strip_estimator(r) for r in results],
        "feature_names": winner_feature_names,
        "arc_base_feature_names": winner_base_feature_names,
        "arc_context_feature_names": winner_context_feature_names,
        "model_input_feature_ids": feature_ids_for_names(winner_feature_names),
        "arc_base_feature_ids": feature_ids_for_names(winner_base_feature_names),
        "winner_policy": winner_policy or {},
        "settings": settings,
    }
    if report_overrides:
        report.update(report_overrides)
    with open(out_report, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)
