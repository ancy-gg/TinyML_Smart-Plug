#pragma once
#include <Arduino.h>
#include <math.h>

// =========================
// Current capture backend
// =========================
#define CUR_BACKEND_ADS8684 0
#define CUR_BACKEND_MCP3204 1

#ifndef CURRENT_CAPTURE_BACKEND
#define CURRENT_CAPTURE_BACKEND CUR_BACKEND_MCP3204
#endif

static inline const char* currentBackendName() {
#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684
  return "ADS8684";
#else
  return "MCP3204";
#endif
}

// =========================
// Backend acquisition config
// =========================
static constexpr float ADS_VREF_V = 4.096f;
static constexpr int   ADS_SPI_HZ = 8000000;
static constexpr int   ADS_SPI_FALLBACK_HZ = 4000000;
static constexpr uint8_t ADS_CAPTURE_OVERSAMPLE = 1;
static constexpr size_t ADS_BURST_OUTPUT_CHUNK_SAMPLES = 384;

static constexpr uint8_t  MCP3204_CHANNEL         = 0;
static constexpr uint32_t MCP3204_SPI_HZ          = 2000000UL;
static constexpr uint8_t  MCP3204_OVERSAMPLE      = 1;
static constexpr uint8_t  MCP3204_MEDIAN_SAMPLES  = 1;
static constexpr uint16_t MCP3204_STARTUP_FLUSH   = 256;
static constexpr uint8_t  MCP3204_WARMUP_BURSTS   = 2;
static constexpr uint8_t  MCP3204_BURST_FLUSH     = 4;

static constexpr bool  MCP3204_MM3_HYBRID_FILTER_ENABLE = false;
static constexpr uint8_t MCP3204_MM3_MEDIAN_WEIGHT = 2; // out=(2*median + mean)/3

// =========================
// Backend-specific profiles
// =========================
#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684

static constexpr float IRMS_GATE_ON_A               = 0.020f;
static constexpr float IRMS_GATE_OFF_A              = 0.012f;
static constexpr float CURRENT_IDLE_SUPPRESS_A      = 0.020f;
static constexpr float FEATURE_MIN_VRMS             = 70.0f;
static constexpr float FEATURE_MIN_IRMS_A           = 0.020f;
static constexpr float ARC_MIN_IRMS_A               = 0.20f;
static constexpr float FEATURE_REQUIRE_FUND_BELOW_A = 0.060f;

static constexpr float CURRENT_LPF_HZ               = 4200.0f;
static constexpr float CURRENT_BASE_LPF_HZ          = 650.0f;

static constexpr float ZC_HYS_MIN_A                 = 0.010f;
static constexpr float ZC_HYS_FRAC                  = 0.08f;
static constexpr float ZC_DWELL_THR_FRAC            = 0.05f;
static constexpr float ZC_DWELL_THR_MIN_A           = 0.012f;

static constexpr float PULSE_MIN_WIDTH_US           = 12.0f;
static constexpr float PULSE_MAX_WIDTH_US           = 420.0f;
static constexpr float PULSE_THRESH_RMS_MUL         = 3.2f;
static constexpr float PULSE_THRESH_MIN_A           = 0.040f;

static constexpr float MIDBAND_LO_HZ                = 300.0f;
static constexpr float MIDBAND_HI_HZ                = 7000.0f;
static constexpr float UPPERMID_LO_HZ               = 1500.0f;
static constexpr float UPPERMID_HI_HZ               = 8000.0f;
static constexpr float HF_BAND_LO_HZ                = 8000.0f;
static constexpr float HF_BAND_HI_HZ                = 25000.0f;
static constexpr float SPEC_ENT_LO_HZ               = 300.0f;
static constexpr float SPEC_ENT_HI_HZ               = 25000.0f;

static constexpr float FUND_SNR_MIN                 = 4.0f;
static constexpr float FUND_MAG_MIN                 = 1e-5f;
static constexpr float SF_EPS                       = 1e-12f;

static constexpr int CURRENT_MIN_ACTIVITY_CHANGES   = 6;
static constexpr uint16_t CURRENT_MIN_CODE_SPAN     = 4;
static constexpr uint16_t LOW_CURRENT_CODE_SPAN     = 12;

static constexpr float CURRENT_FRAME_MIN_FS_HZ      = 24000.0f;
static constexpr uint32_t CURRENT_BOOT_SETTLE_MS    = 300UL;

static constexpr float PULSE_ANALYSIS_MIN_IRMS_A    = 0.015f;
static constexpr float PULSE_ANALYSIS_MIN_RESID_A   = 0.015f;

static constexpr float TRUE_RMS_MIN_COHERENCE       = 0.60f;
static constexpr float TRUE_RMS_RELAXED_COHERENCE   = 0.42f;
static constexpr float TRUE_RMS_MIN_COHERENT_A      = 0.20f;
static constexpr float TRUE_RMS_MAX_RESID_FRAC      = 0.85f;

static constexpr float CURRENT_RMS_FLOOR_SUB_A      = 0.000f;
static constexpr float CURRENT_DISPLAY_ON_A         = 0.020f;
static constexpr float CURRENT_DISPLAY_OFF_A        = 0.010f;

#else

static constexpr float IRMS_GATE_ON_A               = 0.050f;
static constexpr float IRMS_GATE_OFF_A              = 0.025f;
static constexpr float CURRENT_IDLE_SUPPRESS_A      = 0.040f;
static constexpr float FEATURE_MIN_VRMS             = 70.0f;
static constexpr float FEATURE_MIN_IRMS_A           = 0.050f;
static constexpr float ARC_MIN_IRMS_A               = 0.060f;
static constexpr float FEATURE_REQUIRE_FUND_BELOW_A = 0.180f;

static constexpr float CURRENT_LPF_HZ               = 1800.0f;
static constexpr float CURRENT_BASE_LPF_HZ          = 260.0f;

static constexpr float ZC_HYS_MIN_A                 = 0.035f;
static constexpr float ZC_HYS_FRAC                  = 0.18f;
static constexpr float ZC_DWELL_THR_FRAC            = 0.10f;
static constexpr float ZC_DWELL_THR_MIN_A           = 0.035f;

static constexpr float PULSE_MIN_WIDTH_US           = 20.0f;
static constexpr float PULSE_MAX_WIDTH_US           = 500.0f;
static constexpr float PULSE_THRESH_RMS_MUL         = 7.0f;
static constexpr float PULSE_THRESH_MIN_A           = 0.120f;

static constexpr float MIDBAND_LO_HZ                = 220.0f;
static constexpr float MIDBAND_HI_HZ                = 3500.0f;
static constexpr float UPPERMID_LO_HZ               = 900.0f;
static constexpr float UPPERMID_HI_HZ               = 6000.0f;
static constexpr float HF_BAND_LO_HZ                = 6000.0f;
static constexpr float HF_BAND_HI_HZ                = 15000.0f;
static constexpr float SPEC_ENT_LO_HZ               = 220.0f;
static constexpr float SPEC_ENT_HI_HZ               = 15000.0f;

static constexpr float FUND_SNR_MIN                 = 6.0f;
static constexpr float FUND_MAG_MIN                 = 1e-5f;
static constexpr float SF_EPS                       = 1e-12f;

static constexpr int CURRENT_MIN_ACTIVITY_CHANGES   = 10;
static constexpr uint16_t CURRENT_MIN_CODE_SPAN     = 10;
static constexpr uint16_t LOW_CURRENT_CODE_SPAN     = 30;

static constexpr float CURRENT_FRAME_MIN_FS_HZ      = 24000.0f;
static constexpr uint32_t CURRENT_BOOT_SETTLE_MS    = 450UL;

static constexpr float PULSE_ANALYSIS_MIN_IRMS_A    = 0.075f;
static constexpr float PULSE_ANALYSIS_MIN_RESID_A   = 0.035f;

static constexpr float TRUE_RMS_MIN_COHERENCE       = 0.55f;
static constexpr float TRUE_RMS_RELAXED_COHERENCE   = 0.40f;
static constexpr float TRUE_RMS_MIN_COHERENT_A      = 0.10f;
static constexpr float TRUE_RMS_MAX_RESID_FRAC      = 0.80f;

// MCP path on this board currently carries a stable low-current hardware floor.
// Subtract it after RMS estimation, then use a small hysteresis so idle noise
// stays at 0 A while a real fan load can still appear.
static constexpr float CURRENT_RMS_FLOOR_SUB_A      = 0.000f;
static constexpr float CURRENT_DISPLAY_ON_A         = 0.040f;
static constexpr float CURRENT_DISPLAY_OFF_A        = 0.020f;

#endif

// =========================
// Current calibration
// =========================
static constexpr float CURRENT_CAL_C3 = 0.00061628f;
static constexpr float CURRENT_CAL_C2 = -0.000465664f;
static constexpr float CURRENT_CAL_C1 = 1.22698f;
static constexpr float CURRENT_CAL_C0 = -0.0360701f;

struct CurrentCalib {
#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684
  float adcFullScaleV = ADS_VREF_V;
  float dividerRatio  = 4.096f / 5.0f;
#else
  float adcFullScaleV = 3.300f;
  float dividerRatio  = 10.0f / (5.1f + 10.0f);
#endif

  float offsetV      = 2.5f;
  float voltsPerAmp  = 0.100f;
  float ampsScale    = 0.790f;

  float cubic3 = CURRENT_CAL_C3;
  float cubic2 = CURRENT_CAL_C2;
  float cubic1 = CURRENT_CAL_C1;
  float cubic0 = CURRENT_CAL_C0;
};
