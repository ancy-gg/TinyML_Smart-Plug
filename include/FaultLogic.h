#pragma once
#include <Arduino.h>
#include "SmartPlugTypes.h"
#include "SmartPlugConfig.h"

class FaultLogic {
public:
  void resetLatch();

  // Returns current state (latched heating/arc, non-latched overload)
  FaultState update(float tempC, float irmsA, int arcModelOut);

  int  arcCounter() const { return _arcCnt; }
  bool latched() const { return _latched; }

private:
  bool _latched = false;
  FaultState _latchedState = STATE_NORMAL;

  int _arcCnt = 0;
  int _heatFrames = 0;

  // NEW: state hold timers to prevent rapid NORMAL<->ARCING flicker
  uint32_t _arcHoldUntil  = 0;
  uint32_t _heatHoldUntil = 0;
};