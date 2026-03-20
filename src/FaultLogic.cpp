#include "FaultLogic.h"

static inline int clampi(int x, int lo, int hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}

static inline float heatTripTempC() {
#if COLLECTION_ONLY_MODE
  return TEMP_DATA_WARN_C;
#else
  return TEMP_TRIP_C;
#endif
}

void FaultLogic::resetLatch() {
  _arcCnt = 0;
  _heatFrames = 0;
  _arcHoldUntil = 0;
  _heatHoldUntil = 0;
  _underVoltSince = 0;
  _overVoltSince = 0;
  _overloadSince = 0;
}

FaultState FaultLogic::update(float vProtect, float tempC, float irmsA, int arcModelOut, bool arcEligible) {
  const uint32_t now = millis();

  if (!arcEligible || irmsA < ARC_MIN_IRMS_A) arcModelOut = 0;

  if (arcModelOut == 1) _arcCnt += ARC_CNT_INC;
  else                  _arcCnt -= ARC_CNT_DEC;
  _arcCnt = clampi(_arcCnt, 0, ARC_CNT_MAX);

  const bool arcTrip = (_arcCnt >= ARC_CNT_TRIP);

  if (tempC >= heatTripTempC()) _heatFrames++;
  else _heatFrames = clampi(_heatFrames - HEAT_FRAMES_DEC, 0, HEAT_FRAMES_TRIP);

  const bool heatTrip = (_heatFrames >= HEAT_FRAMES_TRIP);

  if (arcTrip)  _arcHoldUntil  = now + ARC_HOLD_MS;
  if (heatTrip) _heatHoldUntil = now + HEAT_HOLD_MS;

  const bool arcActive  = arcTrip  || (now < _arcHoldUntil);
  const bool heatActive = heatTrip || (now < _heatHoldUntil);

  const bool underVoltRaw = (vProtect > VOLT_UNDERVOLT_MIN_V && vProtect < VOLT_UNDERVOLT_MAX_V);
  const bool overVoltRaw  = (vProtect >= VOLT_OVERVOLT_TRIP_V);
  const bool overloadRaw  = (irmsA >= OVERLOAD_WARN_A);

  if (underVoltRaw) {
    if (_underVoltSince == 0) _underVoltSince = now;
  } else {
    _underVoltSince = 0;
  }

  if (overVoltRaw) {
    if (_overVoltSince == 0) _overVoltSince = now;
  } else {
    _overVoltSince = 0;
  }

  if (overloadRaw) {
    if (_overloadSince == 0) _overloadSince = now;
  } else {
    _overloadSince = 0;
  }

  const bool underVoltActive = underVoltRaw && ((now - _underVoltSince) >= UNDERVOLT_TRIP_MS);
  const bool overVoltActive  = overVoltRaw  && ((now - _overVoltSince)  >= OVERVOLT_TRIP_MS);
  const bool overloadActive  = overloadRaw  && ((now - _overloadSince)  >= OVERLOAD_TRIP_MS);

  if (heatActive)      return STATE_HEATING;
  if (arcActive)       return STATE_ARCING;
  if (overVoltActive)  return STATE_OVERVOLTAGE;
  if (underVoltActive) return STATE_UNDERVOLTAGE;
  if (overloadActive)  return STATE_OVERLOAD;
  return STATE_NORMAL;
}
