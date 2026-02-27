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
                            float sf, float cyc_var,
                            float v_rms, float i_rms, float temp_c) {

#ifdef USE_M2CGEN_RF
  // NOTE: you must regenerate TinyML_RF.h to match this 10-feature order.
  double input_features[10] = {
    (double)entropy, (double)sf, (double)thd_pct, 
    (double)zcv, (double)hf_ratio, (double)hf_var,
    (double)cyc_var,
    (double)v_rms, (double)i_rms, (double)temp_c
  };
  double output_probs[2];
  arc_rf_predict(input_features, output_probs);
  return (output_probs[1] >= ARC_THRESHOLD) ? 1 : 0;

#else
  (void)thd_pct; (void)zcv; (void)v_rms; (void)temp_c;

  // Baseline (pre-trained): require HF burstiness + broadband-ness
  if (i_rms < 0.03f) return 0;

  const bool a = (entropy > ENTROPY_ARC_H);
  const bool b = (hf_ratio > 0.18f);
  const bool c = (hf_var   > 0.0025f);
  const bool d = (sf       > 0.45f);
  const bool e = (cyc_var  > 0.015f);

  return (a && b && c) || (d && e && b);
#endif
}