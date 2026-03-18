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
  double input_features[10] = {
    (double)cycle_nmse,
    (double)zcv,
    (double)zc_dwell_ratio,
    (double)pulse_count_per_cycle,
    (double)peak_fluct_cv,
    (double)midband_residual_rms,
    (double)hf_band_energy_ratio,
    (double)wpe_entropy,
    (double)spec_entropy,
    (double)thd_i
  };
  double output_probs[2];
  arc_rf_predict(input_features, output_probs);
  return (output_probs[1] >= ARC_THRESHOLD) ? 1 : 0;
#else
  (void)temp_c;
  if (v_rms < FEATURE_MIN_VRMS) return 0;
  if (i_rms < ARC_MIN_IRMS_A) return 0;

  const bool nmseH  = (cycle_nmse >= CYCLE_NMSE_ARC_H);
  const bool zcvH   = (zcv >= ZCV_ARC_H_MS);
  const bool dwellH = (zc_dwell_ratio >= ZC_DWELL_ARC_H);
  const bool pulseH = (pulse_count_per_cycle >= PULSE_DENS_ARC_H);

  const bool peakH  = (peak_fluct_cv >= PEAK_FLUCT_ARC_H);
  const bool midH   = (midband_residual_rms >= MIDBAND_RESID_ARC_H);
  const bool hfH    = (hf_band_energy_ratio >= HF_ENERGY_ARC_H);
  const bool wpeH   = (wpe_entropy >= WPE_ENT_ARC_H);
  const bool specH  = (spec_entropy >= SPEC_ENT_ARC_H);

  // Steady distorted switching loads are allowed to look "ugly" in THD
  // as long as they are not also pulse-rich and broadband-erratic.
  const bool smpsSteadyGuard =
      (thd_i >= THD_STEADY_GUARD_PCT) &&
      (zc_dwell_ratio >= SMPS_GUARD_ZC_DWELL_H) &&
      (pulse_count_per_cycle <= SMPS_GUARD_PULSE_MAX) &&
      (peak_fluct_cv <= SMPS_GUARD_PEAK_FLUCT_H) &&
      (hf_band_energy_ratio <= SMPS_GUARD_HF_MAX) &&
      (spec_entropy <= SMPS_GUARD_SPEC_MAX) &&
      (midband_residual_rms <= SMPS_GUARD_MIDBAND_MAX) &&
      (zcv <= SMPS_GUARD_ZCV_MAX_MS) &&
      (cycle_nmse <= SMPS_GUARD_NMSE_MAX);

  if (smpsSteadyGuard) return 0;

  const bool temporalAbnormal = nmseH || zcvH || peakH;
  const bool zeroCrossAbnormal = zcvH || dwellH;
  const bool broadbandAbnormal = pulseH || midH || hfH || wpeH || specH;

  int mainScore = 0;
  int supportScore = 0;

  if (nmseH)  mainScore += 2;
  if (zcvH)   mainScore += 2;
  if (dwellH) mainScore += 1;
  if (pulseH) mainScore += 2;

  if (peakH) supportScore++;
  if (midH)  supportScore++;
  if (hfH)   supportScore++;
  if (wpeH)  supportScore++;
  if (specH) supportScore++;

  const bool pulseSpectralCombo = pulseH && (midH || hfH || specH);
  const bool broadConsensus = temporalAbnormal && zeroCrossAbnormal && broadbandAbnormal;
  const bool strongWeighted = (mainScore >= 4) && (supportScore >= 2);

  return (broadConsensus && (strongWeighted || pulseSpectralCombo)) ? 1 : 0;
#endif
}
