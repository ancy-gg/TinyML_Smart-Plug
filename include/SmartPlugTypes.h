#pragma once
#include <Arduino.h>

enum FaultState : uint8_t {
  STATE_NORMAL = 0,
  STATE_OVERLOAD,
  STATE_UNDERVOLTAGE,
  STATE_OVERVOLTAGE,
  STATE_HEATING,
  STATE_ARCING
};

static inline const char* stateToCstr(FaultState s) {
  switch (s) {
    case STATE_OVERLOAD:      return "OVERLOAD";
    case STATE_UNDERVOLTAGE:  return "UNDERVOLTAGE";
    case STATE_OVERVOLTAGE:   return "OVERVOLTAGE";
    case STATE_HEATING:       return "HEATING";
    case STATE_ARCING:        return "ARCING";
    default:                  return "NORMAL";
  }
}

struct FeatureFrame {
  uint64_t epoch_ms  = 0;
  uint32_t uptime_ms = 0;

  // context
  float vrms   = 0.0f;
  float irms   = 0.0f;
  float temp_c = 0.0f;

  // exact main 10-feature order
  float cycle_nmse            = 0.0f;
  float zcv                   = 0.0f;
  float zc_dwell_ratio        = 0.0f;
  float pulse_count_per_cycle = 0.0f;
  float peak_fluct_cv         = 0.0f;
  float midband_residual_rms  = 0.0f;
  float hf_band_energy_ratio  = 0.0f;
  float wpe_entropy           = 0.0f;
  float spec_entropy          = 0.0f;
  float thd_i                 = 0.0f;

  float adc_fs_hz = 0.0f;

  uint8_t feat_valid    = 0;
  uint8_t current_valid = 0;
  uint8_t model_pred    = 0;
};
