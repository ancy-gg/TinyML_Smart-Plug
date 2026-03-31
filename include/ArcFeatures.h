#pragma once
#include <Arduino.h>
#include "MainConfiguration.h"

struct ArcFeatOut {
  float fs_hz   = FS_TARGET_HZ;
  float irms_a  = 0.0f;

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

  bool feat_valid    = false;
  bool current_valid = false;
};

class ArcFeatures {
public:
  bool compute(const uint16_t* raw, size_t n, float fs_hz,
               const CurrentCalib& cal, float mainsHz,
               ArcFeatOut& out);
};
