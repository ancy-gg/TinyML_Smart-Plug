#include "ProtectionManager.h"

static inline int clampi(int x, int lo, int hi) { return (x < lo) ? lo : (x > hi) ? hi : x; }
static inline float heatTripTempC() {
#if PROTECTION
  return TEMP_TRIP_C;
#else
  return TEMP_DATA_WARN_C;
#endif
}

namespace {

enum ResolvedFaultKind : uint8_t {
  RESOLVED_NONE = 0,
  RESOLVED_OVERLOAD_WARN,
  RESOLVED_UNDERVOLT_WARN,
  RESOLVED_OVERVOLT_WARN,
  RESOLVED_UNDERVOLT_TRIP,
  RESOLVED_OVERVOLT_TRIP,
  RESOLVED_SUSTAINED_OVERLOAD_TRIP,
  RESOLVED_HEAT_WARN,
  RESOLVED_ARC_TRIP,
  RESOLVED_HEAT_TRIP,
};

enum ResolvedFaultFamily : uint8_t {
  RESOLVED_FAMILY_NONE = 0,
  RESOLVED_FAMILY_OVERLOAD = 1,
  RESOLVED_FAMILY_UNDERVOLT = 2,
  RESOLVED_FAMILY_OVERVOLT = 3,
  RESOLVED_FAMILY_HEATING = 4,
  RESOLVED_FAMILY_ARC = 5,
};

struct ResolvedFault {
  ResolvedFaultKind kind = RESOLVED_NONE;
  FaultState state = STATE_NORMAL;
  uint8_t family = RESOLVED_FAMILY_NONE;
  uint32_t onsetMs = 0;
  bool tripActive = false;
  bool alarmActive = false;
};

static inline uint8_t resolvedFamilyForState_(FaultState st) {
  switch (st) {
    case STATE_OVERLOAD:
    case STATE_SUSTAINED_OVERLOAD: return RESOLVED_FAMILY_OVERLOAD;
    case STATE_UNDERVOLTAGE: return RESOLVED_FAMILY_UNDERVOLT;
    case STATE_OVERVOLTAGE: return RESOLVED_FAMILY_OVERVOLT;
    case STATE_HEATING: return RESOLVED_FAMILY_HEATING;
    case STATE_ARCING: return RESOLVED_FAMILY_ARC;
    default: return RESOLVED_FAMILY_NONE;
  }
}

static inline FaultState resolvedKindState_(ResolvedFaultKind kind) {
  switch (kind) {
    case RESOLVED_OVERLOAD_WARN: return STATE_OVERLOAD;
    case RESOLVED_UNDERVOLT_WARN:
    case RESOLVED_UNDERVOLT_TRIP: return STATE_UNDERVOLTAGE;
    case RESOLVED_OVERVOLT_WARN:
    case RESOLVED_OVERVOLT_TRIP: return STATE_OVERVOLTAGE;
    case RESOLVED_SUSTAINED_OVERLOAD_TRIP: return STATE_SUSTAINED_OVERLOAD;
    case RESOLVED_HEAT_WARN:
    case RESOLVED_HEAT_TRIP: return STATE_HEATING;
    case RESOLVED_ARC_TRIP: return STATE_ARCING;
    default: return STATE_NORMAL;
  }
}

static inline uint8_t resolvedKindFamily_(ResolvedFaultKind kind) {
  return resolvedFamilyForState_(resolvedKindState_(kind));
}

static inline bool resolvedKindTrip_(ResolvedFaultKind kind) {
  switch (kind) {
    case RESOLVED_UNDERVOLT_TRIP:
    case RESOLVED_OVERVOLT_TRIP:
    case RESOLVED_SUSTAINED_OVERLOAD_TRIP:
    case RESOLVED_ARC_TRIP:
    case RESOLVED_HEAT_TRIP:
      return true;
    default:
      return false;
  }
}

static inline uint8_t tripPriorityForState_(FaultState st) {
  switch (st) {
    case STATE_HEATING: return 90;
    case STATE_ARCING: return 80;
    case STATE_SUSTAINED_OVERLOAD: return 60;
    case STATE_UNDERVOLTAGE:
    case STATE_OVERVOLTAGE:
      return 50;
    default:
      return 0;
  }
}

static inline uint32_t elapsedSince_(uint32_t startMs, uint32_t endMs) {
  if (startMs == 0 || endMs < startMs) return 0;
  return endMs - startMs;
}

}  // namespace

void ProtectionManager::writeLatchOn_(bool asserted) { if (_pinLatchOn >= 0) digitalWrite(_pinLatchOn, asserted ? HIGH : LOW); }
void ProtectionManager::writeLatchOff_(bool asserted){ if (_pinLatchOff >= 0) digitalWrite(_pinLatchOff, asserted ? HIGH : LOW); }

void ProtectionManager::updateRelayPulse_() {
  const uint32_t now = millis();
  if (_pulseOnUntil && (int32_t)(now - _pulseOnUntil) >= 0) { writeLatchOn_(false); _pulseOnUntil = 0; }
  if (_pulseOffUntil && (int32_t)(now - _pulseOffUntil) >= 0) {
    _pulseOffUntil = 0;
    writeLatchOff_(_relayOffHoldActive);
  }
}

void ProtectionManager::begin(int pinLatchOn, int pinLatchOff) {
  _pinLatchOn  = pinLatchOn;
  _pinLatchOff = pinLatchOff;

  pinMode(_pinLatchOn, OUTPUT);
  pinMode(_pinLatchOff, OUTPUT);

  writeLatchOn_(false);
  writeLatchOff_(false);

  _relayLatchedOn = false;
  _relayOffHoldActive = false;
  _pulseOnUntil = 0;
  _pulseOffUntil = 0;
  _lastRelayPulseMs = 0;

  resetLatch();

  delay(20);
  _lastRelayPulseMs = millis() - LATCH_PULSE_GAP_MS;
  pulseRelayOff(LATCH_OFF_PULSE_MS);

  _tripOffEdge = false;
  _autoOnEdge = false;
}

void ProtectionManager::pulseRelayOn(uint32_t pulseMs) {
  if (pulseMs == 0) pulseMs = LATCH_ON_PULSE_MS;
  const uint32_t now = millis();
  if ((now - _lastRelayPulseMs) < LATCH_PULSE_GAP_MS) return;
  updateRelayPulse_();
  _relayOffHoldActive = false;
  writeLatchOff_(false);
  writeLatchOn_(true);
  _pulseOffUntil = 0;
  _pulseOnUntil = now + pulseMs;
  _lastRelayPulseMs = now;
  _relayLatchedOn = true;
}

void ProtectionManager::pulseRelayOff(uint32_t pulseMs) {
  if (pulseMs == 0) pulseMs = LATCH_OFF_PULSE_MS;
  const uint32_t now = millis();
  if ((now - _lastRelayPulseMs) < LATCH_PULSE_GAP_MS) return;
  updateRelayPulse_();
  writeLatchOn_(false);
  writeLatchOff_(true);
  _pulseOnUntil = 0;
  _pulseOffUntil = now + pulseMs;
  _lastRelayPulseMs = now;
  _relayLatchedOn = false;
}

void ProtectionManager::noteActuation(ProtectionActuationKind kind, uint32_t now) {
  if (kind == PROTECTION_ACTUATION_NONE || _telemetryFamily == RESOLVED_FAMILY_NONE) return;
  if (now == 0) now = millis();
  if (kind == PROTECTION_ACTUATION_RELAY_TRIP) {
    _tripActuationPending = true;
    return;
  }
  if (_telemetryActuationKind == PROTECTION_ACTUATION_NONE) {
    _telemetryActuationKind = kind;
    _telemetryActuatedMs = now;
  }
}

void ProtectionManager::updateActuationFeedback(float irmsMeasured, bool currentValidMeasured, uint32_t now) {
  if (!_tripActuationPending || _telemetryFamily == RESOLVED_FAMILY_NONE) return;
  if (_telemetryActuationKind == PROTECTION_ACTUATION_RELAY_TRIP) {
    _tripActuationPending = false;
    return;
  }
  if (now == 0) now = millis();
  const bool currentGone = (!currentValidMeasured) || (irmsMeasured <= LOAD_OFF_DETECT_A);
  if ((!_relayLatchedOn || relayPulseActive()) && currentGone) {
    _telemetryActuationKind = PROTECTION_ACTUATION_RELAY_TRIP;
    _telemetryActuatedMs = now;
    _tripActuationPending = false;
  }
}

void ProtectionManager::setRelayOffHold(bool asserted) {
  _relayOffHoldActive = asserted;
  updateRelayPulse_();
  if (_pulseOnUntil != 0) return;
  writeLatchOff_(asserted || (_pulseOffUntil != 0));
}

void ProtectionManager::maybeLatchTrip_(FaultState st, uint32_t onsetMs, uint8_t priority) {
  if (priority < _latchedFaultPriority) return;
  _latchedFaultState = st;
  _latchedFaultPriority = priority;
  _latchedFaultOnsetMs = onsetMs;
}

void ProtectionManager::resetLatch() {
  _arcCnt = 0;
  _heatFrames = 0;
  _arcHoldUntil = 0;
  _arcSequenceOnsetMs = 0;
  _heatWarnSince = 0;
  _heatTripSince = 0;
  _heatWarnHoldUntil = 0;
  _heatHoldUntil = 0;
  _underVoltSince = 0;
  _overVoltSince = 0;
  _overloadSince = 0;
  _overloadTransientSuppressUntil = 0;
  _sustainedOverloadStateSince = 0;
  _sustainedOverloadLastAvgMs = 0;
  _sustainedOverloadAvgA = 0.0f;
  _sustainedOverloadScore = 0;
  _sustainedOverloadHigh = false;
  _sustainedOverloadAvgInit = false;
  _voltageLockout = false;
  _voltageLockoutKind = STATE_NORMAL;
  _voltageLockoutSince = 0;
  _voltageRecoverySince = 0;
  _tripOffEdge = false;
  _autoOnEdge = false;
  _webLockout = false;
  _resetRequired = false;
  _latchedFaultState = STATE_NORMAL;
  _latchedFaultOnsetMs = 0;
  _latchedFaultPriority = 0;
  _alarmActive = false;
  _loadOn = false;
  _loadOnSince = 0;
  _loadOffSince = 0;
  _prevSustainedTrip = false;
  _arcTripPending = false;
  _arcPendingSawCurrentReturn = false;
  _arcTripPendingSince = 0;
  _arcActiveSince = 0;
  _prevArcActive = false;
  _prevHeatTrip = false;
  _prevOverloadTrip = false;
  _prevIrmsA = 0.0f;
  _telemetryState = STATE_NORMAL;
  _telemetryFamily = RESOLVED_FAMILY_NONE;
  _telemetryTripActive = false;
  _telemetryOnsetMs = 0;
  _telemetryDetectedMs = 0;
  _telemetryActuatedMs = 0;
  _telemetryActuationKind = PROTECTION_ACTUATION_NONE;
  _tripActuationPending = false;
  setRelayOffHold(false);
}

bool ProtectionManager::consumeTripOffEdge() { const bool v = _tripOffEdge; _tripOffEdge = false; return v; }
bool ProtectionManager::consumeAutoOnEdge()  { const bool v = _autoOnEdge; _autoOnEdge = false; return v; }

uint32_t ProtectionManager::stateOnsetUptimeMs(FaultState st) const {
  switch (st) {
    case STATE_OVERLOAD:
      return _overloadSince ? _overloadSince : _latchedFaultOnsetMs;
    case STATE_SUSTAINED_OVERLOAD:
      if (_overloadSince) return _overloadSince;
      if (_sustainedOverloadHigh && _sustainedOverloadStateSince) return _sustainedOverloadStateSince;
      return _latchedFaultOnsetMs;
    case STATE_UNDERVOLTAGE:
      if (_voltageLockout && _voltageLockoutKind == STATE_UNDERVOLTAGE && _voltageLockoutSince) return _voltageLockoutSince;
      return _underVoltSince ? _underVoltSince : _latchedFaultOnsetMs;
    case STATE_OVERVOLTAGE:
      if (_voltageLockout && _voltageLockoutKind == STATE_OVERVOLTAGE && _voltageLockoutSince) return _voltageLockoutSince;
      return _overVoltSince ? _overVoltSince : _latchedFaultOnsetMs;
    case STATE_HEATING:
      if (_heatTripSince) return _heatTripSince;
      return _heatWarnSince ? _heatWarnSince : _latchedFaultOnsetMs;
    case STATE_ARCING:
      if (_arcSequenceOnsetMs) return _arcSequenceOnsetMs;
      return _arcActiveSince ? _arcActiveSince : _latchedFaultOnsetMs;
    case STATE_NORMAL:
    default:
      return faultLatched() ? _latchedFaultOnsetMs : 0;
  }
}

FaultState ProtectionManager::update(float vProtect, float vRaw, float tempC, float irmsA, int arcModelOut, bool arcEligible) {
  const uint32_t now = millis();
  const float prevIrmsA = _prevIrmsA;
  const int prevArcCnt = _arcCnt;

  if (!arcEligible) arcModelOut = 0;
  if (arcModelOut == 1 && prevArcCnt <= 0 && _arcSequenceOnsetMs == 0) {
    _arcSequenceOnsetMs = now;
  }
  if (arcModelOut == 1) _arcCnt += ARC_CNT_INC; else _arcCnt -= ARC_CNT_DEC;
  _arcCnt = clampi(_arcCnt, 0, ARC_CNT_MAX);

  _tripOffEdge = false;
  _autoOnEdge = false;

#if !PROTECTION
  _webLockout = false;
  _resetRequired = false;
  _voltageLockout = false;
  _voltageLockoutKind = STATE_NORMAL;
  _voltageLockoutSince = 0;
  _voltageRecoverySince = 0;
  _latchedFaultState = STATE_NORMAL;
  _latchedFaultOnsetMs = 0;
  _latchedFaultPriority = 0;
  _alarmActive = false;
  _telemetryState = STATE_NORMAL;
  _telemetryFamily = RESOLVED_FAMILY_NONE;
  _telemetryTripActive = false;
  _telemetryOnsetMs = 0;
  _telemetryDetectedMs = 0;
  _telemetryActuatedMs = 0;
  _telemetryActuationKind = PROTECTION_ACTUATION_NONE;
  _tripActuationPending = false;

  if (!_loadOn) {
    if (irmsA >= LOAD_ON_DETECT_A) {
      if (_loadOnSince == 0) _loadOnSince = now;
      if ((now - _loadOnSince) >= LOAD_ON_DETECT_MS) { _loadOn = true; _loadOffSince = 0; }
    } else {
      _loadOnSince = 0;
    }
  } else {
    if (irmsA <= LOAD_OFF_DETECT_A) {
      if (_loadOffSince == 0) _loadOffSince = now;
      if ((now - _loadOffSince) >= LOAD_OFF_DETECT_MS) { _loadOn = false; _loadOnSince = 0; }
    } else {
      _loadOffSince = 0;
    }
  }

  _prevIrmsA = irmsA;
  return STATE_NORMAL;
#endif

  const bool arcTrip = (_arcCnt >= ARC_CNT_TRIP);
  const bool currentGoneForArcDecision = (irmsA <= LOAD_OFF_DETECT_A);

  if (arcTrip) {
    if (!_arcTripPending && currentGoneForArcDecision) {
      _arcTripPending = true;
      _arcPendingSawCurrentReturn = false;
      _arcTripPendingSince = now;
    } else if (!_arcTripPending) {
      _arcHoldUntil = now + ARC_HOLD_MS;
    }
  }

  if (_arcTripPending) {
    if (!currentGoneForArcDecision) _arcPendingSawCurrentReturn = true;

    if ((now - _arcTripPendingSince) >= ARC_CURRENT_RETURN_VERIFY_MS) {
      if (_arcPendingSawCurrentReturn) {
        _arcHoldUntil = now + ARC_HOLD_MS;
      } else {
        _arcCnt = 0;
        _arcHoldUntil = 0;
      }
      _arcTripPending = false;
      _arcPendingSawCurrentReturn = false;
      _arcTripPendingSince = 0;
    }
  } else if (!arcTrip) {
    _arcPendingSawCurrentReturn = false;
    _arcTripPendingSince = 0;
  }

  const bool arcTripNow = (_arcCnt >= ARC_CNT_TRIP);
  const bool arcActive = (!_arcTripPending) && (arcTripNow || (now < _arcHoldUntil));
  if (arcActive && !_prevArcActive) {
    _arcActiveSince = (_arcSequenceOnsetMs != 0) ? _arcSequenceOnsetMs : now;
    _tripOffEdge = true;
    _resetRequired = true;
    maybeLatchTrip_(STATE_ARCING, _arcActiveSince, tripPriorityForState_(STATE_ARCING));
  } else if (!arcActive) {
    _arcActiveSince = 0;
    if (!_arcTripPending && _arcCnt == 0) _arcSequenceOnsetMs = 0;
  }
  _prevArcActive = arcActive;

  const bool heatWarnCandidate = isfinite(tempC) && (tempC >= TEMP_WARN_C);
  const bool heatWarnClear = !isfinite(tempC) || (tempC <= TEMP_WARN_CLEAR_C);
  const bool heatTripRaw = isfinite(tempC) && (tempC >= heatTripTempC());
  if (heatWarnCandidate) {
    if (_heatWarnSince == 0) _heatWarnSince = now;
    _heatWarnHoldUntil = now + HEAT_WARN_HOLD_MS;
  } else if (heatWarnClear && (int32_t)(now - _heatWarnHoldUntil) >= 0) {
    _heatWarnSince = 0;
  }
  if (heatTripRaw) {
    if (_heatTripSince == 0) _heatTripSince = now;
    _heatHoldUntil = now + HEAT_HOLD_MS;
  }
  const bool heatTripActive = heatTripRaw || ((int32_t)(_heatHoldUntil - now) > 0);
  if (!heatTripActive) _heatTripSince = 0;
  const bool heatWarnActive =
      (_heatWarnSince != 0) &&
      (heatWarnCandidate || !heatWarnClear || ((int32_t)(_heatWarnHoldUntil - now) > 0) || heatTripActive);
  const uint32_t heatOnsetMs = (_heatWarnSince != 0) ? _heatWarnSince : now;
  const uint32_t heatTripOnsetMs = (_heatTripSince != 0) ? _heatTripSince : heatOnsetMs;
  if (heatTripActive && !_prevHeatTrip) {
    _tripOffEdge = true;
    _resetRequired = true;
    maybeLatchTrip_(STATE_HEATING, heatTripOnsetMs, tripPriorityForState_(STATE_HEATING));
  }
  _prevHeatTrip = heatTripActive;

  const bool mainsGoneLike = (vRaw <= MAINS_PRESENT_OFF_V) || ((vProtect <= MAINS_PRESENT_OFF_V) && (irmsA <= LOAD_OFF_DETECT_A));
  const bool rawHealthyWindow = (vRaw >= VOLT_RECOVER_MIN_V) && (vRaw <= VOLT_RECOVER_MAX_V);
  const bool rawUnderRange = (vRaw >= VOLT_UV_CANDIDATE_RAW_MIN_V) && (vRaw < VOLT_NORMAL_MIN_V);
  const bool rawOverRange = (vRaw >= VOLT_OV_DELAY_V);
  const float vVote = 0.70f * vProtect + 0.30f * vRaw;

  bool underVoltValid = false;
  bool overVoltValid = false;

  if (mainsGoneLike || rawHealthyWindow) {
    _underVoltSince = 0;
    _overVoltSince = 0;
  }

  const bool underVoltCandidate = !mainsGoneLike && rawUnderRange && (vVote < VOLT_NORMAL_MIN_V);
  const bool overVoltCandidate = !mainsGoneLike && rawOverRange && (vVote > VOLT_NORMAL_MAX_V);

  if (underVoltCandidate && !arcActive) {
    if (_underVoltSince == 0) _underVoltSince = now;
    const uint32_t need = (vVote <= VOLT_UV_INSTANT_V) ? VOLT_UV_INSTANT_MS : VOLT_UV_DELAY_MS;
    underVoltValid = (need == 0UL) || ((now - _underVoltSince) >= need);
  } else if (!_voltageLockout || _voltageLockoutKind != STATE_UNDERVOLTAGE) {
    _underVoltSince = 0;
  }

  if (overVoltCandidate) {
    if (_overVoltSince == 0) _overVoltSince = now;
    const uint32_t need = (vVote >= VOLT_OV_INSTANT_V) ? VOLT_OV_INSTANT_MS : VOLT_OV_DELAY_MS;
    overVoltValid = (need == 0UL) || ((now - _overVoltSince) >= need);
  } else if (!_voltageLockout || _voltageLockoutKind != STATE_OVERVOLTAGE) {
    _overVoltSince = 0;
  }

  const bool overloadRaw = isfinite(irmsA) && (irmsA >= OVERLOAD_WARN_A);
  const bool overloadBelowClear = !isfinite(irmsA) || (irmsA <= OVERLOAD_WARN_CLEAR_A);
  if (overloadRaw) {
    if (_overloadSince == 0) _overloadSince = now;
    const bool crossingWarn = (prevIrmsA < OVERLOAD_WARN_A) &&
                              ((irmsA - prevIrmsA) >= OVERLOAD_WARN_TRANSIENT_STEP_A);
    if (crossingWarn) {
      const uint32_t suppressUntil = now + OVERLOAD_WARN_TRANSIENT_SUPPRESS_MS;
      if ((int32_t)(_overloadTransientSuppressUntil - suppressUntil) < 0) {
        _overloadTransientSuppressUntil = suppressUntil;
      }
    }
  } else if (overloadBelowClear) {
    _overloadSince = 0;
    _overloadTransientSuppressUntil = 0;
  }
  const bool overloadWarnActive =
      (_overloadSince != 0) &&
      !overloadBelowClear &&
      ((now - _overloadSince) >= OVERLOAD_WARN_DELAY_MS) &&
      ((int32_t)(now - _overloadTransientSuppressUntil) >= 0);

  if (!_sustainedOverloadAvgInit) {
    _sustainedOverloadAvgA = irmsA;
    _sustainedOverloadLastAvgMs = now;
    _sustainedOverloadAvgInit = true;
  } else {
    uint32_t dtMs = now - _sustainedOverloadLastAvgMs;
    if (dtMs > 250UL) dtMs = 250UL;
    if (dtMs > 0UL) {
      const float alpha = (float)dtMs / (float)SUSTAINED_OVERLOAD_AVG_WINDOW_MS;
      const float a = (alpha > 1.0f) ? 1.0f : ((alpha < 0.0f) ? 0.0f : alpha);
      _sustainedOverloadAvgA += (irmsA - _sustainedOverloadAvgA) * a;
      _sustainedOverloadLastAvgMs = now;
    }
  }

  bool sustainedOverloadHighNow = _sustainedOverloadHigh;
  if (!_sustainedOverloadHigh) {
    if (_sustainedOverloadAvgA >= SUSTAINED_OVERLOAD_TRIP_A) sustainedOverloadHighNow = true;
  } else {
    if (_sustainedOverloadAvgA <= SUSTAINED_OVERLOAD_CLEAR_A) sustainedOverloadHighNow = false;
  }

  if ((_sustainedOverloadStateSince == 0) || (sustainedOverloadHighNow != _sustainedOverloadHigh)) {
    _sustainedOverloadStateSince = now;
    _sustainedOverloadHigh = sustainedOverloadHighNow;
  } else {
    const uint32_t elapsed = now - _sustainedOverloadStateSince;
    if (elapsed >= SUSTAINED_OVERLOAD_SCORE_STEP_MS) {
      const uint32_t steps = elapsed / SUSTAINED_OVERLOAD_SCORE_STEP_MS;
      if (_sustainedOverloadHigh) {
        _sustainedOverloadScore = clampi(_sustainedOverloadScore + (int)steps, 0, SUSTAINED_OVERLOAD_SCORE_MAX);
      } else {
        _sustainedOverloadScore = clampi(_sustainedOverloadScore - (int)steps, 0, SUSTAINED_OVERLOAD_SCORE_MAX);
      }
      _sustainedOverloadStateSince += steps * SUSTAINED_OVERLOAD_SCORE_STEP_MS;
    }
  }

  const bool sustainedOverloadActive = (_sustainedOverloadScore >= SUSTAINED_OVERLOAD_SCORE_TRIP);
  const uint32_t overloadTripOnsetMs =
      (_overloadSince != 0) ? _overloadSince :
      ((_sustainedOverloadHigh && _sustainedOverloadStateSince != 0) ? _sustainedOverloadStateSince : now);
  if (sustainedOverloadActive && !_prevOverloadTrip) {
    _tripOffEdge = true;
    _resetRequired = true;
    maybeLatchTrip_(STATE_SUSTAINED_OVERLOAD, overloadTripOnsetMs, tripPriorityForState_(STATE_SUSTAINED_OVERLOAD));
  }
  _prevOverloadTrip = sustainedOverloadActive;
  _prevSustainedTrip = sustainedOverloadActive;

  if (mainsGoneLike) {
    _voltageRecoverySince = 0;
    _voltageLockout = false;
    _voltageLockoutKind = STATE_NORMAL;
    _voltageLockoutSince = 0;
    if (!_resetRequired) {
      _latchedFaultState = STATE_NORMAL;
      _latchedFaultOnsetMs = 0;
      _latchedFaultPriority = 0;
    }
  } else if ((underVoltValid && !arcActive) || overVoltValid) {
    const FaultState kind = overVoltValid ? STATE_OVERVOLTAGE : STATE_UNDERVOLTAGE;
    const uint32_t onsetMs = (kind == STATE_OVERVOLTAGE)
        ? ((_overVoltSince != 0) ? _overVoltSince : now)
        : ((_underVoltSince != 0) ? _underVoltSince : now);
    if (!_voltageLockout || _voltageLockoutKind != kind) {
      _tripOffEdge = true;
      _voltageLockoutSince = onsetMs;
    } else if (_voltageLockoutSince == 0) {
      _voltageLockoutSince = onsetMs;
    }
    _voltageLockout = true;
    _voltageLockoutKind = kind;
    _voltageRecoverySince = 0;
    maybeLatchTrip_(kind, _voltageLockoutSince, tripPriorityForState_(kind));
  } else if (_voltageLockout) {
    if (rawHealthyWindow && !underVoltCandidate && !overVoltCandidate) {
      if (_voltageRecoverySince == 0) _voltageRecoverySince = now;
      if ((now - _voltageRecoverySince) >= VOLTAGE_RECLOSE_STABLE_MS) {
        _voltageLockout = false;
        _voltageLockoutKind = STATE_NORMAL;
        _voltageLockoutSince = 0;
        _voltageRecoverySince = 0;
        _autoOnEdge = true;
        if (!_resetRequired) {
          _latchedFaultState = STATE_NORMAL;
          _latchedFaultOnsetMs = 0;
          _latchedFaultPriority = 0;
        }
      }
    } else {
      _voltageRecoverySince = 0;
    }
  }

  if (!_loadOn) {
    if (irmsA >= LOAD_ON_DETECT_A) {
      if (_loadOnSince == 0) _loadOnSince = now;
      if ((now - _loadOnSince) >= LOAD_ON_DETECT_MS) { _loadOn = true; _loadOffSince = 0; }
    } else {
      _loadOnSince = 0;
    }
  } else {
    if (irmsA <= LOAD_OFF_DETECT_A) {
      if (_loadOffSince == 0) _loadOffSince = now;
      if ((now - _loadOffSince) >= LOAD_OFF_DETECT_MS) { _loadOn = false; _loadOnSince = 0; }
    } else {
      _loadOffSince = 0;
    }
  }

  const bool underVoltTripActive = _voltageLockout && (_voltageLockoutKind == STATE_UNDERVOLTAGE);
  const bool overVoltTripActive = _voltageLockout && (_voltageLockoutKind == STATE_OVERVOLTAGE);
  const bool underVoltWarnActive = !_voltageLockout && underVoltCandidate && !underVoltValid && !arcActive;
  const bool overVoltWarnActive = !_voltageLockout && overVoltCandidate && !overVoltValid;

  ResolvedFault resolved;
  if (heatTripActive) {
    resolved.kind = RESOLVED_HEAT_TRIP;
    resolved.onsetMs = heatTripOnsetMs;
  } else if (arcActive) {
    resolved.kind = RESOLVED_ARC_TRIP;
    resolved.onsetMs = (_arcSequenceOnsetMs != 0) ? _arcSequenceOnsetMs :
                       ((_arcActiveSince != 0) ? _arcActiveSince : now);
  } else if (heatWarnActive) {
    resolved.kind = RESOLVED_HEAT_WARN;
    resolved.onsetMs = heatOnsetMs;
  } else if (sustainedOverloadActive) {
    resolved.kind = RESOLVED_SUSTAINED_OVERLOAD_TRIP;
    resolved.onsetMs = overloadTripOnsetMs;
  } else if (overVoltTripActive) {
    resolved.kind = RESOLVED_OVERVOLT_TRIP;
    resolved.onsetMs = (_voltageLockoutSince != 0) ? _voltageLockoutSince : now;
  } else if (underVoltTripActive) {
    resolved.kind = RESOLVED_UNDERVOLT_TRIP;
    resolved.onsetMs = (_voltageLockoutSince != 0) ? _voltageLockoutSince : now;
  } else if (overVoltWarnActive) {
    resolved.kind = RESOLVED_OVERVOLT_WARN;
    resolved.onsetMs = (_overVoltSince != 0) ? _overVoltSince : now;
  } else if (underVoltWarnActive) {
    resolved.kind = RESOLVED_UNDERVOLT_WARN;
    resolved.onsetMs = (_underVoltSince != 0) ? _underVoltSince : now;
  } else if (overloadWarnActive) {
    resolved.kind = RESOLVED_OVERLOAD_WARN;
    resolved.onsetMs = (_overloadSince != 0) ? _overloadSince : now;
  }

  resolved.state = resolvedKindState_(resolved.kind);
  resolved.family = resolvedKindFamily_(resolved.kind);
  resolved.tripActive = resolvedKindTrip_(resolved.kind);
  resolved.alarmActive = (resolved.kind != RESOLVED_NONE);

  if (!_resetRequired && !_voltageLockout) {
    _latchedFaultState = STATE_NORMAL;
    _latchedFaultOnsetMs = 0;
    _latchedFaultPriority = 0;
  }

  FaultState effectiveTelemetryState = resolved.state;
  uint8_t effectiveTelemetryFamily = resolved.family;
  const bool effectiveTripTelemetry = resolved.tripActive || faultLatched();
  uint32_t effectiveTelemetryOnsetMs = resolved.onsetMs;
  if (effectiveTelemetryState == STATE_NORMAL && faultLatched()) {
    effectiveTelemetryState = _latchedFaultState;
    effectiveTelemetryFamily = resolvedFamilyForState_(effectiveTelemetryState);
    effectiveTelemetryOnsetMs = _latchedFaultOnsetMs;
  }

  _alarmActive = (effectiveTelemetryState != STATE_NORMAL);
  if (effectiveTelemetryFamily == RESOLVED_FAMILY_NONE) {
    _telemetryState = STATE_NORMAL;
    _telemetryFamily = RESOLVED_FAMILY_NONE;
    _telemetryTripActive = false;
    _telemetryOnsetMs = 0;
    _telemetryDetectedMs = 0;
    _telemetryActuatedMs = 0;
    _telemetryActuationKind = PROTECTION_ACTUATION_NONE;
    _tripActuationPending = false;
  } else if (_telemetryFamily != effectiveTelemetryFamily || _telemetryTripActive != effectiveTripTelemetry) {
    _telemetryState = effectiveTelemetryState;
    _telemetryFamily = effectiveTelemetryFamily;
    _telemetryTripActive = effectiveTripTelemetry;
    _telemetryOnsetMs = effectiveTelemetryOnsetMs ? effectiveTelemetryOnsetMs : now;
    _telemetryDetectedMs = now;
    _telemetryActuatedMs = 0;
    _telemetryActuationKind = PROTECTION_ACTUATION_NONE;
    _tripActuationPending = false;
    if (_alarmActive && !effectiveTripTelemetry) noteActuation(PROTECTION_ACTUATION_ALARM, now);
  } else {
    _telemetryState = effectiveTelemetryState;
    if (_telemetryOnsetMs == 0 || (effectiveTelemetryOnsetMs != 0 && effectiveTelemetryOnsetMs < _telemetryOnsetMs)) {
      _telemetryOnsetMs = effectiveTelemetryOnsetMs;
    }
    if (_telemetryDetectedMs == 0) _telemetryDetectedMs = now;
    if (_alarmActive && !effectiveTripTelemetry && _telemetryActuationKind == PROTECTION_ACTUATION_NONE) {
      noteActuation(PROTECTION_ACTUATION_ALARM, now);
    }
  }

  _webLockout = _resetRequired || arcActive || heatTripActive || sustainedOverloadActive || _voltageLockout;
  _prevIrmsA = irmsA;
  return resolved.state;
}

void ProtectionManager::apply(FaultState st, float vDisplay, float vProtect, float i, float t) {
  (void)st;
  (void)vDisplay;
  (void)vProtect;
  (void)i;
  (void)t;
  updateRelayPulse_();
}
