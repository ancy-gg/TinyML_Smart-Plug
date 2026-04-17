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

float TempSensor::estimateSocketTempC(float ntcTempC, float irmsA, bool mainsPresent) {
  if (!isfinite(ntcTempC)) return 0.0f;

  const uint32_t now = millis();
  float dtS = 0.5f;
  if (_socketModelInit && now > _lastSocketModelMs) {
    dtS = (now - _lastSocketModelMs) / 1000.0f;
  }
  _lastSocketModelMs = now;
  if (dtS < 0.05f) dtS = 0.05f;
  if (dtS > 2.0f)  dtS = 2.0f;

  if (!_ambientInit) {
    _ambientInit = true;
    _ambientEstC = ntcTempC;
  }

  const float currentA = (isfinite(irmsA) && irmsA > 0.0f) ? irmsA : 0.0f;
  const bool loadOn = mainsPresent && (currentA >= TEMP_SOCKET_MODEL_LOAD_ON_A);
  if (!loadOn || currentA <= TEMP_SOCKET_AMBIENT_TRACK_MAX_A) {
    const float alphaAmbient = fminf(1.0f, dtS / TEMP_SOCKET_AMBIENT_TRACK_TAU_S);
    _ambientEstC += alphaAmbient * (ntcTempC - _ambientEstC);
  }

  float currentRatio = 0.0f;
  if (loadOn && TEMP_SOCKET_NORMAL_IREF_A > 0.1f) {
    currentRatio = currentA / TEMP_SOCKET_NORMAL_IREF_A;
    currentRatio = clampf_ts(currentRatio, 0.0f, TEMP_SOCKET_NORMAL_MAX_RATIO);
  }
  const float currentShape = currentRatio * currentRatio;

  const float normalTargetRiseC = loadOn
      ? (TEMP_SOCKET_NORMAL_BASE_RISE_C + TEMP_SOCKET_NORMAL_GAIN_C * currentShape)
      : 0.0f;
  const float socketMeasuredDeltaC = loadOn
      ? (TEMP_SOCKET_MEASURED_BASE_DELTA_C + TEMP_SOCKET_MEASURED_GAIN_C * currentShape)
      : 0.0f;

  const float tauS = (normalTargetRiseC >= _normalRiseC)
      ? TEMP_SOCKET_HEAT_TAU_S
      : TEMP_SOCKET_COOL_TAU_S;
  const float alpha = fminf(1.0f, dtS / fmaxf(0.25f, tauS));

  if (!_socketModelInit) {
    _socketModelInit = true;
    _normalRiseC = normalTargetRiseC;
  } else {
    _normalRiseC += alpha * (normalTargetRiseC - _normalRiseC);
  }

  _expectedNormalC = clampf_ts(_ambientEstC + _normalRiseC, TEMP_SOCKET_EST_MIN_C, TEMP_SOCKET_EST_MAX_C);
  const float measuredEstimateC = clampf_ts(ntcTempC + socketMeasuredDeltaC, TEMP_SOCKET_EST_MIN_C, TEMP_SOCKET_EST_MAX_C);
  _socketEstimateC = clampf_ts(fmaxf(measuredEstimateC, _expectedNormalC), TEMP_SOCKET_EST_MIN_C, TEMP_SOCKET_EST_MAX_C);
  _socketExcessC = clampf_ts(_socketEstimateC - _expectedNormalC, 0.0f, 80.0f);
  return _socketEstimateC;
}
