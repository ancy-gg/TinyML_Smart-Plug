#pragma once
#include <math.h>
#include <stdint.h>

static inline float tinymlFeatureClamp_(float value, float lo, float hi) {
  if (!isfinite(value)) return 0.0f;
  return (value < lo) ? lo : ((value > hi) ? hi : value);
}

enum TinyMLFeatureId : int {
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

  TINYML_FEATURE_PULSE_COUNT_PER_CYCLE = 23,
  TINYML_FEATURE_ZERO_DWELL_RATIO = 24,
  TINYML_FEATURE_LOW_CURRENT_RATIO = 25,
  TINYML_FEATURE_MAX_LOW_CURRENT_RUN_MS = 26,
};

static constexpr uint8_t TINYML_COMPUTED_FEATURE_COUNT = 20;
static constexpr int TINYML_RUNTIME_EXPORT_FEATURE_IDS[TINYML_COMPUTED_FEATURE_COUNT] = {
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
  TINYML_FEATURE_PULSE_COUNT_PER_CYCLE,
  TINYML_FEATURE_ZERO_DWELL_RATIO,
  TINYML_FEATURE_LOW_CURRENT_RATIO,
  TINYML_FEATURE_MAX_LOW_CURRENT_RUN_MS,
};

static inline float tinymlClampFeatureValue(int featureId, float value) {
  switch (featureId) {
    case TINYML_FEATURE_SPECTRAL_FLUX_MIDHF:          return tinymlFeatureClamp_(value, 0.0f, 200.0f);
    case TINYML_FEATURE_RESIDUAL_CREST_FACTOR:        return tinymlFeatureClamp_(value, -20.0f, 40.0f);
    case TINYML_FEATURE_EDGE_SPIKE_RATIO:             return tinymlFeatureClamp_(value, -80.0f, 20.0f);
    case TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO:       return tinymlFeatureClamp_(value, -80.0f, 20.0f);
    case TINYML_FEATURE_CYCLE_NMSE:                   return tinymlFeatureClamp_(value, 0.0f, 200.0f);
    case TINYML_FEATURE_PEAK_FLUCT_CV:                return tinymlFeatureClamp_(value, 0.0f, 300.0f);
    case TINYML_FEATURE_THD_I:                        return tinymlFeatureClamp_(value, 0.0f, 200.0f);
    case TINYML_FEATURE_HF_ENERGY_DELTA:              return tinymlFeatureClamp_(value, -18.0f, 18.0f);
    case TINYML_FEATURE_ZCV:                          return tinymlFeatureClamp_(value, 0.0f, 10.0f);
    case TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE:  return tinymlFeatureClamp_(value, 0.0f, 25.0f);
    case TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY:        return tinymlFeatureClamp_(value, 0.0f, 20.0f);
    case TINYML_FEATURE_DELTA_IRMS_ABS:               return tinymlFeatureClamp_(value, 0.0f, 15.0f);
    case TINYML_FEATURE_DELTA_HF_ENERGY:              return tinymlFeatureClamp_(value, 0.0f, 24.0f);
    case TINYML_FEATURE_DELTA_FLUX:                   return tinymlFeatureClamp_(value, 0.0f, 200.0f);
    case TINYML_FEATURE_HALFCYCLE_ASYMMETRY:          return tinymlFeatureClamp_(value, 0.0f, 200.0f);
    case TINYML_FEATURE_V_SAG_PCT:                    return tinymlFeatureClamp_(value, 0.0f, 100.0f);
    case TINYML_FEATURE_CONTEXT_FAMILY_CONFIDENCE:    return tinymlFeatureClamp_(value, 0.0f, 1.0f);
    case TINYML_FEATURE_PULSE_COUNT_PER_CYCLE:        return tinymlFeatureClamp_(value, 0.0f, 16.0f);
    case TINYML_FEATURE_ZERO_DWELL_RATIO:             return tinymlFeatureClamp_(value, 0.0f, 100.0f);
    case TINYML_FEATURE_LOW_CURRENT_RATIO:            return tinymlFeatureClamp_(value, 0.0f, 100.0f);
    case TINYML_FEATURE_MAX_LOW_CURRENT_RUN_MS:       return tinymlFeatureClamp_(value, 0.0f, 25.0f);
    default:                                          return isfinite(value) ? value : 0.0f;
  }
}
