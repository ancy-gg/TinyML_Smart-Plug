#include "FaultLogic.h"

static inline int clampi(int x, int lo, int hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}

void FaultLogic::resetLatch() {
  _latched = false;
  _latchedState = STATE_NORMAL;
  _arcCnt = 0;
  _heatFrames = 0;
  _arcHoldUntil = 0;
  _heatHoldUntil = 0;
}

FaultState FaultLogic::update(float tempC, float irmsA, int arcModelOut) {
  const uint32_t now = millis();

  // Extra safety: don’t accumulate arc when current is tiny
  if (irmsA < ARC_MIN_IRMS_A) arcModelOut = 0;

  if (arcModelOut == 1) _arcCnt += ARC_CNT_INC;
  else                  _arcCnt -= ARC_CNT_DEC;
  _arcCnt = clampi(_arcCnt, 0, ARC_CNT_MAX);

  const bool arcTrip = (_arcCnt >= ARC_CNT_TRIP);

  if (tempC > TEMP_TRIP_C) _heatFrames++;
  else _heatFrames = clampi(_heatFrames - HEAT_FRAMES_DEC, 0, HEAT_FRAMES_TRIP);

  const bool heatTrip = (_heatFrames >= HEAT_FRAMES_TRIP);

  // NEW: hold timers to prevent flicker
  if (arcTrip)  _arcHoldUntil  = now + ARC_HOLD_MS;
  if (heatTrip) _heatHoldUntil = now + HEAT_HOLD_MS;

  const bool arcActive  = arcTrip  || (now < _arcHoldUntil);
  const bool heatActive = heatTrip || (now < _heatHoldUntil);

#if USE_SOFT_LATCH
  if (_latched) return _latchedState;
  if (arcActive)  { _latched = true; _latchedState = STATE_ARCING;  return _latchedState; }
  if (heatActive) { _latched = true; _latchedState = STATE_HEATING; return _latchedState; }
#endif

  if (heatActive) return STATE_HEATING;
  if (arcActive)  return STATE_ARCING;
  if (irmsA > OVERLOAD_TRIP_A) return STATE_OVERLOAD;
  return STATE_NORMAL;
}