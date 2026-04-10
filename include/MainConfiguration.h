#pragma once
#include <Arduino.h>
#include <math.h>

#ifndef PROTECTION
#define PROTECTION 0
#endif

static constexpr bool ENABLE_MODEL_INFERENCE = true;
static constexpr bool ENABLE_ML_LOGGER = true;
static constexpr bool ENABLE_AUTO_ARC_CAPTURE = false;

// Cloud / OTA configuration
static constexpr const char* FIREBASE_API_KEY = "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw";
static constexpr const char* FIREBASE_DB_URL  = "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app";
static constexpr const char* FW_VERSION       = "v7.2.3-c-gen1";
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


// =========================
// Context / family inference
// =========================
// Canonical compact runtime taxonomy used by the current trainer/model headers.
// Keep legacy aliases below so older firmware/PWA/trainer symbols still compile.
#ifndef CONTEXT_FAMILY_UNKNOWN
#define CONTEXT_FAMILY_UNKNOWN -1
#endif
#ifndef CONTEXT_FAMILY_RESISTIVE_LINEAR
#define CONTEXT_FAMILY_RESISTIVE_LINEAR 0
#endif
#ifndef CONTEXT_FAMILY_INDUCTIVE_MOTOR
#define CONTEXT_FAMILY_INDUCTIVE_MOTOR 1
#endif
#ifndef CONTEXT_FAMILY_RECTIFIER_SMPS
#define CONTEXT_FAMILY_RECTIFIER_SMPS 2
#endif
#ifndef CONTEXT_FAMILY_PHASE_ANGLE_CONTROLLED
#define CONTEXT_FAMILY_PHASE_ANGLE_CONTROLLED 3
#endif
#ifndef CONTEXT_FAMILY_BRUSH_UNIVERSAL_MOTOR
#define CONTEXT_FAMILY_BRUSH_UNIVERSAL_MOTOR 4
#endif
#ifndef CONTEXT_FAMILY_OTHER_MIXED
#define CONTEXT_FAMILY_OTHER_MIXED 5
#endif

// Legacy family-name aliases from older code/data.
#ifndef CONTEXT_FAMILY_RESISTIVE
#define CONTEXT_FAMILY_RESISTIVE CONTEXT_FAMILY_RESISTIVE_LINEAR
#endif
#ifndef CONTEXT_FAMILY_SMPS
#define CONTEXT_FAMILY_SMPS CONTEXT_FAMILY_RECTIFIER_SMPS
#endif
#ifndef CONTEXT_FAMILY_DIMMER_PHASE
#define CONTEXT_FAMILY_DIMMER_PHASE CONTEXT_FAMILY_PHASE_ANGLE_CONTROLLED
#endif
#ifndef CONTEXT_FAMILY_UNIVERSAL_MOTOR
#define CONTEXT_FAMILY_UNIVERSAL_MOTOR CONTEXT_FAMILY_BRUSH_UNIVERSAL_MOTOR
#endif
#ifndef CONTEXT_FAMILY_MIXED_UNKNOWN
#define CONTEXT_FAMILY_MIXED_UNKNOWN CONTEXT_FAMILY_OTHER_MIXED
#endif
#ifndef CONTEXT_FAMILY_OTHER
#define CONTEXT_FAMILY_OTHER CONTEXT_FAMILY_OTHER_MIXED
#endif

enum ApplianceFamily : int8_t {
  FAMILY_UNKNOWN = CONTEXT_FAMILY_UNKNOWN,
  FAMILY_RESISTIVE_LINEAR = CONTEXT_FAMILY_RESISTIVE_LINEAR,
  FAMILY_INDUCTIVE_MOTOR = CONTEXT_FAMILY_INDUCTIVE_MOTOR,
  FAMILY_RECTIFIER_SMPS = CONTEXT_FAMILY_RECTIFIER_SMPS,
  FAMILY_PHASE_ANGLE_CONTROLLED = CONTEXT_FAMILY_PHASE_ANGLE_CONTROLLED,
  FAMILY_BRUSH_UNIVERSAL_MOTOR = CONTEXT_FAMILY_BRUSH_UNIVERSAL_MOTOR,
  FAMILY_OTHER_MIXED = CONTEXT_FAMILY_OTHER_MIXED,

  // Legacy enum aliases kept so older code still compiles.
  FAMILY_RESISTIVE = FAMILY_RESISTIVE_LINEAR,
  FAMILY_SMPS = FAMILY_RECTIFIER_SMPS,
  FAMILY_DIMMER_PHASE = FAMILY_PHASE_ANGLE_CONTROLLED,
  FAMILY_UNIVERSAL_MOTOR = FAMILY_BRUSH_UNIVERSAL_MOTOR,
  FAMILY_MIXED_UNKNOWN = FAMILY_OTHER_MIXED,
  FAMILY_OTHER = FAMILY_OTHER_MIXED,

  FAMILY_COUNT = 6
};

struct FeatureFrame {
  uint64_t epoch_ms  = 0;
  uint32_t uptime_ms = 0;
  uint32_t frame_start_uptime_ms = 0;
  uint32_t frame_end_uptime_ms = 0;
  uint32_t feature_compute_end_uptime_ms = 0;
  uint32_t log_enqueue_uptime_ms = 0;

  float frame_dt_ms = 0.0f;
  float compute_time_ms = 0.0f;
  float timing_skew_ms = 0.0f;
  float vrms   = 0.0f;
  float irms   = 0.0f;
  float temp_c = 0.0f;

  float spectral_flux_midhf          = 0.0f;
  float residual_crest_factor        = 0.0f;
  float edge_spike_ratio             = 0.0f;
  float midband_residual_ratio       = 0.0f;
  float cycle_nmse                   = 0.0f;
  float peak_fluct_cv                = 0.0f;
  float thd_i                        = 0.0f;
  float hf_energy_delta              = 0.0f;
  float zcv                          = 0.0f;
  float abs_irms_zscore_vs_baseline  = 0.0f;

  // Lightweight temporal / context signals.
  float fs_err_hz = 0.0f;
  float suspicious_run_energy = 0.0f;
  float delta_irms_abs = 0.0f;
  float delta_hf_energy = 0.0f;
  float delta_flux = 0.0f;
  float v_sag_pct = 0.0f;
  float halfcycle_asymmetry = 0.0f;
  float adc_fs_hz = 0.0f;

  uint32_t queue_drop_count = 0;
  uint16_t suspicious_run_len = 0;
  uint16_t invalid_loaded_run_len = 0;
  uint8_t restrike_count_short = 0;
  uint8_t feat_valid    = 0;
  uint8_t current_valid = 0;
  uint8_t model_pred    = 0;
  uint8_t sampling_quality_bad = 0;
  uint8_t invalid_loaded_flag = 0;
  uint8_t invalid_off_flag = 0;
  uint8_t relay_blank_active = 0;
  uint8_t turnon_blank_active = 0;
  uint8_t transient_blank_active = 0;

  int8_t  device_family_code = CONTEXT_FAMILY_UNKNOWN;
  int8_t  context_family_code_runtime = CONTEXT_FAMILY_UNKNOWN;
  int8_t  context_family_code_provisional = CONTEXT_FAMILY_UNKNOWN;
  float   context_family_confidence = 0.0f;
  float   context_family_confidence_provisional = 0.0f;
  uint8_t context_acquiring = 0;
  uint8_t context_latched = 0;
};


// Context timing / confidence tuning
static constexpr float CONTEXT_MIN_IRMS_A = 0.10f;
static constexpr float CONTEXT_UNLATCH_ZERO_IRMS_A = 0.020f;
static constexpr uint32_t CONTEXT_ACQUIRE_WINDOW_MS = 5000UL;
static constexpr uint32_t CONTEXT_UNLATCH_ZERO_MS = 5000UL;
static constexpr uint32_t CONTEXT_PROVISIONAL_MIN_MS = 300UL;
static constexpr float CONTEXT_MIN_CONFIDENCE = 0.45f;
static constexpr uint32_t CONTEXT_RESET_NO_MAINS_MS = 1200UL;

// =========================
// Relay / buzzer hardware
// =========================
static constexpr bool BUZZER_PASSIVE_PWM    = true;
static constexpr uint8_t BUZZER_PWM_BITS    = 8;
static constexpr uint8_t BUZZER_PWM_DUTY    = 128;
static constexpr bool BUZZER_STATUS_ENABLED = true;
static constexpr uint32_t BUZZER_STARTUP_MUTE_MS = 700;
static constexpr uint32_t SYSTEM_READY_CHIME_DELAY_MS = 1000;
static constexpr uint32_t BUZZER_ARTIFACT_SUPPRESS_HOLD_MS = 260UL;
static constexpr float    BUZZER_ARTIFACT_MAX_A = 0.35f;
static constexpr uint16_t BUZZER_STATUS_MAX_HZ = 820;
static constexpr uint8_t  BUZZER_STATUS_MAX_DUTY = 26;

// =========================
// Sampling and FFT
// =========================
// The ADC is tuned for a truthful operating cadence near 27 kHz. Runtime must keep
// the *measured* sampling rate truthful and must not snap or canonicalize it.
static constexpr float    FS_INTENDED_HZ = 27000.0f;
static constexpr float    FS_TARGET_HZ = FS_INTENDED_HZ;
static constexpr uint16_t N_SAMP       = 1024;
static constexpr float    MAINS_F0_HZ  = 60.0f;

// Pace feature generation to a stable latest-frame cadence instead of free-running.
static constexpr float    FEATURE_TARGET_CADENCE_HZ = 10.0f;
static constexpr uint32_t FEATURE_FRAME_PERIOD_US = (uint32_t)(1000000.0f / FEATURE_TARGET_CADENCE_HZ + 0.5f);
static constexpr float    FEATURE_FRAME_PERIOD_MS = 1000.0f / FEATURE_TARGET_CADENCE_HZ;

// The analog AAF is already around 10 kHz / Q≈0.73. Keep a conservative cascaded
// digital LPF as a second anti-alias / de-ringing stage before feature extraction.
static constexpr bool     CURRENT_SOFT_AAF_ENABLE = true;
static constexpr uint8_t  CURRENT_SOFT_AAF_STAGES = 3;
static constexpr float    CURRENT_SOFT_AAF_CUTOFF_HZ = 8700.0f;
static constexpr float    CURRENT_SOFT_AAF_Q = 0.73f;
static constexpr float    CURRENT_SOFT_AAF_MAX_FRAC_NYQUIST = 0.92f;

// =========================
// Wi-Fi startup / portal
// =========================
static constexpr uint32_t WIFI_BOOT_CONNECT_MS      = 15000UL;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS   = 15000UL;
static constexpr uint32_t WIFI_PORTAL_TIMEOUT_MS    = 45000UL;
static constexpr uint32_t WIFI_BACKGROUND_RETRY_MS  = 60000UL;
static constexpr uint32_t WIFI_STA_RETRY_MS         = 2500UL;
static constexpr uint32_t WIFI_BOOT_NO_CRED_AP_WINDOW_MS = 15000UL;
static constexpr uint32_t WIFI_MANUAL_AP_WINDOW_MS  = 45000UL;
static constexpr uint32_t WIFI_MANUAL_PORTAL_TIMEOUT_MS = 240000UL;
static constexpr uint32_t WIFI_PORTAL_AP_GRACE_MS   = 1500UL;
static constexpr const char* WIFI_PORTAL_SSID       = "TinyML-SmartPlug";

// =========================
// Protection thresholds
// =========================
static constexpr float VOLT_NORMAL_MIN_V = 200.0f;
static constexpr float VOLT_NORMAL_MAX_V = 250.0f;
static constexpr float VOLT_UV_INSTANT_V = 170.0f;
static constexpr float VOLT_UV_DELAY_V   = 200.0f;
static constexpr float VOLT_OV_DELAY_V   = 250.0f;
static constexpr float VOLT_OV_INSTANT_V = 260.0f;

static constexpr uint32_t VOLT_UV_INSTANT_MS = 0UL;
static constexpr uint32_t VOLT_UV_DELAY_MS   = 7000UL;
static constexpr uint32_t VOLT_OV_DELAY_MS   = 7000UL;
static constexpr uint32_t VOLT_OV_INSTANT_MS = 0UL;

static constexpr float VOLT_UV_CANDIDATE_RAW_MIN_V = 145.0f;
static constexpr float VOLT_RECOVER_MIN_V = 212.0f;
static constexpr float VOLT_RECOVER_MAX_V = 250.0f;
static constexpr uint32_t VOLTAGE_RECLOSE_STABLE_MS = 5UL * 60UL * 1000UL;

static constexpr float OVERLOAD_WARN_A      = 10.0f;
static constexpr float SUSTAINED_OVERLOAD_TRIP_A = 14.5f;
static constexpr uint32_t SUSTAINED_OVERLOAD_TRIP_MS = 60UL * 1000UL;

static constexpr float TEMP_WARN_C          = 60.0f;
static constexpr float TEMP_TRIP_C          = 70.0f;
static constexpr float TEMP_DATA_WARN_C     = 70.0f;
static constexpr float TEMP_DATA_HARD_C     = 80.0f;

static constexpr float MAINS_PRESENT_OFF_V  = 12.0f;
static constexpr float MAINS_PRESENT_ON_V   = 28.0f;
static constexpr uint32_t MAINS_EDGE_DEBOUNCE_MS = 1000UL;
static constexpr uint32_t UNPLUGGED_BUZZ_DELAY_MS = 5000UL;
static constexpr uint32_t UNPLUGGED_STATE_DELAY_MS = 5000UL;
static constexpr float VOLTAGE_SNAP_ZERO_V = 50.0f;
static constexpr float VOLTAGE_SNAP_RESTORE_V = 200.0f;

// =========================
// Leaky integrator / fault display hold
// =========================
static constexpr int ARC_CNT_INC  = 1;
static constexpr int ARC_CNT_DEC  = 2;
static constexpr int ARC_CNT_TRIP = 6;
static constexpr int ARC_CNT_MAX  = 20;
static constexpr uint32_t ARC_HOLD_MS  = 800;
static constexpr uint32_t HEAT_HOLD_MS = 1200;
static constexpr uint32_t FAULT_ALERT_MIN_MS = 3000UL;
static constexpr uint32_t FAULT_BUZZ_MS      = 3000UL;
static constexpr uint32_t FAULT_NET_QUIET_MS = 2350UL;
static constexpr uint32_t OLED_RENDER_INTERVAL_MS = 33UL;

// Sensing pipeline / timing quality
static constexpr uint8_t  FEATURE_FRAME_QUEUE_LEN        = 12;
static constexpr uint32_t FEATURE_TIMING_GRACE_MS        = 1500UL;
static constexpr float    FRAME_DT_BAD_EARLY_MS          = 70.0f;
static constexpr float    FRAME_DT_BAD_LATE_MS           = 170.0f;
static constexpr float    FRAME_DT_JITTER_BAD_MS         = 48.0f;
static constexpr float    FRAME_COMPUTE_BAD_MS           = 65.0f;
static constexpr float    FS_ERR_BAD_HZ                  = 3200.0f;


static constexpr int   ARC_RUNTIME_FEATURE_SPACE_VERSION = 5;
static constexpr float DB_RATIO_EPS                 = 1e-6f;
static constexpr float DB_POWER_RATIO_EPS           = 1e-6f;
static constexpr float DB_RATIO_CLIP_MIN            = -80.0f;
static constexpr float DB_RATIO_CLIP_MAX            = 20.0f;
static constexpr float DB_THD_CLIP_MIN              = 0.0f;
static constexpr float DB_THD_CLIP_MAX              = 200.0f;
static constexpr float DB_HF_DELTA_CLIP_MIN         = -18.0f;
static constexpr float DB_HF_DELTA_CLIP_MAX         = 18.0f;
static constexpr float HF_DELTA_MIN_BASELINE_SHARE  = 0.010f;
static constexpr float FEATURE_PERCENT_SCALE        = 100.0f;

static constexpr float ARC_SIG_SPECTRAL_FLUX       = 8.50f;     // percent
static constexpr float ARC_SIG_RESIDUAL_CF         = 12.568f;   // 20*log10(4.25)
static constexpr float ARC_SIG_EDGE_SPIKE_RATIO    = -14.894f;  // 20*log10(0.180)
static constexpr float ARC_SIG_MIDBAND_RATIO       = -21.412f;  // 20*log10(0.085)
static constexpr float ARC_SIG_CYCLE_NMSE          = 9.00f;     // percent
static constexpr float ARC_SIG_PEAK_FLUCT          = 1.20f;     // percent
static constexpr float ARC_SIG_THD_I               = 22.0f;     // percent THD
static constexpr float ARC_SIG_HF_ENERGY_DELTA     = 1.500f;    // 10*log10(power ratio), ~1.4x HF rise
static constexpr float ARC_SIG_ZCV                 = 0.200f;
static constexpr float ARC_SIG_IRMS_ZSCORE         = 2.35f;
static constexpr float ARC_SOFT_MIN_IRMS_A         = 0.08f;

static constexpr float BASELINE_STABLE_RESIDUAL_CF_DB    = 9.542f;   // 20*log10(3.0)
static constexpr float BASELINE_STABLE_EDGE_SPIKE_DB     = -20.000f; // 20*log10(0.10)
static constexpr float BASELINE_STABLE_MIDBAND_RATIO_DB  = -24.437f; // 20*log10(0.06)
static constexpr float BASELINE_STABLE_HF_DELTA_DB       = 0.80f;


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
static constexpr uint32_t RELAY_ARTIFACT_BLANK_MS = 2500UL;
static constexpr float    RELAY_ARTIFACT_FORCE_ZERO_A = 0.75f;
static constexpr uint32_t RELAY_ARTIFACT_SELF_HEAL_MS = 1200UL;

// Local assist when the user manually energizes a load while the MCU still thinks the relay is OFF.
// The assist should latch quickly for small loads, survive brief charger/SMPS dropouts, and only
// roll back during the provisional confirmation window if current disappears for long enough.
static constexpr float    MANUAL_RELAY_REARM_MIN_A = 0.08f;
static constexpr float    MANUAL_RELAY_REARM_RELEASE_A = 0.05f;
static constexpr uint32_t MANUAL_RELAY_REARM_DEBOUNCE_MS = 0UL;
static constexpr uint32_t MANUAL_RELAY_REARM_CONFIRM_MS = 10000UL;
static constexpr uint32_t MANUAL_RELAY_REARM_RELEASE_MS = 3000UL;
static constexpr uint32_t MANUAL_RELAY_REARM_COOLDOWN_MS = 2500UL;
static constexpr uint32_t MANUAL_RELAY_WEB_HOLDOFF_MS = 2200UL;
static constexpr uint32_t MANUAL_RELAY_REARM_BLANK_MS = 1400UL;

// Guard against isolated low-current NMSE blowups that can hit exactly 100% without any other arc-like evidence.
static constexpr float    CYCLE_NMSE_SOLO_ARTIFACT_MIN_PCT = 99.0f;
static constexpr float    CYCLE_NMSE_SOLO_ARTIFACT_MAX_IRMS_A = 0.35f;
static constexpr float    CYCLE_NMSE_SOLO_ARTIFACT_MAX_FLUX_PCT = 5.0f;
static constexpr float    CYCLE_NMSE_SOLO_ARTIFACT_MAX_RESIDUAL_CF_DB = 9.5f;
static constexpr float    CYCLE_NMSE_SOLO_ARTIFACT_MAX_EDGE_DB = -18.0f;
static constexpr float    CYCLE_NMSE_SOLO_ARTIFACT_MAX_MIDBAND_DB = -24.0f;
static constexpr float    CYCLE_NMSE_SOLO_ARTIFACT_MAX_PEAK_CV_PCT = 1.2f;
static constexpr float    CYCLE_NMSE_SOLO_ARTIFACT_MAX_HF_DELTA_DB = 0.9f;
static constexpr float    CYCLE_NMSE_SOLO_ARTIFACT_REPLACEMENT_PCT = 4.0f;

// =========================
// Logger / control polling
// =========================
static constexpr float    ARC_TURNON_LOW_A               = 0.20f;
static constexpr float    ARC_TURNON_ACTIVE_A            = 1.00f;
static constexpr uint32_t ARC_TURNON_LOW_MS              = 220UL;
static constexpr uint32_t ARC_TURNON_BLANK_MS            = 1600UL;

static constexpr uint32_t ARC_TRANSIENT_BLANK_MS         = 900UL;
static constexpr float    ARC_TRANSIENT_STEP_A           = 0.60f;
static constexpr float    ARC_TRANSIENT_STEP_FRAC        = 0.22f;

static constexpr uint32_t FEAT_STALE_MS                 = 350UL;
static constexpr uint32_t SENSOR_BOOT_SETTLE_MS         = 450UL;
static constexpr uint32_t PROTECTION_INHIBIT_MS         = 5000UL;
static constexpr uint32_t ML_CONTROL_POLL_MS            = 3000UL;
static constexpr uint32_t CLOUD_CONTROL_POLL_MS         = 1500UL;
static constexpr uint16_t AUTO_ARC_CAPTURE_DURATION_S   = 12;
static constexpr uint32_t AUTO_ARC_CAPTURE_COOLDOWN_MS  = 60000UL;
static constexpr int8_t ML_UNKNOWN_LABEL                = -1;
static constexpr uint16_t ML_LOG_DURATION_S             = 10;
static constexpr uint16_t ML_LOG_CHUNK_DURATION_S       = 10;
static constexpr uint16_t ML_LOG_MIN_DURATION_S         = 1;
static constexpr uint16_t ML_LOG_MAX_DURATION_S         = 7200;
static constexpr uint32_t ML_LOG_IDLE_ARM_GRACE_MS      = 8000UL;
static constexpr uint32_t ML_LOG_SETTLE_EXCLUDE_MS      = 1200UL;
static constexpr uint8_t  ML_LOG_SETTLE_GOOD_FRAMES     = 4;
static constexpr float    ML_LOG_SETTLE_FRAME_DT_MIN_MS = 70.0f;
static constexpr float    ML_LOG_SETTLE_FRAME_DT_MAX_MS = 130.0f;
static constexpr float    ML_LOG_SETTLE_COMPUTE_MAX_MS  = 55.0f;
static constexpr uint16_t ML_LOG_AUTO_MIN_DURATION_S    = 5;
static constexpr uint16_t ML_LOG_AUTO_MAX_DURATION_S    = 60;

static constexpr uint32_t CLOUD_LIVE_NORMAL_INTERVAL_MS = 5000UL;
static constexpr uint32_t CLOUD_LIVE_FAULT_INTERVAL_MS  = 1200UL;
static constexpr uint32_t CLOUD_REFRESH_KEEPALIVE_MS    = 1000UL;

// =========================
// Current backend (MCP3204 only)
// =========================
static constexpr uint8_t  MCP3204_CHANNEL        = 0;
static constexpr uint32_t MCP3204_SPI_HZ         = 1200000UL;
static constexpr uint8_t  MCP3204_OVERSAMPLE     = 1;
static constexpr uint8_t  MCP3204_MEDIAN_SAMPLES = 1;
static constexpr uint16_t MCP3204_STARTUP_FLUSH  = 256;
static constexpr uint8_t  MCP3204_WARMUP_BURSTS  = 2;
static constexpr uint8_t  MCP3204_BURST_FLUSH    = 4;
static constexpr float    MCP3204_FS_WARN_LOW_HZ = 23000.0f;
static constexpr float    MCP3204_FS_WARN_HIGH_HZ = 31000.0f;
static constexpr float    MCP3204_FS_HARD_LOW_HZ = 20000.0f;
static constexpr float    MCP3204_FS_HARD_HIGH_HZ = 34000.0f;

static constexpr float IRMS_GATE_ON_A               = 0.050f;
static constexpr float IRMS_GATE_OFF_A              = 0.025f;
static constexpr float CURRENT_IDLE_SUPPRESS_A      = 0.040f;
static constexpr float FEATURE_MIN_VRMS             = 70.0f;
static constexpr float FEATURE_MIN_IRMS_A           = 0.050f;
static constexpr float ARC_MIN_IRMS_A               = 0.060f;
static constexpr float FEATURE_REQUIRE_FUND_BELOW_A = 0.180f;
static constexpr float CURRENT_LPF_HZ               = 2000.0f;
static constexpr float CURRENT_BASE_LPF_HZ          = 240.0f;
static constexpr float ZC_HYS_MIN_A                 = 0.035f;
static constexpr float ZC_HYS_FRAC                  = 0.18f;
static constexpr float ZC_DWELL_THR_FRAC            = 0.10f;
static constexpr float ZC_DWELL_THR_MIN_A           = 0.035f;
static constexpr float PULSE_MIN_WIDTH_US           = 20.0f;
static constexpr float PULSE_MAX_WIDTH_US           = 500.0f;
static constexpr float PULSE_THRESH_RMS_MUL         = 7.0f;
static constexpr float PULSE_THRESH_MIN_A           = 0.120f;
static constexpr float MIDBAND_LO_HZ                = 220.0f;
static constexpr float MIDBAND_HI_HZ                = 3200.0f;
static constexpr float UPPERMID_LO_HZ               = 900.0f;
static constexpr float UPPERMID_HI_HZ               = 4500.0f;
static constexpr float HF_BAND_LO_HZ                = 4500.0f;
static constexpr float HF_BAND_HI_HZ                = 9000.0f;
static constexpr float SPEC_ENT_LO_HZ               = 220.0f;
static constexpr float SPEC_ENT_HI_HZ               = 9000.0f;
static constexpr float FUND_SNR_MIN                 = 6.0f;
static constexpr float FUND_MAG_MIN                 = 1e-5f;
static constexpr float SF_EPS                       = 1e-12f;
static constexpr int CURRENT_MIN_ACTIVITY_CHANGES   = 10;
static constexpr uint16_t CURRENT_MIN_CODE_SPAN     = 10;
static constexpr uint16_t LOW_CURRENT_CODE_SPAN     = 30;
static constexpr float CURRENT_FRAME_MIN_FS_HZ      = MCP3204_FS_HARD_LOW_HZ;
static constexpr float PULSE_ANALYSIS_MIN_IRMS_A    = 0.075f;
static constexpr float PULSE_ANALYSIS_MIN_RESID_A   = 0.035f;
static constexpr float CURRENT_DISPLAY_ON_A         = 0.040f;
static constexpr float CURRENT_DISPLAY_OFF_A        = 0.020f;
static constexpr float CURRENT_ANALYSIS_IDLE_A       = 0.020f;
static constexpr uint32_t CURRENT_IDLE_SUPPRESS_HOLD_MS = 5000UL;
static constexpr float ARC_WEAK_EVENT_RESID_MUL     = 0.75f;

static constexpr float SPECTRAL_FLUX_LO_HZ             = 900.0f;
static constexpr float SPECTRAL_FLUX_HI_HZ             = 9000.0f;
static constexpr int   SPECTRAL_FLUX_HARM_SKIP_BINS    = 2;
static constexpr float EDGE_SPIKE_WINDOW_MS            = 0.35f;
static constexpr float EDGE_SPIKE_MIN_BASELINE_A       = 0.10f;
static constexpr float BASELINE_MIN_IRMS_A             = 0.08f;
static constexpr float BASELINE_RESET_IRMS_A           = 0.03f;
static constexpr float BASELINE_STD_FLOOR_A            = 0.03f;
static constexpr float BASELINE_STD_FLOOR_FRAC         = 0.08f;
static constexpr float BASELINE_IRMS_ALPHA             = 0.08f;
static constexpr float BASELINE_HF_ALPHA               = 0.10f;
static constexpr float BASELINE_STEP_FREEZE_A          = 0.60f;
static constexpr float BASELINE_STEP_FREEZE_FRAC       = 0.22f;
static constexpr uint32_t BASELINE_FREEZE_MS           = 900UL;
static constexpr uint32_t BASELINE_RESET_IDLE_MS       = 1800UL;

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

static inline float ratio_to_db20(float ratio, float floor_ratio = DB_RATIO_EPS) {
  return 20.0f * log10f(fmaxf(ratio, floor_ratio));
}

static inline float ratio_to_db10(float ratio, float floor_ratio = DB_POWER_RATIO_EPS) {
  return 10.0f * log10f(fmaxf(ratio, floor_ratio));
}

static inline float db20_to_ratio(float db) {
  return powf(10.0f, db / 20.0f);
}

static inline float db10_to_ratio(float db) {
  return powf(10.0f, db / 10.0f);
}

static inline float thd_percent_to_db(float thd_percent) {
  return ratio_to_db20(fmaxf(thd_percent, 0.0f) * 0.01f);
}

static inline float db_to_thd_percent(float thd_db) {
  return 100.0f * db20_to_ratio(thd_db);
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
