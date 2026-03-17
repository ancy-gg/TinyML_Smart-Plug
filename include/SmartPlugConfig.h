#pragma once
#include <Arduino.h>

// =========================
// Build / operating modes
// =========================
// #define USE_M2CGEN_RF
#define DATA_COLLECTION_MODE

#ifndef ENABLE_ML_LOGGER
#define ENABLE_ML_LOGGER 1
#endif

#define USE_SOFT_LATCH 0

// =========================
// Relay / buzzer hardware
// =========================
static constexpr bool RELAY_ACTIVE_LOW      = true;   // LOW = relay ON / AC conducted
static constexpr bool BUZZER_PASSIVE_PWM    = true;
static constexpr uint8_t BUZZER_PWM_BITS    = 8;
static constexpr uint8_t BUZZER_PWM_DUTY    = 128;    // 50% duty default for passive piezo
static constexpr bool BUZZER_STATUS_ENABLED = true;   // allow Wi-Fi / ready / OTA status chirps
static constexpr uint32_t BUZZER_STARTUP_MUTE_MS = 700;
static constexpr uint32_t SYSTEM_READY_CHIME_DELAY_MS = 1000;

// =========================
// Sampling and FFT
// =========================
static constexpr float    FS_TARGET_HZ = 125000.0f;
static constexpr uint16_t N_SAMP       = 4096;
static constexpr float    MAINS_F0_HZ  = 60.0f;

// =========================
// Protection thresholds
// =========================
static constexpr float OVERLOAD_WARN_A      = 10.0f;
static constexpr float OVERLOAD_HARD_TRIP_A = 15.0f;
static constexpr float OVERLOAD_TRIP_A      = OVERLOAD_WARN_A;

static constexpr float TEMP_TRIP_C          = 70.0f;
static constexpr float TEMP_DATA_WARN_C     = 80.0f;
static constexpr float TEMP_DATA_HARD_C     = 90.0f;
static constexpr float VOLT_SURGE_TRIP_V    = 260.0f;

static constexpr float MAINS_PRESENT_OFF_V  = 12.0f;
static constexpr float MAINS_PRESENT_ON_V   = 25.0f;

// Battery hold / outage shutdown
static constexpr uint32_t OUTAGE_SHUTDOWN_MS         = 10UL * 60UL * 1000UL;
static constexpr uint32_t OUTAGE_BOOT_ARM_MS         = 30000UL; // ignore outage countdown during early boot/filter settle
static constexpr uint32_t OUTAGE_PRESENT_DEBOUNCE_MS = 3000UL;
static constexpr uint32_t OUTAGE_ABSENT_DEBOUNCE_MS  = 5000UL;

// =========================
// Arc decision thresholds
// =========================
static constexpr float ENTROPY_ARC_H   = 0.82f;
static constexpr float THD_ARC_H_PCT   = 100.0f;
static constexpr float ZCV_ARC_H_MS    = 2.0f;
static constexpr float HF_RATIO_ARC_H  = 0.24f;
static constexpr float HF_VAR_ARC_H    = 0.0060f;
static constexpr float ARC_MIN_IRMS_A  = 0.03f;
static constexpr float SF_ARC_H        = 0.62f;
static constexpr float CYC_VAR_ARC_H   = 0.030f;

static constexpr float HF_BAND_LO_HZ = 2000.0f;
static constexpr float HF_BAND_HI_HZ = 20000.0f;
static constexpr float LF_BAND_HI_HZ = 1000.0f;

// =========================
// Feature gating / transient control
// =========================
static constexpr float IDLE_IRMS_A        = 0.02f;
static constexpr float FUND_SNR_MIN       = 6.0f;
static constexpr float FUND_MAG_MIN       = 1e-4f;
static constexpr float ZC_HYS_FRAC        = 0.30f;
static constexpr float ZC_HYS_MIN_A       = 0.01f;
static constexpr float ENTROPY_MAX_HZ     = 50000.0f;
static constexpr float SF_EPS             = 1e-12f;
static constexpr float THD_GUARD_PCT      = 180.0f;
static constexpr float HF_VAR_GUARD_LO    = 0.0020f;
static constexpr float CYC_VAR_GUARD_LO   = 0.015f;
static constexpr uint32_t FEAT_STALE_MS   = 350;
static constexpr uint32_t ML_CTRL_POLL_MS = 10000;

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

static constexpr int PIN_RELAY      = D7;
static constexpr int PIN_BUZZER_PWM = D2;

// =========================
// ADS8684
// =========================
static constexpr float ADS_VREF_V = 4.096f;
static constexpr int   ADS_SPI_HZ = 4000000;

// =========================
// Current calibration
// =========================
struct CurrentCalib {
  // Divider return is already applied in ArcFeatures.cpp using dividerRatio.
  // If clamp-vs-device current still disagrees after re-test, tune ampsScale using:
  // ampsScale_new = ampsScale_old * (clamp_meter_A / device_reported_A)
  float dividerRatio = 4.096f / 5.0f;
  float offsetV      = 2.5f;
  float voltsPerAmp  = 0.100f;
  float ampsScale    = 0.790f;
};

struct VoltageCalib {
  float sensitivity = 580.0f;
};

// =========================
// Logging
// =========================
static constexpr uint16_t ML_LOG_RATE_HZ     = 30;
static constexpr uint16_t ML_LOG_DURATION_S  = 10;
