#include "TempSensor.h"
#include <math.h>

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
  steinhart /= B_COEFF;
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
