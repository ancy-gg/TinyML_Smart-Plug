#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

#if defined(USE_M2CGEN_RF) && !defined(DATA_COLLECTION_MODE)
  #include "TinyML_RF.h"
#endif

#ifndef ARC_THRESHOLD
  #define ARC_THRESHOLD 0.50f
#endif

static inline int ArcPredict(float cycle_nmse, float zcv, float zc_dwell_ratio,
                             float pulse_count_per_cycle, float peak_fluct_cv,
                             float midband_residual_rms, float hf_band_energy_ratio,
                             float wpe_entropy, float spec_entropy, float thd_i,
                             float v_rms, float i_rms, float temp_c) {
#if defined(USE_M2CGEN_RF) && !defined(DATA_COLLECTION_MODE)
  (void)temp_c;
  double input_features[13] = {
    (double)cycle_nmse,
    (double)zcv,
    (double)zc_dwell_ratio,
    (double)pulse_count_per_cycle,
    (double)peak_fluct_cv,
    (double)midband_residual_rms,
    (double)hf_band_energy_ratio,
    (double)wpe_entropy,
    (double)spec_entropy,
    (double)thd_i,
    (double)v_rms,
    (double)i_rms,
    (double)temp_c
  };
  double output_probs[2];
  arc_rf_predict(input_features, output_probs);
  return (output_probs[1] >= ARC_THRESHOLD) ? 1 : 0;
#else
  (void)temp_c;
  if (v_rms < FEATURE_MIN_VRMS) return 0;
  if (i_rms < ARC_MIN_IRMS_A) return 0;

  // Distorted but steady charger/SMPS guard.
  if (thd_i >= THD_STEADY_GUARD_PCT &&
      cycle_nmse < LOW_UNCERT_CYCLE_NMSE &&
      zcv < LOW_UNCERT_ZCV_MS &&
      pulse_count_per_cycle < LOW_UNCERT_PULSE_DENS) {
    return 0;
  }

  int mainScore = 0;
  int supportScore = 0;

  if (cycle_nmse >= CYCLE_NMSE_ARC_H)        mainScore++;
  if (zcv >= ZCV_ARC_H_MS)                   mainScore++;
  if (zc_dwell_ratio >= ZC_DWELL_ARC_H)      mainScore++;
  if (pulse_count_per_cycle >= PULSE_DENS_ARC_H) mainScore++;

  if (peak_fluct_cv >= PEAK_FLUCT_ARC_H)         supportScore++;
  if (midband_residual_rms >= MIDBAND_RESID_ARC_H) supportScore++;
  if (hf_band_energy_ratio >= HF_ENERGY_ARC_H)     supportScore++;
  if (wpe_entropy >= WPE_ENT_ARC_H)                supportScore++;
  if (spec_entropy >= SPEC_ENT_ARC_H)              supportScore++;

  const bool strongMain = (mainScore >= 3);
  const bool broadSupport = (supportScore >= 2);
  const bool pulsePlusSpectrum =
      (pulse_count_per_cycle >= PULSE_DENS_ARC_H) &&
      ((hf_band_energy_ratio >= HF_ENERGY_ARC_H) || (midband_residual_rms >= MIDBAND_RESID_ARC_H));

  return (strongMain && (broadSupport || pulsePlusSpectrum)) ? 1 : 0;
#endif
}
