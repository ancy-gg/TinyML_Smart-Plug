#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

struct ArcFeatOut {
  float fs_hz   = FS_TARGET_HZ;
  float irms_a  = 0.0f;
  float thd_pct = 0.0f;
  float entropy = 0.0f;  // 0..1
  float zcv_ms  = 0.0f;  // ms
};

class ArcFeatures {
public:
  bool compute(const uint16_t* raw, size_t n, float fs_hz,
               const CurrentCalib& cal, float mainsHz,
               ArcFeatOut& out);
};