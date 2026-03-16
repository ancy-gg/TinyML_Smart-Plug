#pragma once
#include <Arduino.h>
#include "SmartPlugTypes.h"
#include "SmartPlugConfig.h"

class FaultLogic {
public:
  void resetLatch();

  // Returns current protection state.
  // Arc/heating use hold timers to avoid rapid flicker.
  FaultState update(float tempC, float irmsA, int arcModelOut);

  int arcCounter() const { return _arcCnt; }

private:
  int _arcCnt = 0;
  int _heatFrames = 0;

  uint32_t _arcHoldUntil  = 0;
  uint32_t _heatHoldUntil = 0;
};
