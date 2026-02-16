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
  uint64_t epoch_ms  = 0;   // 0 if not synced
  uint32_t uptime_ms = 0;

  float vrms   = 0;
  float irms   = 0;
  float temp_c = 0;

  float zcv_ms   = 0;
  float thd_pct  = 0;
  float entropy  = 0;      // 0..1 normalized

  uint8_t model_pred = 0;  // TinyML output (0/1)
};