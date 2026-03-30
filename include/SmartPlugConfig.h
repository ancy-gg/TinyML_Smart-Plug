#pragma once
#include <Arduino.h>
#include <math.h>
#include "CurrentBackend.h"

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
// Fan control
// =========================
static constexpr bool  FAN_FORCE_MAX_TEST       = false;   // test mode: keep fan fully on
static constexpr bool  FAN_BYPASS_PWM_WHEN_FORCED = true;  // cheap 2-wire fans often prefer DC, not PWM
static constexpr float FAN_MIN_TEMP_C            = 25.0f;
static constexpr float FAN_MAX_TEMP_C            = 50.0f;
static constexpr uint32_t FAN_PWM_HZ             = 25000UL;
static constexpr uint8_t FAN_PWM_BITS            = 8;
static constexpr uint8_t FAN_MIN_SPIN_DUTY       = 96;
static constexpr uint32_t FAN_START_KICK_MS      = 1200UL;

// =========================
// Sampling and FFT
// =========================
#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684
static constexpr float    FS_TARGET_HZ = 80000.0f;
#else
static constexpr float    FS_TARGET_HZ = 35000.0f;
#endif
static constexpr uint16_t N_SAMP       = 4096;
static constexpr float    MAINS_F0_HZ  = 60.0f;


// Software anti-alias / cleanup for current waveform.
// Effective cutoff is clamped below Nyquist at runtime.
static constexpr bool     CURRENT_SOFT_AAF_ENABLE = true; // ADS-only in practice; MCP bypasses this in ArcFeatures.cpp
#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684
static constexpr float    CURRENT_SOFT_AAF_CUTOFF_HZ = 25000.0f;
#else
static constexpr float    CURRENT_SOFT_AAF_CUTOFF_HZ = 15000.0f;
#endif
static constexpr float    CURRENT_SOFT_AAF_Q = 0.70710678f;
#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684
static constexpr float    CURRENT_SOFT_AAF_MAX_FRAC_NYQUIST = 0.95f;
#else
static constexpr float    CURRENT_SOFT_AAF_MAX_FRAC_NYQUIST = 0.98f;
#endif

// =========================
// Wi-Fi startup / portal
// =========================
static constexpr uint32_t WIFI_BOOT_BLOCK_MS      = 10000UL;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 10000UL;
static constexpr uint32_t WIFI_PORTAL_TIMEOUT_MS  = 30000UL;
static constexpr uint32_t WIFI_RETRY_INTERVAL_MS  = 15000UL;
static constexpr uint32_t WIFI_PORTAL_AP_GRACE_MS = 1500UL;
static constexpr uint32_t WIFI_CONTROL_POLL_MS    = 5000UL;
static constexpr uint8_t  WIFI_TRIPLE_TAP_COUNT   = 2;
static constexpr bool     WIFI_OPEN_PORTAL_ON_TIMEOUT = true;
static constexpr const char* WIFI_PORTAL_SSID = "TinyML-SmartPlug";

// =========================
// Protection thresholds
// =========================
static constexpr float OVERLOAD_WARN_A      = 10.0f;
static constexpr float OVERLOAD_HARD_TRIP_A = 15.0f;
static constexpr float OVERLOAD_TRIP_A      = OVERLOAD_WARN_A;

static constexpr float TEMP_TRIP_C          = 70.0f;
static constexpr float TEMP_DATA_WARN_C     = 80.0f;
static constexpr float TEMP_DATA_HARD_C     = 90.0f;

static constexpr float VOLT_UNDERVOLT_MIN_V = 100.0f;
static constexpr float VOLT_UNDERVOLT_MAX_V = 200.0f;
static constexpr float VOLT_OVERVOLT_TRIP_V = 250.0f;
static constexpr float VOLT_SURGE_TRIP_V    = VOLT_OVERVOLT_TRIP_V;

static constexpr float MAINS_PRESENT_OFF_V  = 12.0f;
static constexpr float MAINS_PRESENT_ON_V   = 25.0f;
static constexpr uint32_t MAINS_EDGE_DEBOUNCE_MS = 1000UL;
static constexpr uint32_t UNPLUGGED_BUZZ_DELAY_MS = 2000UL;
static constexpr uint32_t UNPLUGGED_STATE_DELAY_MS = 3000UL;

// =========================
// Leaky integrator
// =========================
static constexpr int ARC_CNT_INC  = 1;
static constexpr int ARC_CNT_DEC  = 2;
static constexpr int ARC_CNT_TRIP = 10;
static constexpr int ARC_CNT_MAX  = 20;
static constexpr uint32_t ARC_HOLD_MS  = 800;
static constexpr uint32_t HEAT_HOLD_MS = 1200;
static constexpr uint32_t FAULT_ALERT_MIN_MS = 2000UL;
static constexpr uint32_t UNDERVOLT_TRIP_MS = 350UL;
static constexpr uint32_t OVERVOLT_TRIP_MS  = 120UL;
static constexpr uint32_t OVERLOAD_TRIP_MS  = 250UL;

static constexpr int HEAT_FRAMES_TRIP = 6;
static constexpr int HEAT_FRAMES_DEC  = 1;

// =========================
// Pins (XIAO ESP32S3)
// =========================
static constexpr int PIN_VOLT_ADC    = D0;
static constexpr int PIN_TEMP_ADC    = D1;
static constexpr int PIN_FAN_PWM     = D6;

static constexpr int PIN_ADC_CS   = D3;
static constexpr int PIN_ADC_SCK  = D8;
static constexpr int PIN_ADC_MISO = D9;
static constexpr int PIN_ADC_MOSI = D10;

static constexpr int PIN_RELAY      = D7;
static constexpr int PIN_BUZZER_PWM = D2;

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

static inline float eval_cubic_signed_mag(float x, float c3, float c2, float c1, float c0) {
  const float mag = fabsf(x);
  float y = eval_cubic_horner(mag, c3, c2, c1, c0);
  if (y < 0.0f) y = 0.0f;
  return (x < 0.0f) ? -y : y;
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

// =========================
// Logging
// =========================
static constexpr uint16_t ML_LOG_RATE_HZ     = 30;
static constexpr uint16_t ML_LOG_DURATION_S  = 10;

// =========================
// Current display cleanup
// =========================
static constexpr bool     CURRENT_IDLE_LEARN_ENABLE      = false;
static constexpr uint32_t CURRENT_IDLE_LEARN_WINDOW_MS   = 15000UL;
static constexpr float    CURRENT_IDLE_LEARN_MAX_A       = 0.35f;
static constexpr uint16_t CURRENT_IDLE_LEARN_MIN_FRAMES  = 8;
static constexpr float    CURRENT_IDLE_FLOOR_MARGIN_A    = 0.015f;
static constexpr float    CURRENT_DISPLAY_GATE_ON_A      = 0.060f;
static constexpr float    CURRENT_DISPLAY_GATE_OFF_A     = 0.030f;
static constexpr float    CURRENT_DISPLAY_SMOOTH_TAU_LOW_S  = 0.45f;
static constexpr float    CURRENT_DISPLAY_SMOOTH_TAU_HIGH_S = 0.18f;
static constexpr float    CURRENT_DISPLAY_SMOOTH_SPLIT_A    = 1.00f;

static constexpr uint32_t DEVICE_PLUG_CUE_INHIBIT_MS = 2000UL;
static constexpr uint32_t DEVICE_PLUG_CUE_COOLDOWN_MS = 6000UL;
static constexpr uint32_t DEVICE_PLUG_STABLE_MS = 1200UL;
