import json
import os
from dataclasses import dataclass
from typing import Dict, List, Tuple

import joblib
import m2cgen as m2c
import numpy as np
import pandas as pd

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
from sklearn.model_selection import GroupKFold, GroupShuffleSplit, RandomizedSearchCV


FEATURES = [
    "cycle_nmse",
    "zcv",
    "zc_dwell_ratio",
    "cycle_rms_drop_ratio",
    "peak_fluct_cv",
    "midband_residual_rms",
    "hf_band_energy_ratio",
    "spec_entropy",
    "neg_dip_event_ratio",
    "irms_drop_vs_baseline",
]
TARGET = "label_arc"
GROUP_COL_CANDIDATES = ["session_id", "session", "sid"]


MODEL_CONFIGS = {
    "rf": {
        "pretty_name": "RandomForest",
        "builder": lambda: RandomForestClassifier(
            random_state=42,
            class_weight="balanced_subsample",
            n_jobs=-1,
            bootstrap=True,
            oob_score=False,
        ),
        "param_dist": {
            "n_estimators": [40, 60, 80, 120],
            "max_depth": [6, 8, 10, 12, 14, None],
            "min_samples_leaf": [1, 2, 4, 6],
            "min_samples_split": [2, 4, 8, 12],
            "max_features": ["sqrt", 0.6, 0.8],
            "bootstrap": [True],
            "ccp_alpha": [0.0, 0.0002, 0.0005, 0.001],
        },
    },
    "et": {
        "pretty_name": "ExtraTrees",
        "builder": lambda: ExtraTreesClassifier(
            random_state=42,
            class_weight="balanced",
            n_jobs=-1,
            bootstrap=False,
        ),
        "param_dist": {
            "n_estimators": [40, 60, 80, 120],
            "max_depth": [6, 8, 10, 12, 14, None],
            "min_samples_leaf": [1, 2, 4, 6],
            "min_samples_split": [2, 4, 8, 12],
            "max_features": ["sqrt", 0.6, 0.8],
            "bootstrap": [False],
            "ccp_alpha": [0.0, 0.0002, 0.0005, 0.001],
        },
    },
}


def ensure_dir(path: str) -> None:
    folder = os.path.dirname(path)
    if folder:
        os.makedirs(folder, exist_ok=True)


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
        "session_id": "session_id",
        "epoch_ms": "epoch_ms",
        "adc_fs_hz": "adc_fs_hz",
        "feat_valid": "feat_valid",
        "current_valid": "current_valid",
        "rf_train_row": "rf_train_row",
        "sample_weight": "sample_weight",
        "cycle_nmse": "cycle_nmse",
        "zcv": "zcv",
        "zc_dwell_ratio": "zc_dwell_ratio",
        "cycle_rms_drop_ratio": "cycle_rms_drop_ratio",
        "peak_fluct_cv": "peak_fluct_cv",
        "midband_residual_rms": "midband_residual_rms",
        "hf_band_energy_ratio": "hf_band_energy_ratio",
        "spec_entropy": "spec_entropy",
        "neg_dip_event_ratio": "neg_dip_event_ratio",
        "irms_drop_vs_baseline": "irms_drop_vs_baseline",
        "pre_dip_spike_ratio": "irms_drop_vs_baseline",
        "thd_i": "thd_i",
        "pulse_count_per_cycle": "pulse_count_per_cycle",
    }

    rename_map = {
        c: aliases[str(c).strip()]
        for c in df.columns
        if str(c).strip() in aliases
    }
    if rename_map:
        df = df.rename(columns=rename_map)

    if (
        "cycle_rms_drop_ratio" not in df.columns
        and "pulse_count_per_cycle" in df.columns
    ):
        df["cycle_rms_drop_ratio"] = pd.to_numeric(
            df["pulse_count_per_cycle"],
            errors="coerce",
        )

    for missing in ["neg_dip_event_ratio", "irms_drop_vs_baseline", "thd_i"]:
        if missing not in df.columns:
            df[missing] = 0.0

    return df


def clean_df(df: pd.DataFrame, include_invalid: bool = False) -> pd.DataFrame:
    df = normalize_feature_names(df.copy())

    missing = [c for c in FEATURES + [TARGET] if c not in df.columns]
    if missing:
        raise ValueError(f"Missing required columns: {missing}")

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
            curv = pd.to_numeric(
                df.get("current_valid", 1),
                errors="coerce",
            ).fillna(1)
            df = df[
                (pd.to_numeric(df["feat_valid"], errors="coerce") == 1)
                & (curv == 1)
            ].copy()

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
        "spec_entropy": 1.0,
        "neg_dip_event_ratio": 1.0,
        "irms_drop_vs_baseline": 2.0,
    }
    for c, hi in clip_hi.items():
        df[c] = df[c].clip(0.0, hi)

    if "sample_weight" in df.columns:
        df["sample_weight"] = pd.to_numeric(
            df["sample_weight"],
            errors="coerce",
        ).fillna(1.0)
        df["sample_weight"] = df["sample_weight"].clip(0.05, 100.0)
    else:
        df["sample_weight"] = 1.0

    df[TARGET] = df[TARGET].astype(int)
    return df


def select_threshold_cost(
    y_true: np.ndarray,
    y_proba: np.ndarray,
    fn_weight: float = 80.0,
    fp_weight: float = 4.0,
    min_recall: float = 0.97,
) -> dict:
    thresholds = np.arange(0.05, 0.96, 0.01)
    best = {"thr": 0.5, "cost": float("inf")}

    y_true = np.asarray(y_true).astype(int)
    y_proba = np.asarray(y_proba).astype(float)

    for thr in thresholds:
        y_pred = (y_proba >= thr).astype(int)
        tn, fp, fn, tp = confusion_matrix(
            y_true,
            y_pred,
            labels=[0, 1],
        ).ravel()

        recall = tp / max(1, tp + fn)
        if recall < min_recall:
            continue

        cost = fn_weight * fn + fp_weight * fp
        if cost < best["cost"]:
            best = {
                "thr": float(thr),
                "cost": float(cost),
                "tn": int(tn),
                "fp": int(fp),
                "fn": int(fn),
                "tp": int(tp),
                "recall": float(recall),
            }

    if best["cost"] == float("inf"):
        best = {"thr": 0.5, "cost": float("inf")}
        for thr in thresholds:
            y_pred = (y_proba >= thr).astype(int)
            tn, fp, fn, tp = confusion_matrix(
                y_true,
                y_pred,
                labels=[0, 1],
            ).ravel()

            cost = fn_weight * fn + fp_weight * fp
            if cost < best["cost"]:
                best = {
                    "thr": float(thr),
                    "cost": float(cost),
                    "tn": int(tn),
                    "fp": int(fp),
                    "fn": int(fn),
                    "tp": int(tp),
                    "recall": float(tp / max(1, tp + fn)),
                }
    return best


def model_size_estimate(model) -> int:
    return int(sum(est.tree_.node_count for est in model.estimators_))




def _safe_float(value, default: float = 0.0) -> float:
    try:
        out = float(value)
        if np.isnan(out) or np.isinf(out):
            return float(default)
        return out
    except Exception:
        return float(default)



def _safe_rate(num: float, den: float) -> float:
    return float(num) / max(1.0, float(den))



def _safe_average_precision(y_true, y_score) -> float:
    try:
        return float(average_precision_score(y_true, y_score))
    except Exception:
        return 0.0



def _safe_roc_auc(y_true, y_score) -> float:
    try:
        return float(roc_auc_score(y_true, y_score))
    except Exception:
        return 0.0



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
    except Exception:
        mcc = 0.0

    out = {
        "threshold": float(threshold),
        "accuracy": float(accuracy_score(y_true, y_pred)),
        "balanced_accuracy": float(balanced_accuracy_score(y_true, y_pred)),
        "average_precision": _safe_average_precision(y_true, y_score),
        "roc_auc": _safe_roc_auc(y_true, y_score),
        "precision": float(precision_score(y_true, y_pred, zero_division=0)),
        "recall": float(recall_score(y_true, y_pred, zero_division=0)),
        "f1": float(f1_score(y_true, y_pred, zero_division=0)),
        "specificity": _safe_rate(tn, tn + fp),
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

    return {
        "mode": winner_mode,
        "description": (
            "Composite winner score using cross-validation AP plus validation-set "
            "average precision, recall, balanced accuracy, F1, specificity, ROC AUC, "
            "MCC, validation threshold cost penalty, and model size penalty. "
            "Test metrics are intentionally excluded from winner selection."
        ),
        "uses_test_metrics_for_selection": False,
        "weights": {
            "cv_best_average_precision": 0.18,
            "validation_average_precision": 0.20,
            "validation_recall": 0.20,
            "validation_balanced_accuracy": 0.15,
            "validation_f1": 0.10,
            "validation_specificity": 0.06,
            "validation_roc_auc": 0.04,
            "validation_mcc_norm": 0.03,
            "validation_cost_penalty": -0.03,
            "node_penalty": -0.01,
        },
        "cost_context": {
            "fn_weight": float(fn_weight),
            "fp_weight": float(fp_weight),
            "cost_unit": float(max(1.0, fn_weight + (5.0 * fp_weight))),
        },
        "tie_break_order": [
            "winner_score",
            "validation_recall",
            "validation_balanced_accuracy",
            "validation_average_precision",
            "cv_best_average_precision",
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
    val_bal_acc = _safe_float(val.get("balanced_accuracy"))
    val_f1 = _safe_float(val.get("f1"))
    val_spec = _safe_float(val.get("specificity"))
    val_roc_auc = _safe_float(val.get("roc_auc"))
    val_mcc_norm = (_safe_float(val.get("mcc")) + 1.0) / 2.0
    val_cost = _safe_float(result.get("validation_threshold_result", {}).get("cost"))
    node_count = _safe_float(result.get("estimated_node_count"))

    cost_unit = max(1.0, fn_weight + (5.0 * fp_weight))
    val_cost_penalty = float(np.tanh(val_cost / cost_unit))
    node_penalty = min(1.0, node_count / 20000.0)

    components = {
        "cv_best_average_precision": 0.18 * cv_ap,
        "validation_average_precision": 0.20 * val_ap,
        "validation_recall": 0.20 * val_recall,
        "validation_balanced_accuracy": 0.15 * val_bal_acc,
        "validation_f1": 0.10 * val_f1,
        "validation_specificity": 0.06 * val_spec,
        "validation_roc_auc": 0.04 * val_roc_auc,
        "validation_mcc_norm": 0.03 * val_mcc_norm,
        "validation_cost_penalty": -0.03 * val_cost_penalty,
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
    return c_code


def load_clean_dataset(csv_path: str, include_invalid: bool = False):
    if not os.path.isfile(csv_path):
        raise ValueError(f"Merged dataset not found: {csv_path}")

    raw_df = pd.read_csv(csv_path)
    raw_df = normalize_feature_names(raw_df)
    df = clean_df(raw_df, include_invalid=include_invalid)

    if df.empty or df[TARGET].nunique() < 2:
        raise ValueError("Training requires both classes in cleaned_data.csv.")

    group_col = pick_group_column(df)
    if group_col is None:
        df["_group"] = np.arange(len(df)) // 200
        group_col = "_group"

    groups = df[group_col].astype(str)
    X = df[FEATURES].astype(float)
    y = df[TARGET].astype(int)
    w = df["sample_weight"].astype(float).to_numpy()
    return df, X, y, groups, w


def make_group_splits(X, y, groups, test_size=0.20):
    gss_test = GroupShuffleSplit(
        n_splits=1,
        test_size=test_size,
        random_state=42,
    )
    train_full_idx, test_idx = next(gss_test.split(X, y, groups=groups))

    X_train_full = X.iloc[train_full_idx]
    y_train_full = y.iloc[train_full_idx]
    groups_train_full = groups.iloc[train_full_idx]

    X_test = X.iloc[test_idx]
    y_test = y.iloc[test_idx]

    gss_val = GroupShuffleSplit(
        n_splits=1,
        test_size=0.20,
        random_state=123,
    )
    train_idx, val_idx = next(
        gss_val.split(
            X_train_full,
            y_train_full,
            groups=groups_train_full,
        )
    )

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
    }


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
) -> dict:
    if model_key not in MODEL_CONFIGS:
        raise ValueError(f"Unsupported model: {model_key}")

    cfg = MODEL_CONFIGS[model_key]
    pretty = cfg["pretty_name"]
    base = cfg["builder"]()
    param_dist = cfg["param_dist"]

    n_splits = min(5, groups_train.nunique())
    if n_splits < 2:
        raise ValueError("Need at least 2 distinct sessions in cleaned_data.csv.")

    cv = GroupKFold(n_splits=n_splits)
    search = RandomizedSearchCV(
        base,
        param_dist,
        n_iter=n_iter,
        scoring="average_precision",
        cv=cv,
        n_jobs=-1,
        verbose=1,
        random_state=42,
    )
    search.fit(
        X_train,
        y_train,
        groups=groups_train,
        sample_weight=w_train,
    )
    best = search.best_estimator_

    best.fit(X_train, y_train, sample_weight=w_train)
    val_proba = best.predict_proba(X_val)[:, 1]
    best_thr = select_threshold_cost(
        y_val.to_numpy(),
        val_proba,
        fn_weight=fn_weight,
        fp_weight=fp_weight,
        min_recall=min_recall,
    )
    thr = float(best_thr["thr"])

    val_metrics = evaluate_binary_scores(y_val.to_numpy(), val_proba, thr)
    y_score = best.predict_proba(X_test)[:, 1]
    test_metrics = evaluate_binary_scores(y_test, y_score, thr)

    fi = pd.Series(
        best.feature_importances_,
        index=FEATURES,
    ).sort_values(ascending=False)

    result = {
        "model_key": model_key,
        "model_name": pretty,
        "best_params": search.best_params_,
        "cv_best_average_precision": float(search.best_score_),
        "estimated_node_count": model_size_estimate(best),
        "threshold": thr,
        "validation_threshold_result": best_thr,
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
        "split_sizes": {
            "train_rows": int(len(X_train)),
            "validation_rows": int(len(X_val)),
            "val_rows": int(len(X_val)),
            "test_rows": int(len(X_test)),
            "train_groups": int(groups_train.nunique()),
            "train_positive": int(np.sum(np.asarray(y_train) == 1)),
            "validation_positive": int(np.sum(np.asarray(y_val) == 1)),
            "test_positive": int(np.sum(np.asarray(y_test) == 1)),
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
                _safe_float(r.get("validation_threshold_result", {}).get("fp"), 1e18),
                _safe_float(r.get("validation_threshold_result", {}).get("fn"), 1e18),
                _safe_float(r.get("estimated_node_count"), 1e18),
            ),
        )
    else:
        ordered = sorted(
            annotated,
            key=lambda r: (
                -_safe_float(r.get("winner_score")),
                -_safe_float(r.get("validation_recall")),
                -_safe_float(r.get("validation_balanced_accuracy")),
                -_safe_float(r.get("validation_average_precision")),
                -_safe_float(r.get("cv_best_average_precision")),
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


def export_header(
    model,
    threshold: float,
    out_header: str,
    model_name: str,
) -> None:
    ensure_dir(out_header)
    c_code = m2c.export_to_c(model, function_name="arc_rf_predict")
    c_code = postprocess_m2c_header(c_code)

    with open(out_header, "w", encoding="utf-8") as f:
        f.write("#pragma once\n")
        f.write(
            f"// Auto-generated by m2cgen from scikit-learn {model_name}\n"
        )
        f.write("#define ARC_MODEL_FEATURE_VERSION 2\n")
        f.write(f"#define ARC_THRESHOLD {threshold:.4f}\n\n")
        f.write("// Input Feature Order:\n")
        for i, name in enumerate(FEATURES):
            f.write(f"// [{i}] {name}\n")
        f.write("\n#include <string.h>\n")
        f.write(c_code)


def save_model_bundle(
    result: dict,
    out_header: str,
    out_joblib: str,
    out_report: str,
    settings: dict,
) -> None:
    ensure_dir(out_joblib)
    ensure_dir(out_report)

    joblib.dump(result["estimator"], out_joblib)
    export_header(
        model=result["estimator"],
        threshold=float(result["threshold"]),
        out_header=out_header,
        model_name=result["model_name"],
    )

    report = strip_estimator(result)
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
) -> None:
    ensure_dir(out_joblib)
    ensure_dir(out_report)

    joblib.dump(winner["estimator"], out_joblib)
    export_header(
        model=winner["estimator"],
        threshold=float(winner["threshold"]),
        out_header=out_header,
        model_name=winner["model_name"],
    )

    report = {
        "winner": strip_estimator(winner),
        "all_results": [strip_estimator(r) for r in results],
        "winner_policy": winner_policy or {},
        "settings": settings,
    }
    with open(out_report, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)
