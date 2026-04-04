import argparse
import json
import glob
import os
import re
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
TRUSTED_NORMAL_WEIGHT = 6.0

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



def _normalized_name(path: str) -> str:
    stem = os.path.splitext(os.path.basename(path))[0].lower()
    return re.sub(r"[^a-z0-9]+", "_", stem)



def _has_any_name_token(path: str, tokens) -> bool:
    name = _normalized_name(path)
    return any(tok in name for tok in tokens)



def infer_source_kind(path: str):
    name = os.path.basename(path).lower()
    if "scaffold" in name:
        return "scaffold", 1, 0
    if _has_any_name_token(path, TRUSTED_NORMAL_NAME_TOKENS):
        return "real", 4, 1
    return "real", 3, 0



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

        source_kind, source_priority, trusted_normal_session = infer_source_kind(path)
        df["_source_file"] = os.path.basename(path)
        df["source_kind"] = source_kind
        df["source_priority"] = source_priority
        df["trusted_normal_session"] = int(trusted_normal_session)
        if "rf_train_row" not in df.columns:
            df["rf_train_row"] = 1
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
    neg = _clip_score(_num(df, "neg_dip_event_ratio"), 0.040)
    base_drop = _clip_score(_num(df, "irms_drop_vs_baseline"), 0.055)
    cyc_drop = _clip_score(_num(df, "cycle_rms_drop_ratio"), 0.025)
    peak = _clip_score(_num(df, "peak_fluct_cv"), 0.012)
    mid = _clip_score(_num(df, "midband_residual_rms"), 0.055)
    nmse = _clip_score(_num(df, "cycle_nmse"), 0.090)
    zcv = _clip_score(_num(df, "zcv"), 0.20)
    hf = _clip_score(_num(df, "hf_band_energy_ratio"), 0.30)
    ent = _clip_score(_num(df, "spec_entropy"), 0.65)

    arc_base = (
        0.24 * neg
        + 0.20 * base_drop
        + 0.18 * cyc_drop
        + 0.12 * mid
        + 0.10 * nmse
        + 0.08 * peak
        + 0.04 * zcv
        + 0.02 * hf
        + 0.02 * ent
    )

    if "session_id" in df.columns:
        order_cols = ["session_id"]
        if "epoch_ms" in df.columns:
            order_cols.append("epoch_ms")
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

    arc_like = (0.72 * arc_base) + (0.28 * arc_cluster.clip(0.0, 1.5))
    arc_like = (arc_like * (0.85 + 0.15 * feat_valid) * (0.90 + 0.10 * cur_valid)).clip(0.0, 1.5)

    normal_like = (
        (1.0 - arc_like.clip(0.0, 1.0)) * 0.70
        + 0.12 * feat_valid
        + 0.10 * cur_valid
        + 0.10 * v_normal
        + 0.08 * i_active
        + 0.35 * trusted
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

    # Explicit trusted-normal files remain strong negatives.
    df.loc[df["trusted_normal_session"] == 1, TARGET] = 0

    df["label_trust"] = _build_label_trust(df)
    df["arc_like_score"], df["normal_like_score"] = build_quality_scores(df)
    df["clean_reason"] = "kept"
    df["clean_quality"] = "trainable"

    invalid_or_off = (
        _num(df, "i_rms") < 0.05
    ) | (
        _num(df, "v_rms") < 170.0
    )
    if "feat_valid" in df.columns:
        invalid_or_off |= (_num(df, "feat_valid") < 1.0)
    if "current_valid" in df.columns:
        invalid_or_off |= (_num(df, "current_valid") < 1.0)
    if "adc_fs_hz" in df.columns:
        invalid_or_off |= (_num(df, "adc_fs_hz") <= 0.0)

    df.loc[invalid_or_off, "rf_train_row"] = 0
    df.loc[invalid_or_off, "clean_reason"] = "invalid_or_off"
    df.loc[invalid_or_off, "clean_quality"] = "excluded_invalid"

    positive_mask = (df[TARGET].astype(int) == 1) & (df["rf_train_row"] == 1)
    very_weak_positive = positive_mask & (df["arc_like_score"] < VERY_WEAK_POSITIVE_SCORE)
    soft_positive = positive_mask & ~very_weak_positive & (df["arc_like_score"] < SOFT_POSITIVE_SCORE)

    df.loc[very_weak_positive, "rf_train_row"] = 0
    df.loc[very_weak_positive, "clean_reason"] = "very_weak_positive_label"
    df.loc[very_weak_positive, "clean_quality"] = "excluded_weak_positive"

    df.loc[soft_positive, "label_trust"] = np.minimum(df.loc[soft_positive, "label_trust"], 0.30)
    df.loc[soft_positive, "clean_reason"] = "soft_positive_low_trust"
    df.loc[soft_positive, "clean_quality"] = "soft_positive"

    negative_mask = (df[TARGET].astype(int) == 0) & (df["rf_train_row"] == 1) & (df["trusted_normal_session"] == 0)
    strong_arc_like_negative = negative_mask & (df["arc_like_score"] >= STRONG_NEGATIVE_ARC_SCORE)
    soft_arc_like_negative = negative_mask & ~strong_arc_like_negative & (df["arc_like_score"] >= SOFT_NEGATIVE_ARC_SCORE)

    df.loc[strong_arc_like_negative, "rf_train_row"] = 0
    df.loc[strong_arc_like_negative, "clean_reason"] = "strong_arc_like_negative"
    df.loc[strong_arc_like_negative, "clean_quality"] = "excluded_arc_like_negative"

    df.loc[soft_arc_like_negative, "label_trust"] = np.minimum(df.loc[soft_arc_like_negative, "label_trust"], 0.55)
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
        sort_cols = ["session_id", "epoch_ms"] if "epoch_ms" in df.columns else ["session_id"]
        df = df.sort_values(sort_cols).copy()
        di = df.groupby("session_id")["i_rms"].diff().abs().fillna(0.0)
        turn_like = (di >= 0.60) | (
            (_num(df, "i_rms") >= 0.10) & (_num(df, "i_rms") <= 0.50)
        )
        transient_norm = (
            (df[TARGET] == 0)
            & (df["rf_train_row"] == 1)
            & turn_like
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

    sig_cols = [c for c in FEATURES if c in df.columns]
    rounded_exact = df[sig_cols].round(4).astype(str)
    rounded_coarse = df[sig_cols].round(CONFLICT_ROUND_DECIMALS).astype(str)
    df["_feature_sig"] = rounded_exact.agg("|".join, axis=1)
    df["_conflict_sig"] = rounded_coarse.agg("|".join, axis=1)

    quality_summary = {
        "invalid_or_off_rows": int(invalid_or_off.sum()),
        "very_weak_positive_rows": int(very_weak_positive.sum()),
        "soft_positive_rows": int(soft_positive.sum()),
        "strong_arc_like_negative_rows": int(strong_arc_like_negative.sum()),
        "soft_arc_like_negative_rows": int(soft_arc_like_negative.sum()),
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
    for _, group in df.groupby("_conflict_sig", sort=False, dropna=False):
        stats["groups_seen"] += 1
        g = group.copy()
        g[TARGET] = pd.to_numeric(g[TARGET], errors="coerce").fillna(0).astype(int)
        g["rf_train_row"] = pd.to_numeric(g["rf_train_row"], errors="coerce").fillna(0).astype(int)
        g["label_trust"] = pd.to_numeric(g["label_trust"], errors="coerce").fillna(0.0)
        g["source_priority"] = pd.to_numeric(g["source_priority"], errors="coerce").fillna(0)
        g["sample_weight"] = pd.to_numeric(g["sample_weight"], errors="coerce").fillna(0.0)
        g["trusted_normal_session"] = pd.to_numeric(g.get("trusted_normal_session", 0), errors="coerce").fillna(0).astype(int)
        g["arc_like_score"] = pd.to_numeric(g.get("arc_like_score", 0.0), errors="coerce").fillna(0.0)
        g["normal_like_score"] = pd.to_numeric(g.get("normal_like_score", 0.0), errors="coerce").fillna(0.0)

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



def clean_dataset(df: pd.DataFrame) -> tuple[pd.DataFrame, dict]:
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
        "trusted_normal_session",
        "label_trust",
        "sample_weight",
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

    df, quality_summary = attach_training_metadata(df)
    df, conflict_stats = resolve_near_duplicate_label_conflicts(df)

    # Final exact dedupe after conflict handling. Ignore the label here so exact
    # feature collisions do not keep both a 0 and a 1 copy.
    df = df.sort_values(
        ["rf_train_row", "label_trust", "source_priority", "sample_weight"],
        ascending=[False, False, False, False],
    ).drop_duplicates(subset=FEATURES, keep="first").copy()

    df[TARGET] = pd.to_numeric(df[TARGET], errors="coerce").fillna(0).astype(int)
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
    df["label_conflict"] = pd.to_numeric(df.get("label_conflict", 0), errors="coerce").fillna(0).astype(int)

    print("Rows before cleaning:", before)
    print("Rows after cleaning :", len(df))
    print("Rows removed        :", before - len(df))
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
        "rows_after_cleaning": int(len(df)),
        "rows_removed": int(before - len(df)),
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
        "quality_summary": {str(k): int(v) for k, v in quality_summary.items()},
        "conflict_resolution": {str(k): int(v) for k, v in conflict_stats.items()},
    }
    return df, summary



def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv_glob", default=DEFAULT_INPUT_GLOB)
    ap.add_argument("--output", default=DEFAULT_OUTPUT)
    args = ap.parse_args()

    csv_files = resolve_csv_files(args.csv_glob, args.output)
    df = load_and_tag_csvs(csv_files)
    df, summary = clean_dataset(df)
    ensure_dir(args.output)
    df.to_csv(args.output, index=False)
    print("Saved merged dataset to:", args.output)
    print("__CLEANER_SUMMARY__ " + json.dumps(summary, sort_keys=True))


if __name__ == "__main__":
    main()
