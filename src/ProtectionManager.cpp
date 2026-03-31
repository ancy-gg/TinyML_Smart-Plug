#include "ProtectionManager.h"

static inline int clampi(int x, int lo, int hi) { return (x < lo) ? lo : (x > hi) ? hi : x; }
static inline float heatTripTempC() {
#if COLLECTION_ONLY_MODE
  return TEMP_DATA_WARN_C;
#else
  return TEMP_TRIP_C;
#endif
}

void ProtectionManager::writeLatchOn_(bool asserted) { if (_pinLatchOn >= 0) digitalWrite(_pinLatchOn, asserted ? HIGH : LOW); }
void ProtectionManager::writeLatchOff_(bool asserted){ if (_pinLatchOff >= 0) digitalWrite(_pinLatchOff, asserted ? HIGH : LOW); }
void ProtectionManager::updateRelayPulse_() {
  const uint32_t now = millis();
  if (_pulseOnUntil && (int32_t)(now - _pulseOnUntil) >= 0) { writeLatchOn_(false); _pulseOnUntil = 0; }
  if (_pulseOffUntil && (int32_t)(now - _pulseOffUntil) >= 0) { writeLatchOff_(false); _pulseOffUntil = 0; }
}

void ProtectionManager::begin(int pinLatchOn, int pinLatchOff) {
  _pinLatchOn  = pinLatchOn;
  _pinLatchOff = pinLatchOff;

  pinMode(_pinLatchOn, OUTPUT);
  pinMode(_pinLatchOff, OUTPUT);

  writeLatchOn_(false);
  writeLatchOff_(false);

  _relayLatchedOn   = false;
  _pulseOnUntil     = 0;
  _pulseOffUntil    = 0;
  _loadDetectSince  = 0;
  _lastRelayPulseMs = 0;

  resetLatch();

  delay(20);
  _lastRelayPulseMs = millis() - LATCH_PULSE_GAP_MS;
  pulseRelayOff(LATCH_OFF_PULSE_MS);

  _tripOffEdge = false;
  _autoOnEdge  = false;
}

void ProtectionManager::pulseRelayOn(uint32_t pulseMs) {
  if (pulseMs == 0) pulseMs = LATCH_ON_PULSE_MS;
  const uint32_t now = millis();
  if ((now - _lastRelayPulseMs) < LATCH_PULSE_GAP_MS) return;
  updateRelayPulse_(); writeLatchOff_(false); writeLatchOn_(true);
  _pulseOffUntil = 0; _pulseOnUntil = now + pulseMs; _lastRelayPulseMs = now; _relayLatchedOn = true;
}
void ProtectionManager::pulseRelayOff(uint32_t pulseMs) {
  if (pulseMs == 0) pulseMs = LATCH_OFF_PULSE_MS;
  const uint32_t now = millis();
  if ((now - _lastRelayPulseMs) < LATCH_PULSE_GAP_MS) return;
  updateRelayPulse_(); writeLatchOn_(false); writeLatchOff_(true);
  _pulseOnUntil = 0; _pulseOffUntil = now + pulseMs; _lastRelayPulseMs = now; _relayLatchedOn = false;
}

void ProtectionManager::resetLatch() {
  _arcCnt = 0; _heatFrames = 0; _arcHoldUntil = 0; _heatHoldUntil = 0;
  _underVoltSince = 0; _overVoltSince = 0; _overloadSince = 0; _sustainedOverloadSince = 0;
  _voltageLockout = false; _voltageLockoutKind = STATE_NORMAL; _voltageRecoverySince = 0;
  _tripOffEdge = false; _autoOnEdge = false; _webLockout = false;
  _loadOn = false; _loadOnSince = 0; _loadOffSince = 0; _prevSustainedTrip = false;
}
bool ProtectionManager::consumeTripOffEdge() { const bool v = _tripOffEdge; _tripOffEdge = false; return v; }
bool ProtectionManager::consumeAutoOnEdge()  { const bool v = _autoOnEdge; _autoOnEdge = false; return v; }

FaultState ProtectionManager::update(float vProtect, float vRaw, float tempC, float irmsA, int arcModelOut, bool arcEligible) {
  const uint32_t now = millis();
  if (!arcEligible || irmsA < ARC_MIN_IRMS_A) arcModelOut = 0;

  if (arcModelOut == 1) _arcCnt += ARC_CNT_INC; else _arcCnt -= ARC_CNT_DEC;
  _arcCnt = clampi(_arcCnt, 0, ARC_CNT_MAX);
  const bool arcTrip = (_arcCnt >= ARC_CNT_TRIP);

  if (tempC >= heatTripTempC()) _heatFrames++; else _heatFrames = clampi(_heatFrames - HEAT_FRAMES_DEC, 0, HEAT_FRAMES_TRIP);
  const bool heatTrip = (_heatFrames >= HEAT_FRAMES_TRIP);
  if (arcTrip) _arcHoldUntil = now + ARC_HOLD_MS;
  if (heatTrip) _heatHoldUntil = now + HEAT_HOLD_MS;
  const bool arcActive = arcTrip || (now < _arcHoldUntil);
  const bool heatActive = heatTrip || (now < _heatHoldUntil);

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
  const bool overVoltCandidate  = !mainsGoneLike && rawOverRange  && (vVote > VOLT_NORMAL_MAX_V);

  if (underVoltCandidate && !arcActive) {
    if (_underVoltSince == 0) _underVoltSince = now;
    uint32_t need = VOLT_UV_WARN_MS;
    if (vVote <= VOLT_UV_INSTANT_V) need = VOLT_UV_INSTANT_MS;
    else if (vVote <= VOLT_UV_DELAY_V) need = VOLT_UV_DELAY_MS;
    underVoltValid = (need == 0UL) || ((now - _underVoltSince) >= need);
  } else {
    _underVoltSince = 0;
  }

  if (overVoltCandidate) {
    if (_overVoltSince == 0) _overVoltSince = now;
    const uint32_t need = (vVote >= VOLT_OV_INSTANT_V) ? VOLT_OV_INSTANT_MS : VOLT_OV_DELAY_MS;
    overVoltValid = (need == 0UL) || ((now - _overVoltSince) >= need);
  }

  const bool overloadRaw = (irmsA >= OVERLOAD_WARN_A);
  if (overloadRaw) { if (_overloadSince == 0) _overloadSince = now; } else _overloadSince = 0;
  const bool overloadActive = overloadRaw && ((now - _overloadSince) >= OVERLOAD_TRIP_MS);

  const bool sustainedOverloadRaw = (irmsA >= SUSTAINED_OVERLOAD_TRIP_A);
  if (sustainedOverloadRaw) { if (_sustainedOverloadSince == 0) _sustainedOverloadSince = now; } else _sustainedOverloadSince = 0;
  const bool sustainedOverloadActive = sustainedOverloadRaw && ((now - _sustainedOverloadSince) >= SUSTAINED_OVERLOAD_TRIP_MS);

  if (mainsGoneLike) {
    _voltageRecoverySince = 0; _voltageLockout = false; _voltageLockoutKind = STATE_NORMAL;
  } else if ((underVoltValid && !arcActive) || overVoltValid) {
    const FaultState kind = overVoltValid ? STATE_OVERVOLTAGE : STATE_UNDERVOLTAGE;
    if (!_voltageLockout || _voltageLockoutKind != kind) _tripOffEdge = true;
    _voltageLockout = true; _voltageLockoutKind = kind; _voltageRecoverySince = 0;
  } else if (_voltageLockout) {
    if (rawHealthyWindow && !underVoltCandidate && !overVoltCandidate) {
      if (_voltageRecoverySince == 0) _voltageRecoverySince = now;
      if ((now - _voltageRecoverySince) >= VOLTAGE_RECLOSE_STABLE_MS) {
        _voltageLockout = false; _voltageLockoutKind = STATE_NORMAL; _voltageRecoverySince = 0; _autoOnEdge = true;
      }
    } else {
      _voltageRecoverySince = 0;
    }
  }

  if (sustainedOverloadActive && !_prevSustainedTrip) _tripOffEdge = true;
  _prevSustainedTrip = sustainedOverloadActive;

  if (!_loadOn) {
    if (irmsA >= LOAD_ON_DETECT_A) {
      if (_loadOnSince == 0) _loadOnSince = now;
      if ((now - _loadOnSince) >= LOAD_ON_DETECT_MS) { _loadOn = true; _loadOffSince = 0; }
    } else _loadOnSince = 0;
  } else {
    if (irmsA <= LOAD_OFF_DETECT_A) {
      if (_loadOffSince == 0) _loadOffSince = now;
      if ((now - _loadOffSince) >= LOAD_OFF_DETECT_MS) { _loadOn = false; _loadOnSince = 0; }
    } else _loadOffSince = 0;
  }

  FaultState st = STATE_NORMAL;
  if (heatActive) st = STATE_HEATING;
  else if (arcActive) st = STATE_ARCING;
  else if (overVoltValid) st = STATE_OVERVOLTAGE;
  else if (underVoltValid && !arcActive) st = STATE_UNDERVOLTAGE;
  else if (sustainedOverloadActive) st = STATE_SUSTAINED_OVERLOAD;
  else if (overloadActive) st = STATE_OVERLOAD;

  _webLockout = (st == STATE_ARCING) || (st == STATE_HEATING) || (st == STATE_OVERLOAD) || (st == STATE_SUSTAINED_OVERLOAD) || _voltageLockout;
  return st;
}

void ProtectionManager::apply(FaultState st, float vDisplay, float vProtect, float i, float t) {
  (void)vDisplay;
  (void)t;
  const uint32_t now = millis();
  updateRelayPulse_();

  const bool arcActive  = (st == STATE_ARCING);
  const bool heatActive = (st == STATE_HEATING);
  const bool underVoltActive = (st == STATE_UNDERVOLTAGE);
  const bool overVoltActive  = (st == STATE_OVERVOLTAGE);
  const bool overloadActive  = (st == STATE_OVERLOAD) || (st == STATE_SUSTAINED_OVERLOAD) || (i >= OVERLOAD_WARN_A);

  static bool mainsStable = false;
  static bool mainsInit = false;
  static uint32_t mainsOffSince = 0;
  static uint32_t mainsOnSince = 0;
  const bool rawOn = (vProtect >= MAINS_PRESENT_ON_V);
  const bool rawOff = (vProtect <= MAINS_PRESENT_OFF_V);
  if (!mainsInit) { mainsStable = rawOn; mainsInit = true; }
  if (rawOff) {
    if (mainsOffSince == 0) mainsOffSince = now;
    mainsOnSince = 0;
    if (mainsStable && (now - mainsOffSince) >= UNPLUGGED_BUZZ_DELAY_MS) mainsStable = false;
  } else if (rawOn) {
    if (mainsOnSince == 0) mainsOnSince = now;
    mainsOffSince = 0;
    if (!mainsStable && (now - mainsOnSince) >= MAINS_EDGE_DEBOUNCE_MS) mainsStable = true;
  } else {
    mainsOffSince = 0;
    mainsOnSince = 0;
  }

  const bool unplugged = rawOff && (mainsOffSince != 0) && ((now - mainsOffSince) >= UNPLUGGED_STATE_DELAY_MS);
  const bool criticalBlock = arcActive || heatActive || underVoltActive || overVoltActive || overloadActive;
  if (unplugged && _relayLatchedOn) pulseRelayOff(LATCH_OFF_PULSE_MS);

  if (!criticalBlock && mainsStable && !_relayLatchedOn) {
    if (i >= LOAD_ON_DETECT_A) {
      if (_loadDetectSince == 0) _loadDetectSince = now;
      if ((now - _loadDetectSince) >= LOAD_ON_DETECT_MS) {
        pulseRelayOn(LATCH_ON_PULSE_MS);
        _loadDetectSince = 0;
      }
    } else {
      _loadDetectSince = 0;
    }
  } else {
    _loadDetectSince = 0;
  }
}
