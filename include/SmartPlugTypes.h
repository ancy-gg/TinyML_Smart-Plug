#pragma once
#include <Arduino.h>

enum FaultState : uint8_t {
  STATE_NORMAL = 0,
  STATE_OVERLOAD,
  STATE_HEATING,
  STATE_ARCING
};

static inline const char* stateToCstr(FaultState s) {
  switch (s) {
    case STATE_OVERLOAD: return "OVERLOAD";
    case STATE_HEATING:  return "HEATING";
    case STATE_ARCING:   return "ARCING";
    default:             return "NORMAL";
  }
}

// One unified “feature frame” that core0/core1 can fill progressively.
struct FeatureFrame {
  uint64_t epoch_ms  = 0;   // set in core1 logger tick
  uint32_t uptime_ms = 0;   // set in Core0Task when FFT frame computed

  float vrms   = 0;
  float irms   = 0;
  float temp_c = 0;

  float zcv_ms   = 0;
  float thd_pct  = 0;
  float entropy  = 0;

  // HF & robustness
  float hf_ratio = 0;       // 0..1
  float hf_var   = 0;

  // NEW
  float sf       = 0;       // spectral flatness 0..1
  float cyc_var  = 0;       // normalized cycle-to-cycle Irms variance

  uint8_t feat_valid = 0;   // 1 if this FFT frame is valid
  uint8_t model_pred = 0;
};