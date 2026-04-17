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
  if (!_socketModelInit) {
    _socketModelInit = true;
    _socketDeltaC = 0.0f;
    _lastSocketModelMs = now;
    return clampf_ts(ntcTempC, TEMP_SOCKET_EST_MIN_C, TEMP_SOCKET_EST_MAX_C);
  }

  float dtS = (now > _lastSocketModelMs) ? ((now - _lastSocketModelMs) / 1000.0f) : 0.25f;
  _lastSocketModelMs = now;
  if (dtS < 0.05f) dtS = 0.05f;
  if (dtS > 5.0f)  dtS = 5.0f;

  const float absI = isfinite(irmsA) ? fabsf(irmsA) : 0.0f;
  float deltaTarget = 0.0f;

  if (mainsPresent && absI >= TEMP_SOCKET_LOAD_ON_A) {
    const float curFrac = clampf_ts(
        (absI - TEMP_SOCKET_LOAD_ON_A) / (TEMP_SOCKET_CURRENT_DEAD_A - TEMP_SOCKET_LOAD_ON_A),
        0.0f, 1.0f);
    float iEx = absI - TEMP_SOCKET_CURRENT_DEAD_A;
    if (iEx < 0.0f) iEx = 0.0f;

    deltaTarget = (TEMP_SOCKET_BASE_DELTA_C * curFrac)
                + (TEMP_SOCKET_IEXCESS2_GAIN_C_PER_A2 * iEx * iEx);
    deltaTarget = clampf_ts(deltaTarget, 0.0f, TEMP_SOCKET_MAX_DELTA_C);
  }

  const bool coolingMode = (!mainsPresent) || (absI <= TEMP_SOCKET_LOAD_OFF_A);
  const float tauS = (deltaTarget > _socketDeltaC) ? TEMP_SOCKET_HEAT_TAU_S
                                                   : TEMP_SOCKET_COOL_TAU_S;
  const float alpha = 1.0f - expf(-dtS / fmaxf(0.10f, tauS));
  _socketDeltaC += alpha * (deltaTarget - _socketDeltaC);

  if (coolingMode && fabsf(_socketDeltaC) < 0.02f) {
    _socketDeltaC = 0.0f;
  }

  const float estimated = ntcTempC + _socketDeltaC;
  return clampf_ts(estimated, TEMP_SOCKET_EST_MIN_C, TEMP_SOCKET_EST_MAX_C);
}
