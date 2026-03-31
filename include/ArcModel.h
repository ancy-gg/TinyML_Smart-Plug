#pragma once
#include <Arduino.h>
#include "MainConfiguration.h"
#include "TinyML_RF.h"

static inline int ArcPredict(float cycle_nmse, float zcv, float zc_dwell_ratio,
                             float pulse_count_per_cycle, float peak_fluct_cv,
                             float midband_residual_rms, float hf_band_energy_ratio,
                             float wpe_entropy, float spec_entropy, float thd_i,
                             float v_rms, float i_rms, float temp_c) {
  (void)v_rms; (void)i_rms; (void)temp_c;
  double input_features[10] = {
    (double)cycle_nmse, (double)zcv, (double)zc_dwell_ratio,
    (double)pulse_count_per_cycle, (double)peak_fluct_cv,
    (double)midband_residual_rms, (double)hf_band_energy_ratio,
    (double)wpe_entropy, (double)spec_entropy, (double)thd_i
  };
  double output_probs[2] = {0.0, 0.0};
  arc_rf_predict(input_features, output_probs);
  return (output_probs[1] >= ARC_THRESHOLD) ? 1 : 0;
}
