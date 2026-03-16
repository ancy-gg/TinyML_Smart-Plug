#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

#if defined(USE_M2CGEN_RF) && !defined(DATA_COLLECTION_MODE)
  #include "TinyML_RF.h"
#endif

#ifndef ARC_THRESHOLD
  #define ARC_THRESHOLD 0.50f
#endif

static inline int ArcPredict(float entropy, float thd_pct, float zcv,
                             float hf_ratio, float hf_var,
                             float sf, float cyc_var,
                             float v_rms, float i_rms, float temp_c) {

#if defined(USE_M2CGEN_RF) && !defined(DATA_COLLECTION_MODE)
  // 10-feature order must match DataLogger CSV + train_arc_rf.py
  double input_features[10] = {
    (double)entropy,
    (double)sf,
    (double)thd_pct,
    (double)zcv,
    (double)hf_ratio,
    (double)hf_var,
    (double)cyc_var,
    (double)v_rms,
    (double)i_rms,
    (double)temp_c
  };
  double output_probs[2];
  arc_rf_predict(input_features, output_probs);
  return (output_probs[1] >= ARC_THRESHOLD) ? 1 : 0;
#else
  (void)temp_c;

  if (v_rms < 60.0f) return 0;
  if (i_rms < ARC_MIN_IRMS_A) return 0;

  // Distorted but very steady SMPS/charger waveform guard.
  if ((thd_pct > THD_GUARD_PCT) &&
      (hf_var  < HF_VAR_GUARD_LO) &&
      (cyc_var < CYC_VAR_GUARD_LO)) {
    return 0;
  }

  const bool hfBurst = (hf_ratio > HF_RATIO_ARC_H) &&
                       (hf_var   > HF_VAR_ARC_H);

  const bool irregular = (cyc_var > CYC_VAR_ARC_H) ||
                         (zcv     > ZCV_ARC_H_MS);

  const bool broadband = (entropy > ENTROPY_ARC_H) &&
                         (sf      > SF_ARC_H);

  return (hfBurst && broadband && irregular) ? 1 : 0;
#endif
}
