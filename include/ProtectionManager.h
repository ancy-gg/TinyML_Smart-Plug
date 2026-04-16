#pragma once
#include <Arduino.h>
#include "MainConfiguration.h"

class ProtectionManager {
public:
  void begin(int pinLatchOn, int pinLatchOff);

  FaultState update(float vProtect, float vRaw, float tempC, float irmsA, int arcModelOut, bool arcEligible);
  void resetLatch();
  int arcCounter() const { return _arcCnt; }
  bool consumeTripOffEdge();
  bool consumeAutoOnEdge();
  bool webControlLocked() const { return _webLockout; }
  bool voltageLockoutActive() const { return _voltageLockout; }
  bool faultLatched() const { return _resetRequired || _voltageLockout; }
  FaultState latchedFaultState() const { return _latchedFaultState; }
  const char* loadState() const { return _loadOn ? "LOAD ON" : "LOAD OFF"; }

  void apply(FaultState st, float vDisplay, float vProtect, float i, float t);
  void pulseRelayOn(uint32_t pulseMs = 0);
  void pulseRelayOff(uint32_t pulseMs = 0);
  void setRelayOffHold(bool asserted);
  bool relayLatchedOn() const { return _relayLatchedOn; }
  bool relayOffHoldActive() const { return _relayOffHoldActive; }
  bool relayPulseActive() const {
    const uint32_t now = millis();
    return (_pulseOnUntil && (int32_t)(now - _pulseOnUntil) < 0) ||
           (_pulseOffUntil && (int32_t)(now - _pulseOffUntil) < 0);
  }

private:
  int _pinLatchOn = -1;
  int _pinLatchOff = -1;

  bool _relayLatchedOn = false;
  bool _relayOffHoldActive = false;
  uint32_t _pulseOnUntil = 0;
  uint32_t _pulseOffUntil = 0;
  uint32_t _lastRelayPulseMs = 0;

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
  bool _resetRequired = false;
  FaultState _latchedFaultState = STATE_NORMAL;
  bool _loadOn = false;
  uint32_t _loadOnSince = 0;
  uint32_t _loadOffSince = 0;
  bool _prevSustainedTrip = false;
  bool _arcTripPending = false;
  bool _arcPendingSawCurrentReturn = false;
  uint32_t _arcTripPendingSince = 0;
  bool _prevArcActive = false;
  bool _prevHeatTrip = false;
  bool _prevOverloadTrip = false;

  void writeLatchOn_(bool asserted);
  void writeLatchOff_(bool asserted);
  void updateRelayPulse_();
};
