#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

struct ArcFeatOut {
  float fs_hz   = FS_TARGET_HZ;
  float irms_a  = 0.0f;
  float thd_pct = 0.0f;
  float entropy = 0.0f;
  float zcv_ms  = 0.0f;
  float hf_ratio = 0.0f;
  float hf_var   = 0.0f;
};

class ArcFeatures {
public:
  bool compute(const uint16_t* raw, size_t n, float fs_hz,
               const CurrentCalib& cal, float mainsHz,
               ArcFeatOut& out);
};