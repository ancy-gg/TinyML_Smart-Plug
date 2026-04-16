#include "TempSensor.h"
#include <math.h>

static inline float clampf_ts(float x, float lo, float hi) {
  return (x < lo) ? lo : ((x > hi) ? hi : x);
}

TempSensor::TempSensor(int pin) {
  _pin = pin;
}

void TempSensor::begin() {
  pinMode(_pin, INPUT);
#if defined(ARDUINO_ARCH_ESP32)
  analogSetPinAttenuation(_pin, ADC_11db);
#endif
}

float TempSensor::readTempC() {
  uint32_t mvSum = 0;
  constexpr int N = 5;
  for (int i = 0; i < N; i++) {
    mvSum += (uint32_t)analogReadMilliVolts(_pin);
  }

  const float vOut = (mvSum / (float)N) / 1000.0f;

  if (vOut < 0.1f) return -99.0f;

  float vClamped = vOut;
  if (vClamped >= VCC - 0.1f) vClamped = VCC - 0.1f;

  const float rNtc = (vClamped * R_DIVIDER) / (VCC - vClamped);

  float steinhart = rNtc / R_NTC_NOMINAL;
  steinhart = logf(steinhart);
  steinhart /= TEMP_NTC_BETA;
  steinhart += 1.0f / (TEMP_NOMINAL + 273.15f);
  steinhart = 1.0f / steinhart;
  steinhart -= 273.15f;

  const uint32_t now = millis();
  if (!_filtInit) {
    _filtInit = true;
    _filtTempC = steinhart;
    _lastReadMs = now;
    return _filtTempC;
  }

  float dtS = (now > _lastReadMs) ? ((now - _lastReadMs) / 1000.0f) : 0.5f;
  _lastReadMs = now;
  if (dtS < 0.05f) dtS = 0.05f;
  if (dtS > 2.0f)  dtS = 2.0f;

  if (fabsf(steinhart - _filtTempC) >= _avgJumpC) {
    _filtTempC = steinhart;
  } else {
    const float alpha = fminf(1.0f, dtS / _avgTauS);
    _filtTempC += alpha * (steinhart - _filtTempC);
  }

  return _filtTempC;
}

float TempSensor::estimateSocketTempC(float ntcTempC) const {
  if (!isfinite(ntcTempC)) return 0.0f;

  // Calibration-ready behavior:
  // - no current-based blending or IRMS curvature
  // - default to raw NTC temperature until a trusted socket calibration is ready
  // - optional polynomial curve can be enabled later in MainConfiguration.h
  if (!TEMP_SOCKET_USE_CALIBRATION_CURVE) {
    return clampf_ts(ntcTempC, TEMP_SOCKET_EST_MIN_C, TEMP_SOCKET_EST_MAX_C);
  }

  // When a future calibration is enabled, avoid low-side extrapolation outside
  // the calibrated NTC region.
  if (ntcTempC < TEMP_SOCKET_CURVE_MIN_NTC_C) {
    return clampf_ts(ntcTempC, TEMP_SOCKET_EST_MIN_C, TEMP_SOCKET_EST_MAX_C);
  }

  const float centered = ntcTempC - TEMP_SOCKET_CURVE_REF_C;
  const float fitted = TEMP_SOCKET_CURVE_C0
                     + (TEMP_SOCKET_CURVE_C1 * centered)
                     + (TEMP_SOCKET_CURVE_C2 * centered * centered);
  return clampf_ts(fitted, TEMP_SOCKET_EST_MIN_C, TEMP_SOCKET_EST_MAX_C);
}
