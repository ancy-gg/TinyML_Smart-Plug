#include "FaultLogic.h"

static inline int clampi(int x, int lo, int hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}

void FaultLogic::resetLatch() {
  _latched = false;
  _latchedState = STATE_NORMAL;
  _arcCnt = 0;
  _heatFrames = 0;
}

FaultState FaultLogic::update(float tempC, float irmsA, int arcModelOut) {
  if (arcModelOut == 1) _arcCnt += ARC_CNT_INC;
  else _arcCnt -= ARC_CNT_DEC;
  _arcCnt = clampi(_arcCnt, 0, ARC_CNT_MAX);

  const bool arcTrip = (_arcCnt >= ARC_CNT_TRIP);

  if (tempC > TEMP_TRIP_C) _heatFrames++;
  else _heatFrames = clampi(_heatFrames - HEAT_FRAMES_DEC, 0, HEAT_FRAMES_TRIP);

  const bool heatTrip = (_heatFrames >= HEAT_FRAMES_TRIP);

#if USE_SOFT_LATCH
  if (_latched) return _latchedState;
  if (arcTrip)  { _latched = true; _latchedState = STATE_ARCING;  return _latchedState; }
  if (heatTrip) { _latched = true; _latchedState = STATE_HEATING; return _latchedState; }
#endif

  if (heatTrip) return STATE_HEATING;
  if (arcTrip)  return STATE_ARCING;
  if (irmsA > OVERLOAD_TRIP_A) return STATE_OVERLOAD;
  return STATE_NORMAL;
}