#pragma once
#include <Arduino.h>
#include "MainConfiguration.h"

struct ArcDetectionResult {
  float fs_hz   = FS_TARGET_HZ;
  float irms_a  = 0.0f;

  float cycle_nmse            = 0.0f;
  float zcv                   = 0.0f;
  float zc_dwell_ratio        = 0.0f;
  float cycle_rms_drop_ratio  = 0.0f;
  float peak_fluct_cv         = 0.0f;
  float midband_residual_rms  = 0.0f;
  float hf_band_energy_ratio  = 0.0f;
  float wpe_entropy           = 0.0f;
  float spec_entropy          = 0.0f;
  float dip_rebound_ratio     = 0.0f;
  float thd_i                 = 0.0f;

  bool feat_valid    = false;
  bool current_valid = false;
  uint8_t model_pred = 0;
};

class ArcDetection {
public:
  bool compute(const uint16_t* raw, size_t n, float fs_hz,
               const CurrentCalib& cal, float mainsHz,
               ArcDetectionResult& out);

  int predict(float cycle_nmse, float zcv, float zc_dwell_ratio,
              float cycle_rms_drop_ratio, float peak_fluct_cv,
              float midband_residual_rms, float hf_band_energy_ratio,
              float wpe_entropy, float spec_entropy, float dip_rebound_ratio,
              float v_rms, float i_rms, float temp_c) const;

  int computeAndPredict(const uint16_t* raw, size_t n, float fs_hz,
                        const CurrentCalib& cal, float mainsHz,
                        float v_rms, float temp_c,
                        ArcDetectionResult& out);
};
