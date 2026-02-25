#pragma once
#include <Arduino.h>

// Data Collection Mode
#define DATA_COLLECTION_MODE //recomment when done

// TinyML Mode
//#define USE_M2CGEN_RF

#define ENABLE_ML_LOGGER 1

// Sampling and FFT
static constexpr float   FS_TARGET_HZ = 125000.0f;
static constexpr uint16_t N_SAMP      = 4096;
static constexpr float   MAINS_F0_HZ  = 60.0f;

// Constant Thresholds
static constexpr float TEMP_TRIP_C     = 70.0f;
static constexpr float OVERLOAD_TRIP_A = 10.0f;

// ========= Arc decision thresholds (baseline) =========
static constexpr float ENTROPY_ARC_H   = 0.75f;  // 0..1
static constexpr float THD_ARC_H_PCT   = 100.0f;  // %
static constexpr float ZCV_ARC_H_MS    = 2.0f;  // ms

// ========= Feature gating / transient control =========
static constexpr float IDLE_IRMS_A      = 0.02f;   // idle gate just above your ~0.02A noise
static constexpr float FUND_SNR_MIN     = 6.0f;    // fundamental must be >= 6x avg noise mag
static constexpr float FUND_MAG_MIN     = 1e-4f;   // absolute magnitude guard
static constexpr float ZC_HYS_FRAC      = 0.30f;   // hysteresis = 30% of Irms
static constexpr float ZC_HYS_MIN_A     = 0.01f;   // minimum hysteresis
static constexpr float ENTROPY_MAX_HZ   = 50000.0f; // entropy band limit

static constexpr uint32_t ML_CTRL_POLL_MS = 10000; // ESP polls /ml_log every 10s

// ========= Leaky integrator =========
static constexpr int ARC_CNT_INC  = 2;
static constexpr int ARC_CNT_DEC  = 1;
static constexpr int ARC_CNT_TRIP = 6;  // “>5” means trip at 6
static constexpr int ARC_CNT_MAX  = 12;

// ========= Heating latch debounce =========
static constexpr int HEAT_FRAMES_TRIP = 6;
static constexpr int HEAT_FRAMES_DEC  = 1;

// ========= Pins (YOUR EXACT WIRING) =========
// ESP32 ADC (12-bit)
static constexpr int PIN_VOLT_ADC  = D0;   // ZMPT101B
static constexpr int PIN_TEMP_ADC  = D1;   // NTC divider
static constexpr int PIN_RESET_BTN = D6;   // freed GPIO

// ADS8684 SPI (Current sensor AUX, 16-bit)
static constexpr int PIN_ADC_CS   = D7;
static constexpr int PIN_ADC_SCK  = D8;
static constexpr int PIN_ADC_MISO = D9;
static constexpr int PIN_ADC_MOSI = D10;

// Outputs (edit only if your actual board wiring differs)
static constexpr int PIN_RELAY      = D2;
static constexpr int PIN_BUZZER_PWM = D3;

// ========= ADS8684 =========
static constexpr float ADS_VREF_V = 4.096f;
static constexpr int   ADS_SPI_HZ = 4000000;  // ~125kSPS theoretical (SPI/32 clocks)

// ========= Current calibration (you will tune later) =========
struct CurrentCalib {
  // You stated: 5V scaled to 4.096V at AUX => ratio = 4.096/5
  float dividerRatio = 4.096f / 5.0f;

  // Sensor output at 0A (measure this!)
  float offsetV = 2.5f;

  // Sensor sensitivity (V/A) (replace for your ACS37035 variant!)
  float voltsPerAmp = 0.100f; //100mV/A
};

// ========= Voltage calibration =========
struct VoltageCalib {
  // Your current VoltageSensor uses a multiplier “sensitivity”
  float sensitivity = 580.0f; // tune this with a multimeter
};

// ========= Logger =========
static constexpr uint16_t ML_LOG_RATE_HZ      = 30;
static constexpr uint16_t ML_LOG_DURATION_S  = 10; // 5–10 typical