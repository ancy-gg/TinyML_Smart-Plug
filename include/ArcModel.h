#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

#ifdef USE_M2CGEN_RF
  #include "TinyML_RF.h"
#endif

#ifndef ARC_THRESHOLD
  #define ARC_THRESHOLD 0.50f
#endif

static inline int ArcPredict(float entropy, float thd_pct, float zcv,
                             float hf_ratio, float hf_var,
                             float v_rms, float i_rms, float temp_c) {

#ifdef USE_M2CGEN_RF
  double input_features[8] = {
    (double)entropy, (double)thd_pct, (double)zcv,
    (double)hf_ratio, (double)hf_var,
    (double)v_rms, (double)i_rms, (double)temp_c
  };
  double output_probs[2];
  arc_rf_predict(input_features, output_probs);
  return (output_probs[1] >= ARC_THRESHOLD) ? 1 : 0;
#else
  (void)thd_pct; (void)zcv; (void)v_rms; (void)temp_c;

  if (i_rms >= ARC_MIN_IRMS_A) {
    if (entropy > ENTROPY_ARC_H && hf_ratio > HF_RATIO_ARC_H && hf_var > HF_VAR_ARC_H) return 1;
  }

  return 0;
#endif
}