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

struct FeatureFrame {
  uint64_t epoch_ms  = 0;
  uint32_t uptime_ms = 0;

  float vrms   = 0;
  float irms   = 0;
  float temp_c = 0;

  float zcv_ms   = 0;
  float thd_pct  = 0;
  float entropy  = 0;

  // High-frequency arc discrimination
  float hf_ratio = 0; // bandpower(2k..20k) / bandpower(0..1k)
  float hf_var   = 0; // variance of hf_ratio over recent frames

  uint8_t model_pred = 0;
};