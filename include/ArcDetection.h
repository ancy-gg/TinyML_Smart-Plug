#pragma once
#include <Arduino.h>
#include "MainConfiguration.h"

struct ArcDetectionResult {
  float fs_hz   = FS_INTENDED_HZ;
  float irms_a  = 0.0f;

  float spectral_flux_midhf         = 0.0f;
  float residual_crest_factor       = 0.0f;
  float edge_spike_ratio            = 0.0f;
  float midband_residual_ratio      = 0.0f;
  float cycle_nmse                  = 0.0f;
  float peak_fluct_cv               = 0.0f;
  float thd_i                       = 0.0f;
  float hf_energy_delta             = 0.0f;
  float zcv                         = 0.0f;
  float abs_irms_zscore_vs_baseline = 0.0f;
  float suspicious_run_energy       = 0.0f;
  float delta_irms_abs              = 0.0f;
  float delta_hf_energy             = 0.0f;
  float delta_flux                  = 0.0f;
  float halfcycle_asymmetry         = 0.0f;
  float v_sag_pct                   = 0.0f;

  bool feat_valid    = false;
  bool current_valid = false;
  uint8_t model_pred = 0;
};

class ArcDetection {
public:
  void resetRuntime();
  void setContext(int8_t family, float confidence);
  bool compute(const uint16_t* raw, size_t n, float fs_hz,
               const CurrentCalib& cal, float mainsHz,
               ArcDetectionResult& out);

  int predict(float spectral_flux_midhf, float residual_crest_factor,
              float edge_spike_ratio, float midband_residual_ratio,
              float cycle_nmse, float peak_fluct_cv,
              float thd_i, float hf_energy_delta,
              float zcv, float abs_irms_zscore_vs_baseline,
              float suspicious_run_energy, float delta_irms_abs,
              float delta_hf_energy, float delta_flux,
              float halfcycle_asymmetry, float v_sag_pct,
              float v_rms, float i_rms, float temp_c) const;

  int computeAndPredict(const uint16_t* raw, size_t n, float fs_hz,
                        const CurrentCalib& cal, float mainsHz,
                        float v_rms, float temp_c,
                        ArcDetectionResult& out);
};