#pragma once
#include <Arduino.h>

#define USE_M2CGEN_RF

#ifndef ENABLE_MODEL_INFERENCE
#define ENABLE_MODEL_INFERENCE 1
#endif

#ifndef ENABLE_RULE_FALLBACK
#define ENABLE_RULE_FALLBACK 0
#endif

#ifndef ENABLE_ML_LOGGER
#define ENABLE_ML_LOGGER 1
#endif

#ifndef COLLECTION_ONLY_MODE
#define COLLECTION_ONLY_MODE 0
#endif

#ifndef ENABLE_AUTO_ARC_CAPTURE
#define ENABLE_AUTO_ARC_CAPTURE 1
#endif

#define USE_SOFT_LATCH 0

// =========================
// Relay / buzzer hardware
// =========================
static constexpr bool RELAY_ACTIVE_LOW      = true;
static constexpr bool BUZZER_PASSIVE_PWM    = true;
static constexpr uint8_t BUZZER_PWM_BITS    = 8;
static constexpr uint8_t BUZZER_PWM_DUTY    = 128;
static constexpr bool BUZZER_STATUS_ENABLED = true;
static constexpr uint32_t BUZZER_STARTUP_MUTE_MS = 700;
static constexpr uint32_t SYSTEM_READY_CHIME_DELAY_MS = 1000;

// =========================
// Sampling and FFT
// =========================
static constexpr float    FS_TARGET_HZ = 125000.0f;
static constexpr uint16_t N_SAMP       = 4096;
static constexpr float    MAINS_F0_HZ  = 60.0f;

// =========================
// Wi-Fi startup / portal
// =========================
static constexpr uint32_t WIFI_BOOT_BLOCK_MS      = 10000UL;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 10000UL;
static constexpr uint32_t WIFI_PORTAL_TIMEOUT_MS  = 180000UL;
static constexpr uint8_t  WIFI_TRIPLE_TAP_COUNT   = 3;

// =========================
// Protection thresholds
// =========================
static constexpr float OVERLOAD_WARN_A      = 10.0f;
static constexpr float OVERLOAD_HARD_TRIP_A = 15.0f;
static constexpr float OVERLOAD_TRIP_A      = OVERLOAD_WARN_A;

// Temperature cutoff:
// collection mode -> 80 C
// normal mode     -> 70 C
static constexpr float TEMP_TRIP_C          = 70.0f;
static constexpr float TEMP_DATA_WARN_C     = 80.0f;
static constexpr float TEMP_DATA_HARD_C     = 90.0f;

// Voltage protection
static constexpr float VOLT_UNDERVOLT_MIN_V = 100.0f;
static constexpr float VOLT_UNDERVOLT_MAX_V = 200.0f;
static constexpr float VOLT_OVERVOLT_TRIP_V = 250.0f;

// Backward-compatible alias for older code paths
static constexpr float VOLT_SURGE_TRIP_V    = VOLT_OVERVOLT_TRIP_V;

static constexpr float MAINS_PRESENT_OFF_V  = 12.0f;
static constexpr float MAINS_PRESENT_ON_V   = 25.0f;

// Battery hold / outage shutdown
static constexpr uint32_t OUTAGE_SHUTDOWN_MS         = 10UL * 60UL * 1000UL;
static constexpr uint32_t OUTAGE_BOOT_ARM_MS         = 30000UL;
static constexpr uint32_t OUTAGE_PRESENT_DEBOUNCE_MS = 3000UL;
static constexpr uint32_t OUTAGE_ABSENT_DEBOUNCE_MS  = 5000UL;

// =========================
// Leaky integrator
// =========================
static constexpr int ARC_CNT_INC  = 1;
static constexpr int ARC_CNT_DEC  = 2;
static constexpr int ARC_CNT_TRIP = 10;
static constexpr int ARC_CNT_MAX  = 20;
static constexpr uint32_t ARC_HOLD_MS  = 800;
static constexpr uint32_t HEAT_HOLD_MS = 1200;

static constexpr int HEAT_FRAMES_TRIP = 6;
static constexpr int HEAT_FRAMES_DEC  = 1;

// =========================
// Pins (XIAO ESP32S3)
// =========================
static constexpr int PIN_VOLT_ADC    = D0;
static constexpr int PIN_TEMP_ADC    = D1;
static constexpr int PIN_BAT_HOLD_EN = D6;

static constexpr int PIN_ADC_CS   = D3;
static constexpr int PIN_ADC_SCK  = D8;
static constexpr int PIN_ADC_MISO = D9;
static constexpr int PIN_ADC_MOSI = D10;

// Direct ESP32 ADC fallback pin
static constexpr int PIN_CUR_ADC_ANALOG = D10;

static constexpr int PIN_RELAY      = D7;
static constexpr int PIN_BUZZER_PWM = D2;

// =========================
// Current capture backend
// =========================
#define CUR_BACKEND_ADS8684   0
#define CUR_BACKEND_MCP3204   1
#define CUR_BACKEND_ESP32_ADC 2

#ifndef CURRENT_CAPTURE_BACKEND
#define CURRENT_CAPTURE_BACKEND CUR_BACKEND_MCP3204
#endif

static inline const char* currentBackendName() {
#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684
  return "ADS8684";
#elif CURRENT_CAPTURE_BACKEND == CUR_BACKEND_MCP3204
  return "MCP3204";
#else
  return "ESP32 ADC";
#endif
}

// =========================
// ADC / backend acquisition
// =========================
static constexpr float ADS_VREF_V = 4.096f;
static constexpr int   ADS_SPI_HZ = 4000000;
static constexpr int   ADS_SPI_FALLBACK_HZ = 2000000;

// MCP3204 temporary backend
static constexpr uint8_t  MCP3204_CHANNEL         = 0;
static constexpr uint32_t MCP3204_SPI_HZ          = 2000000UL;
static constexpr uint8_t  MCP3204_OVERSAMPLE      = 1;

// 1 = single sample, 2 = average of 2, 3 = median-of-3.
// For MCP3204, 3 is the best default to suppress impulsive junk
// without washing out arc-like detail too much.
static constexpr uint8_t  MCP3204_MEDIAN_SAMPLES  = 3;

static constexpr uint16_t MCP3204_STARTUP_FLUSH   = 512;
static constexpr uint8_t  MCP3204_WARMUP_BURSTS   = 4;
static constexpr uint8_t  MCP3204_BURST_FLUSH     = 8;

// Direct ESP32 ADC fallback
static constexpr uint8_t  ESP32_ADC_OVERSAMPLE   = 2;

// =========================
// Backend-specific profiles
// =========================
#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684

// Clean / intended final hardware profile
static constexpr float IRMS_GATE_ON_A             = 0.008f;
static constexpr float IRMS_GATE_OFF_A            = 0.004f;
static constexpr float CURRENT_IDLE_SUPPRESS_A    = 0.010f;
static constexpr float FEATURE_MIN_VRMS           = 70.0f;
static constexpr float FEATURE_MIN_IRMS_A         = 0.020f;
static constexpr float ARC_MIN_IRMS_A             = 0.030f;
static constexpr float FEATURE_REQUIRE_FUND_BELOW_A = 0.080f;

static constexpr float CURRENT_LPF_HZ             = 3200.0f;
static constexpr float CURRENT_BASE_LPF_HZ        = 550.0f;

static constexpr float ZC_HYS_MIN_A               = 0.012f;
static constexpr float ZC_HYS_FRAC                = 0.10f;
static constexpr float ZC_DWELL_THR_FRAC          = 0.06f;
static constexpr float ZC_DWELL_THR_MIN_A         = 0.015f;

static constexpr float PULSE_MIN_WIDTH_US         = 15.0f;
static constexpr float PULSE_MAX_WIDTH_US         = 450.0f;
static constexpr float PULSE_THRESH_RMS_MUL       = 3.5f;
static constexpr float PULSE_THRESH_MIN_A         = 0.050f;

static constexpr float MIDBAND_LO_HZ              = 300.0f;
static constexpr float MIDBAND_HI_HZ              = 6000.0f;
static constexpr float UPPERMID_LO_HZ             = 1500.0f;
static constexpr float UPPERMID_HI_HZ             = 6000.0f;
static constexpr float HF_BAND_LO_HZ              = 6000.0f;
static constexpr float HF_BAND_HI_HZ              = 20000.0f;
static constexpr float SPEC_ENT_LO_HZ             = 300.0f;
static constexpr float SPEC_ENT_HI_HZ             = 20000.0f;

static constexpr float FUND_SNR_MIN               = 4.0f;
static constexpr float FUND_MAG_MIN               = 1e-5f;
static constexpr float SF_EPS                     = 1e-12f;

static constexpr int CURRENT_MIN_ACTIVITY_CHANGES = 8;
static constexpr uint16_t CURRENT_MIN_CODE_SPAN   = 6;
static constexpr uint16_t LOW_CURRENT_CODE_SPAN   = 16;

static constexpr float CURRENT_FRAME_MIN_FS_HZ    = 20000.0f;
static constexpr uint32_t CURRENT_BOOT_SETTLE_MS  = 300UL;

#elif CURRENT_CAPTURE_BACKEND == CUR_BACKEND_MCP3204

// Temporary MCP3204 profile: prioritize stability, medium loads, and avoid idle junk.
static constexpr float IRMS_GATE_ON_A             = 0.075f;
static constexpr float IRMS_GATE_OFF_A            = 0.045f;
static constexpr float CURRENT_IDLE_SUPPRESS_A    = 0.075f;
static constexpr float FEATURE_MIN_VRMS           = 70.0f;
static constexpr float FEATURE_MIN_IRMS_A         = 0.075f;
static constexpr float ARC_MIN_IRMS_A             = 0.075f;
static constexpr float FEATURE_REQUIRE_FUND_BELOW_A = 0.180f;

static constexpr float CURRENT_LPF_HZ             = 1800.0f;
static constexpr float CURRENT_BASE_LPF_HZ        = 260.0f;

static constexpr float ZC_HYS_MIN_A               = 0.035f;
static constexpr float ZC_HYS_FRAC                = 0.18f;
static constexpr float ZC_DWELL_THR_FRAC          = 0.10f;
static constexpr float ZC_DWELL_THR_MIN_A         = 0.035f;

static constexpr float PULSE_MIN_WIDTH_US         = 20.0f;
static constexpr float PULSE_MAX_WIDTH_US         = 500.0f;
static constexpr float PULSE_THRESH_RMS_MUL       = 7.0f;
static constexpr float PULSE_THRESH_MIN_A         = 0.120f;

static constexpr float MIDBAND_LO_HZ              = 220.0f;
static constexpr float MIDBAND_HI_HZ              = 3200.0f;
static constexpr float UPPERMID_LO_HZ             = 900.0f;
static constexpr float UPPERMID_HI_HZ             = 3200.0f;
static constexpr float HF_BAND_LO_HZ              = 3200.0f;
static constexpr float HF_BAND_HI_HZ              = 6800.0f;
static constexpr float SPEC_ENT_LO_HZ             = 220.0f;
static constexpr float SPEC_ENT_HI_HZ             = 6800.0f;

static constexpr float FUND_SNR_MIN               = 6.0f;
static constexpr float FUND_MAG_MIN               = 1e-5f;
static constexpr float SF_EPS                     = 1e-12f;

static constexpr int CURRENT_MIN_ACTIVITY_CHANGES = 10;
static constexpr uint16_t CURRENT_MIN_CODE_SPAN   = 10;
static constexpr uint16_t LOW_CURRENT_CODE_SPAN   = 30;

static constexpr float CURRENT_FRAME_MIN_FS_HZ    = 9000.0f;
static constexpr uint32_t CURRENT_BOOT_SETTLE_MS  = 450UL;

#else

// Emergency ESP32 ADC profile: very conservative
static constexpr float IRMS_GATE_ON_A             = 0.060f;
static constexpr float IRMS_GATE_OFF_A            = 0.040f;
static constexpr float CURRENT_IDLE_SUPPRESS_A    = 0.050f;
static constexpr float FEATURE_MIN_VRMS           = 70.0f;
static constexpr float FEATURE_MIN_IRMS_A         = 0.180f;
static constexpr float ARC_MIN_IRMS_A             = 0.350f;
static constexpr float FEATURE_REQUIRE_FUND_BELOW_A = 0.500f;

static constexpr float CURRENT_LPF_HZ             = 1100.0f;
static constexpr float CURRENT_BASE_LPF_HZ        = 180.0f;

static constexpr float ZC_HYS_MIN_A               = 0.040f;
static constexpr float ZC_HYS_FRAC                = 0.18f;
static constexpr float ZC_DWELL_THR_FRAC          = 0.10f;
static constexpr float ZC_DWELL_THR_MIN_A         = 0.040f;

static constexpr float PULSE_MIN_WIDTH_US         = 20.0f;
static constexpr float PULSE_MAX_WIDTH_US         = 700.0f;
static constexpr float PULSE_THRESH_RMS_MUL       = 7.0f;
static constexpr float PULSE_THRESH_MIN_A         = 0.120f;

static constexpr float MIDBAND_LO_HZ              = 150.0f;
static constexpr float MIDBAND_HI_HZ              = 1800.0f;
static constexpr float UPPERMID_LO_HZ             = 700.0f;
static constexpr float UPPERMID_HI_HZ             = 1800.0f;
static constexpr float HF_BAND_LO_HZ              = 1800.0f;
static constexpr float HF_BAND_HI_HZ              = 4500.0f;
static constexpr float SPEC_ENT_LO_HZ             = 150.0f;
static constexpr float SPEC_ENT_HI_HZ             = 4500.0f;

static constexpr float FUND_SNR_MIN               = 7.0f;
static constexpr float FUND_MAG_MIN               = 1e-5f;
static constexpr float SF_EPS                     = 1e-12f;

static constexpr int CURRENT_MIN_ACTIVITY_CHANGES = 12;
static constexpr uint16_t CURRENT_MIN_CODE_SPAN   = 16;
static constexpr uint16_t LOW_CURRENT_CODE_SPAN   = 44;

static constexpr float CURRENT_FRAME_MIN_FS_HZ    = 3000.0f;
static constexpr uint32_t CURRENT_BOOT_SETTLE_MS  = 300UL;

#endif

#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684
static constexpr float PULSE_ANALYSIS_MIN_IRMS_A  = 0.020f;
static constexpr float PULSE_ANALYSIS_MIN_RESID_A = 0.020f;
#elif CURRENT_CAPTURE_BACKEND == CUR_BACKEND_MCP3204
static constexpr float PULSE_ANALYSIS_MIN_IRMS_A  = 0.075f;
static constexpr float PULSE_ANALYSIS_MIN_RESID_A = 0.035f;
#else
static constexpr float PULSE_ANALYSIS_MIN_IRMS_A  = 0.180f;
static constexpr float PULSE_ANALYSIS_MIN_RESID_A = 0.050f;
#endif

static constexpr uint32_t FEAT_STALE_MS           = 350;
static constexpr uint32_t ML_CTRL_POLL_MS         = 10000;
static constexpr uint16_t AUTO_ARC_CAPTURE_DURATION_S = 12;
static constexpr uint32_t AUTO_ARC_CAPTURE_COOLDOWN_MS = 60000UL;
static constexpr int8_t ML_UNKNOWN_LABEL = -1;

// =========================
// Rule fallback thresholds
// =========================
static constexpr float CYCLE_NMSE_ARC_H        = 0.18f;
static constexpr float ZCV_ARC_H_MS            = 0.20f;
static constexpr float ZC_DWELL_ARC_H          = 0.22f;
static constexpr float PULSE_DENS_ARC_H        = 1.20f;
static constexpr float PEAK_FLUCT_ARC_H        = 0.11f;
static constexpr float MIDBAND_RESID_ARC_H     = 0.08f;
static constexpr float HF_ENERGY_ARC_H         = 0.28f;
static constexpr float WPE_ENT_ARC_H           = 0.60f;
static constexpr float SPEC_ENT_ARC_H          = 0.68f;
static constexpr float THD_STEADY_GUARD_PCT    = 135.0f;
static constexpr float LOW_UNCERT_CYCLE_NMSE   = 0.06f;
static constexpr float LOW_UNCERT_ZCV_MS       = 0.06f;
static constexpr float LOW_UNCERT_PULSE_DENS   = 0.25f;

// =========================
// Calibration helpers
// =========================
static inline float eval_cubic_horner(float x, float c3, float c2, float c1, float c0) {
  return (((c3 * x) + c2) * x + c1) * x + c0;
}

// =========================
// Current calibration
// =========================
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

  float cubic3 = 0.0f;
  float cubic2 = 0.0f;
  float cubic1 = 1.0f;
  float cubic0 = 0.0f;
};

struct VoltageCalib {
  float sensitivity  = 580.0f;
  float cubic3 = 0.0f;
  float cubic2 = 0.0f;
  float cubic1 = 1.0f;
  float cubic0 = 0.0f;
};

// =========================
// Logging
// =========================
static constexpr uint16_t ML_LOG_RATE_HZ     = 30;
static constexpr uint16_t ML_LOG_DURATION_S  = 10;