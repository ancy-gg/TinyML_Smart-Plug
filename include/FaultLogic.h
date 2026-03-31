#pragma once
#include <Arduino.h>
#include "SmartPlugTypes.h"
#include "SmartPlugConfig.h"

class FaultLogic {
public:
  void resetLatch();
  FaultState update(float vProtect, float vRaw, float tempC, float irmsA, int arcModelOut, bool arcEligible);

  int arcCounter() const { return _arcCnt; }
  bool consumeTripOffEdge();
  bool consumeAutoOnEdge();
  bool webControlLocked() const { return _webLockout; }
  bool voltageLockoutActive() const { return _voltageLockout; }
  const char* loadState() const { return _loadOn ? "LOAD ON" : "LOAD OFF"; }

private:
  int _arcCnt = 0;
  int _heatFrames = 0;

  uint32_t _arcHoldUntil  = 0;
  uint32_t _heatHoldUntil = 0;

  uint32_t _underVoltSince = 0;
  uint32_t _overVoltSince  = 0;
  uint32_t _overloadSince  = 0;
  uint32_t _sustainedOverloadSince = 0;

  bool _voltageLockout = false;
  FaultState _voltageLockoutKind = STATE_NORMAL;
  uint32_t _voltageRecoverySince = 0;

  bool _tripOffEdge = false;
  bool _autoOnEdge = false;
  bool _webLockout = false;

  bool _loadOn = false;
  uint32_t _loadOnSince = 0;
  uint32_t _loadOffSince = 0;
  bool _prevSustainedTrip = false;
};
