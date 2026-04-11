#pragma once

#include <math.h>
#include <stdint.h>

enum TinyMLFeatureId : uint8_t {
  TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE = 0,
  TINYML_FEATURE_DELTA_IRMS_ABS = 1,
  TINYML_FEATURE_HALFCYCLE_ASYMMETRY = 2,
  TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY = 3,
  TINYML_FEATURE_DELTA_HF_ENERGY = 4,
  TINYML_FEATURE_DELTA_FLUX = 5,
  TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO = 6,
  TINYML_FEATURE_ZCV = 7,
  TINYML_FEATURE_SPECTRAL_FLUX_MIDHF = 8,
  TINYML_FEATURE_PEAK_FLUCT_CV = 9,
  TINYML_FEATURE_RESIDUAL_CREST_FACTOR = 10,
  TINYML_FEATURE_THD_I = 11,
  TINYML_FEATURE_HF_ENERGY_DELTA = 12,
  TINYML_FEATURE_EDGE_SPIKE_RATIO = 13,
  TINYML_FEATURE_V_SAG_PCT = 14,
  TINYML_FEATURE_CYCLE_NMSE = 15,
  TINYML_FEATURE_CTX_FAMILY_RESISTIVE_LINEAR = 16,
  TINYML_FEATURE_CTX_FAMILY_INDUCTIVE_MOTOR = 17,
  TINYML_FEATURE_CTX_FAMILY_RECTIFIER_SMPS = 18,
  TINYML_FEATURE_CTX_FAMILY_PHASE_ANGLE_CONTROLLED = 19,
  TINYML_FEATURE_CTX_FAMILY_BRUSH_UNIVERSAL_MOTOR = 20,
  TINYML_FEATURE_CTX_FAMILY_OTHER_MIXED = 21,
  TINYML_FEATURE_CONTEXT_FAMILY_CONFIDENCE = 22,
};

static constexpr uint8_t TINYML_COMPUTED_FEATURE_COUNT = 16;
static constexpr uint8_t TINYML_ARC_CONTEXT_INPUT_COUNT = 7;
static constexpr uint8_t TINYML_MODEL_INPUT_FEATURE_COUNT = 23;

// Canonical feature order shared by the trainer-exported model metadata,
// logger CSVs, and runtime/PWA adapters.
static constexpr const char* TINYML_FEATURE_NAMES[TINYML_MODEL_INPUT_FEATURE_COUNT] = {
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
  "ctx_family_resistive_linear",
  "ctx_family_inductive_motor",
  "ctx_family_rectifier_smps",
  "ctx_family_phase_angle_controlled",
  "ctx_family_brush_universal_motor",
  "ctx_family_other_mixed",
  "context_family_confidence",
};

static constexpr uint8_t TINYML_RUNTIME_EXPORT_FEATURE_IDS[TINYML_COMPUTED_FEATURE_COUNT] = {
  TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE,
  TINYML_FEATURE_DELTA_IRMS_ABS,
  TINYML_FEATURE_HALFCYCLE_ASYMMETRY,
  TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY,
  TINYML_FEATURE_DELTA_HF_ENERGY,
  TINYML_FEATURE_DELTA_FLUX,
  TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO,
  TINYML_FEATURE_ZCV,
  TINYML_FEATURE_SPECTRAL_FLUX_MIDHF,
  TINYML_FEATURE_PEAK_FLUCT_CV,
  TINYML_FEATURE_RESIDUAL_CREST_FACTOR,
  TINYML_FEATURE_THD_I,
  TINYML_FEATURE_HF_ENERGY_DELTA,
  TINYML_FEATURE_EDGE_SPIKE_RATIO,
  TINYML_FEATURE_V_SAG_PCT,
  TINYML_FEATURE_CYCLE_NMSE,
};

static constexpr uint8_t TINYML_PWA_VISIBLE_FEATURE_IDS[12] = {
  TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE,
  TINYML_FEATURE_DELTA_IRMS_ABS,
  TINYML_FEATURE_HALFCYCLE_ASYMMETRY,
  TINYML_FEATURE_SPECTRAL_FLUX_MIDHF,
  TINYML_FEATURE_PEAK_FLUCT_CV,
  TINYML_FEATURE_RESIDUAL_CREST_FACTOR,
  TINYML_FEATURE_EDGE_SPIKE_RATIO,
  TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO,
  TINYML_FEATURE_ZCV,
  TINYML_FEATURE_HF_ENERGY_DELTA,
  TINYML_FEATURE_THD_I,
  TINYML_FEATURE_V_SAG_PCT,
};

static constexpr float TINYML_FEATURE_MIN[TINYML_MODEL_INPUT_FEATURE_COUNT] = {
  0.0f,   // abs_irms_zscore_vs_baseline
  0.0f,   // delta_irms_abs
  0.0f,   // halfcycle_asymmetry
  0.0f,   // suspicious_run_energy
  0.0f,   // delta_hf_energy
  0.0f,   // delta_flux
  -80.0f, // midband_residual_ratio
  0.0f,   // zcv
  0.0f,   // spectral_flux_midhf
  0.0f,   // peak_fluct_cv
  -20.0f, // residual_crest_factor
  0.0f,   // thd_i
  -18.0f, // hf_energy_delta
  -80.0f, // edge_spike_ratio
  0.0f,   // v_sag_pct
  0.0f,   // cycle_nmse
  0.0f,   // ctx_family_resistive_linear
  0.0f,   // ctx_family_inductive_motor
  0.0f,   // ctx_family_rectifier_smps
  0.0f,   // ctx_family_phase_angle_controlled
  0.0f,   // ctx_family_brush_universal_motor
  0.0f,   // ctx_family_other_mixed
  0.0f,   // context_family_confidence
};

static constexpr float TINYML_FEATURE_MAX[TINYML_MODEL_INPUT_FEATURE_COUNT] = {
  25.0f,  // abs_irms_zscore_vs_baseline
  15.0f,  // delta_irms_abs
  200.0f, // halfcycle_asymmetry
  20.0f,  // suspicious_run_energy
  24.0f,  // delta_hf_energy
  200.0f, // delta_flux
  20.0f,  // midband_residual_ratio
  10.0f,  // zcv
  200.0f, // spectral_flux_midhf
  300.0f, // peak_fluct_cv
  40.0f,  // residual_crest_factor
  200.0f, // thd_i
  18.0f,  // hf_energy_delta
  20.0f,  // edge_spike_ratio
  100.0f, // v_sag_pct
  200.0f, // cycle_nmse
  1.0f,   // ctx_family_resistive_linear
  1.0f,   // ctx_family_inductive_motor
  1.0f,   // ctx_family_rectifier_smps
  1.0f,   // ctx_family_phase_angle_controlled
  1.0f,   // ctx_family_brush_universal_motor
  1.0f,   // ctx_family_other_mixed
  1.0f,   // context_family_confidence
};

static inline const char* tinymlFeatureNameById(int featureId) {
  if (featureId < 0 || featureId >= (int)TINYML_MODEL_INPUT_FEATURE_COUNT) return "";
  return TINYML_FEATURE_NAMES[featureId];
}

static inline float tinymlClampFeatureValue(int featureId, float value) {
  if (!isfinite(value)) return 0.0f;
  if (featureId < 0 || featureId >= (int)TINYML_MODEL_INPUT_FEATURE_COUNT) return value;
  const float lo = TINYML_FEATURE_MIN[featureId];
  const float hi = TINYML_FEATURE_MAX[featureId];
  if (value < lo) return lo;
  if (value > hi) return hi;
  return value;
}
