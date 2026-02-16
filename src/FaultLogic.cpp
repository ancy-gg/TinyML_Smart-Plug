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
  if (_latched) return _latchedState;

  // Leaky integrator for arc
  if (arcModelOut == 1) _arcCnt += ARC_CNT_INC;
  else _arcCnt -= ARC_CNT_DEC;
  _arcCnt = clampi(_arcCnt, 0, ARC_CNT_MAX);

  if (_arcCnt >= ARC_CNT_TRIP) {
    _latched = true;
    _latchedState = STATE_ARCING;
    return _latchedState;
  }

  // Heating debounce then latch
  if (tempC > TEMP_TRIP_C) _heatFrames++;
  else _heatFrames = clampi(_heatFrames - HEAT_FRAMES_DEC, 0, HEAT_FRAMES_TRIP);

  if (_heatFrames >= HEAT_FRAMES_TRIP) {
    _latched = true;
    _latchedState = STATE_HEATING;
    return _latchedState;
  }

  // Overload (not latched)
  if (irmsA > OVERLOAD_TRIP_A) return STATE_OVERLOAD;

  return STATE_NORMAL;
}