#pragma once
#include <Arduino.h>
#include "SmartPlugTypes.h"
#include "SmartPlugConfig.h"

class FaultLogic {
public:
  void resetLatch();

  // Returns current protection state.
  // Arc and heating use hold timers to avoid rapid flicker.
  // Voltage and overload are evaluated every call and prioritized in the state order.
  FaultState update(float vProtect, float tempC, float irmsA, int arcModelOut, bool arcEligible);

  int arcCounter() const { return _arcCnt; }

private:
  int _arcCnt = 0;
  int _heatFrames = 0;

  uint32_t _arcHoldUntil  = 0;
  uint32_t _heatHoldUntil = 0;
};
