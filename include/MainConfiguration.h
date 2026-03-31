#pragma once
#include <Arduino.h>
#include <math.h>

#ifndef COLLECTION_ONLY_MODE
#define COLLECTION_ONLY_MODE 0
#endif

static constexpr bool ENABLE_MODEL_INFERENCE = true;
static constexpr bool ENABLE_ML_LOGGER = true;
static constexpr bool ENABLE_AUTO_ARC_CAPTURE = true;

// Cloud / OTA configuration
static constexpr const char* FIREBASE_API_KEY = "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw";
static constexpr const char* FIREBASE_DB_URL  = "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app";
static constexpr const char* FW_VERSION       = "TSP-v4.1.0-p-mcp";
static constexpr const char* OTA_DESIRED_VERSION_PATH = "/ota/desired_version";
static constexpr const char* OTA_FIRMWARE_URL_PATH    = "/ota/firmware_url";

// =========================
// Fault / data types
// =========================
enum FaultState : uint8_t {
  STATE_NORMAL = 0,
  STATE_OVERLOAD,
  STATE_SUSTAINED_OVERLOAD,
  STATE_UNDERVOLTAGE,
  STATE_OVERVOLTAGE,
  STATE_HEATING,
  STATE_ARCING
};

static inline const char* stateToCstr(FaultState s) {
  switch (s) {
    case STATE_OVERLOAD:            return "OVERLOAD";
    case STATE_SUSTAINED_OVERLOAD:  return "SUSTAINED OVERLOAD";
    case STATE_UNDERVOLTAGE:        return "UNDERVOLTAGE";
    case STATE_OVERVOLTAGE:         return "OVERVOLTAGE";
    case STATE_HEATING:             return "HEATING";
    case STATE_ARCING:              return "ARCING";
    default:                        return "NORMAL";
  }
}

struct FeatureFrame {
  uint64_t epoch_ms  = 0;
  uint32_t uptime_ms = 0;

  float vrms   = 0.0f;
  float irms   = 0.0f;
  float temp_c = 0.0f;

  float cycle_nmse            = 0.0f;
  float zcv                   = 0.0f;
  float zc_dwell_ratio        = 0.0f;
  float pulse_count_per_cycle = 0.0f;
  float peak_fluct_cv         = 0.0f;
  float midband_residual_rms  = 0.0f;
  float hf_band_energy_ratio  = 0.0f;
  float wpe_entropy           = 0.0f;
  float spec_entropy          = 0.0f;
  float thd_i                 = 0.0f;

  float adc_fs_hz = 0.0f;

  uint8_t feat_valid    = 0;
  uint8_t current_valid = 0;
  uint8_t model_pred    = 0;
};

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
static constexpr uint32_t BUZZER_ARTIFACT_SUPPRESS_HOLD_MS = 260UL;
static constexpr float    BUZZER_ARTIFACT_MAX_A = 0.60f;
static constexpr uint16_t BUZZER_STATUS_MAX_HZ = 820;
static constexpr uint8_t  BUZZER_STATUS_MAX_DUTY = 26;

// =========================
// Sampling and FFT
// =========================
static constexpr float    FS_TARGET_HZ = 35000.0f;
static constexpr uint16_t N_SAMP       = 4096;
static constexpr float    MAINS_F0_HZ  = 60.0f;

static constexpr bool     CURRENT_SOFT_AAF_ENABLE = true;
static constexpr float    CURRENT_SOFT_AAF_CUTOFF_HZ = 15000.0f;
static constexpr float    CURRENT_SOFT_AAF_Q = 0.70710678f;
static constexpr float    CURRENT_SOFT_AAF_MAX_FRAC_NYQUIST = 0.98f;

// =========================
// Wi-Fi startup / portal
// =========================
static constexpr uint32_t WIFI_BOOT_CONNECT_MS      = 10000UL;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS   = 10000UL;
static constexpr uint32_t WIFI_PORTAL_TIMEOUT_MS    = 30000UL;
static constexpr uint32_t WIFI_BACKGROUND_RETRY_MS  = 60000UL;
static constexpr uint32_t WIFI_BOOT_NO_CRED_AP_WINDOW_MS = 10000UL;
static constexpr uint32_t WIFI_MANUAL_AP_WINDOW_MS  = 15000UL;
static constexpr uint32_t WIFI_PORTAL_AP_GRACE_MS   = 1500UL;
static constexpr const char* WIFI_PORTAL_SSID       = "TinyML-SmartPlug";

// =========================
// Protection thresholds
// =========================
static constexpr float VOLT_NORMAL_MIN_V = 207.0f;
static constexpr float VOLT_NORMAL_MAX_V = 253.0f;
static constexpr float VOLT_UV_INSTANT_V = 170.0f;
static constexpr float VOLT_UV_DELAY_V   = 200.0f;
static constexpr float VOLT_UV_WARN_V    = 207.0f;
static constexpr float VOLT_OV_DELAY_V   = 250.0f;
static constexpr float VOLT_OV_INSTANT_V = 265.0f;

static constexpr uint32_t VOLT_UV_INSTANT_MS = 0UL;
static constexpr uint32_t VOLT_UV_DELAY_MS   = 7000UL;
static constexpr uint32_t VOLT_UV_WARN_MS    = 60000UL;
static constexpr uint32_t VOLT_OV_DELAY_MS   = 7000UL;
static constexpr uint32_t VOLT_OV_INSTANT_MS = 0UL;

static constexpr float VOLT_UV_CANDIDATE_RAW_MIN_V = 145.0f;
static constexpr float VOLT_RECOVER_MIN_V = 212.0f;
static constexpr float VOLT_RECOVER_MAX_V = 250.0f;
static constexpr uint32_t VOLTAGE_RECLOSE_STABLE_MS = 5UL * 60UL * 1000UL;

static constexpr float OVERLOAD_WARN_A      = 10.0f;
static constexpr float OVERLOAD_HARD_TRIP_A = 15.0f;
static constexpr float SUSTAINED_OVERLOAD_TRIP_A = 14.5f;
static constexpr uint32_t OVERLOAD_TRIP_MS  = 250UL;
static constexpr uint32_t SUSTAINED_OVERLOAD_TRIP_MS = 60UL * 1000UL;

static constexpr float TEMP_TRIP_C          = 70.0f;
static constexpr float TEMP_DATA_WARN_C     = 80.0f;
static constexpr float TEMP_DATA_HARD_C     = 90.0f;

static constexpr float MAINS_PRESENT_OFF_V  = 12.0f;
static constexpr float MAINS_PRESENT_ON_V   = 28.0f;
static constexpr uint32_t MAINS_EDGE_DEBOUNCE_MS = 1000UL;
static constexpr uint32_t UNPLUGGED_BUZZ_DELAY_MS = 2000UL;
static constexpr uint32_t UNPLUGGED_STATE_DELAY_MS = 3000UL;

// =========================
// Leaky integrator / fault display hold
// =========================
static constexpr int ARC_CNT_INC  = 1;
static constexpr int ARC_CNT_DEC  = 2;
static constexpr int ARC_CNT_TRIP = 10;
static constexpr int ARC_CNT_MAX  = 20;
static constexpr uint32_t ARC_HOLD_MS  = 800;
static constexpr uint32_t HEAT_HOLD_MS = 1200;
static constexpr uint32_t FAULT_ALERT_MIN_MS = 5000UL;

static constexpr int HEAT_FRAMES_TRIP = 6;
static constexpr int HEAT_FRAMES_DEC  = 1;

// =========================
// Pins (XIAO ESP32S3)
// =========================
static constexpr int PIN_VOLT_ADC    = D0;
static constexpr int PIN_TEMP_ADC    = D1;
static constexpr int PIN_LATCH_ON    = D6;
static constexpr int PIN_LATCH_OFF   = D7;
static constexpr int PIN_BUZZER_PWM  = D2;
static constexpr int PIN_ADC_CS      = D3;
static constexpr int PIN_ADC_SCK     = D8;
static constexpr int PIN_ADC_MISO    = D9;
static constexpr int PIN_ADC_MOSI    = D10;

// =========================
// Hardware latch pulse control
// =========================
static constexpr uint32_t LATCH_ON_PULSE_MS  = 120UL;
static constexpr uint32_t LATCH_OFF_PULSE_MS = 120UL;
static constexpr uint32_t LATCH_PULSE_GAP_MS = 250UL;

// =========================
// Load state / button override sync
// =========================
static constexpr float LOAD_ON_DETECT_A   = 0.12f;
static constexpr float LOAD_OFF_DETECT_A  = 0.03f;
static constexpr uint32_t LOAD_ON_DETECT_MS  = 350UL;
static constexpr uint32_t LOAD_OFF_DETECT_MS = 1200UL;

// =========================
// Logger / control polling
// =========================
static constexpr uint32_t FEAT_STALE_MS                 = 350UL;
static constexpr uint32_t SENSOR_BOOT_SETTLE_MS         = 450UL;
static constexpr uint32_t PROTECTION_INHIBIT_MS         = 5000UL;
static constexpr uint32_t ML_CONTROL_POLL_MS            = 3000UL;
static constexpr uint32_t CLOUD_CONTROL_POLL_MS         = 1500UL;
static constexpr uint16_t AUTO_ARC_CAPTURE_DURATION_S   = 12;
static constexpr uint32_t AUTO_ARC_CAPTURE_COOLDOWN_MS  = 60000UL;
static constexpr int8_t ML_UNKNOWN_LABEL                = -1;
static constexpr uint16_t ML_LOG_RATE_HZ                = 30;
static constexpr uint16_t ML_LOG_DURATION_S             = 10;
static constexpr uint16_t ML_LOG_CHUNK_DURATION_S       = 10;
static constexpr uint16_t ML_LOG_MIN_DURATION_S         = 1;
static constexpr uint16_t ML_LOG_MAX_DURATION_S         = 7200;
static constexpr uint16_t ML_LOG_AUTO_MIN_DURATION_S    = 5;
static constexpr uint16_t ML_LOG_AUTO_MAX_DURATION_S    = 60;

static constexpr uint32_t CLOUD_LIVE_NORMAL_INTERVAL_MS = 10000UL;
static constexpr uint32_t CLOUD_LIVE_FAULT_INTERVAL_MS  = 2500UL;
static constexpr uint32_t CLOUD_REFRESH_KEEPALIVE_MS    = 1000UL;

// =========================
// Current backend (MCP3204 only)
// =========================
static constexpr uint8_t  MCP3204_CHANNEL        = 0;
static constexpr uint32_t MCP3204_SPI_HZ         = 2000000UL;
static constexpr uint8_t  MCP3204_OVERSAMPLE     = 1;
static constexpr uint8_t  MCP3204_MEDIAN_SAMPLES = 1;
static constexpr uint16_t MCP3204_STARTUP_FLUSH  = 256;
static constexpr uint8_t  MCP3204_WARMUP_BURSTS  = 2;
static constexpr uint8_t  MCP3204_BURST_FLUSH    = 4;

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
static constexpr float PULSE_ANALYSIS_MIN_IRMS_A    = 0.075f;
static constexpr float PULSE_ANALYSIS_MIN_RESID_A   = 0.035f;
static constexpr float CURRENT_DISPLAY_ON_A         = 0.040f;
static constexpr float CURRENT_DISPLAY_OFF_A        = 0.020f;

static constexpr float CURRENT_CAL_C3 = 0.00061628f;
static constexpr float CURRENT_CAL_C2 = -0.000465664f;
static constexpr float CURRENT_CAL_C1 = 1.22698f;
static constexpr float CURRENT_CAL_C0 = -0.0360701f;

struct CurrentCalib {
  float adcFullScaleV = 3.300f;
  float dividerRatio  = 10.0f / (5.1f + 10.0f);
  float offsetV       = 2.5f;
  float voltsPerAmp   = 0.100f;
  float ampsScale     = 0.790f;
  float cubic3 = CURRENT_CAL_C3;
  float cubic2 = CURRENT_CAL_C2;
  float cubic1 = CURRENT_CAL_C1;
  float cubic0 = CURRENT_CAL_C0;
};

// =========================
// Voltage calibration and display cleanup
// =========================
static inline float eval_cubic_horner(float x, float c3, float c2, float c1, float c0) {
  return (((c3 * x) + c2) * x + c1) * x + c0;
}

static constexpr float VOLTAGE_CAL_C3 = 0.00000122081f;
static constexpr float VOLTAGE_CAL_C2 = -0.000503178f;
static constexpr float VOLTAGE_CAL_C1 = 1.05726f;
static constexpr float VOLTAGE_CAL_C0 = -0.495263f;

struct VoltageCalib {
  float sensitivity  = 580.0f;
  float cubic3 = VOLTAGE_CAL_C3;
  float cubic2 = VOLTAGE_CAL_C2;
  float cubic1 = VOLTAGE_CAL_C1;
  float cubic0 = VOLTAGE_CAL_C0;
};

static constexpr bool     CURRENT_IDLE_LEARN_ENABLE         = true;
static constexpr uint32_t CURRENT_IDLE_LEARN_WINDOW_MS      = 15000UL;
static constexpr float    CURRENT_IDLE_LEARN_MAX_A          = 0.35f;
static constexpr uint16_t CURRENT_IDLE_LEARN_MIN_FRAMES     = 8;
static constexpr float    CURRENT_IDLE_FLOOR_MARGIN_A       = 0.015f;
static constexpr float    CURRENT_DISPLAY_GATE_ON_A         = 0.060f;
static constexpr float    CURRENT_DISPLAY_GATE_OFF_A        = 0.030f;
static constexpr float    CURRENT_DISPLAY_SMOOTH_TAU_LOW_S  = 0.45f;
static constexpr float    CURRENT_DISPLAY_SMOOTH_TAU_HIGH_S = 0.18f;
static constexpr float    CURRENT_DISPLAY_SMOOTH_SPLIT_A    = 1.00f;

static constexpr uint32_t DEVICE_PLUG_CUE_INHIBIT_MS = 2000UL;
static constexpr uint32_t DEVICE_PLUG_CUE_COOLDOWN_MS = 6000UL;
static constexpr uint32_t DEVICE_PLUG_STABLE_MS = 1200UL;
