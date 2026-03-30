#include "FaultLogic.h"

static inline int clampi(int x, int lo, int hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}

static inline float heatTripTempC() {
#if COLLECTION_ONLY_MODE
  return TEMP_DATA_WARN_C;
#else
  return TEMP_TRIP_C;
#endif
}

void FaultLogic::resetLatch() {
  _arcCnt = 0;
  _heatFrames = 0;
  _arcHoldUntil = 0;
  _heatHoldUntil = 0;
  _underVoltSince = 0;
  _overVoltSince = 0;
  _overloadSince = 0;
  _sustainedOverloadSince = 0;
  _voltageLockout = false;
  _voltageLockoutKind = STATE_NORMAL;
  _voltageRecoverySince = 0;
  _tripOffEdge = false;
  _autoOnEdge = false;
  _webLockout = false;
  _prevSustainedTrip = false;
  _loadOn = false;
  _loadOnSince = 0;
  _loadOffSince = 0;
}

bool FaultLogic::consumeTripOffEdge() {
  const bool v = _tripOffEdge;
  _tripOffEdge = false;
  return v;
}

bool FaultLogic::consumeAutoOnEdge() {
  const bool v = _autoOnEdge;
  _autoOnEdge = false;
  return v;
}

FaultState FaultLogic::update(float vProtect, float tempC, float irmsA, int arcModelOut, bool arcEligible) {
  const uint32_t now = millis();

  if (!arcEligible || irmsA < ARC_MIN_IRMS_A) arcModelOut = 0;

  if (arcModelOut == 1) _arcCnt += ARC_CNT_INC;
  else                  _arcCnt -= ARC_CNT_DEC;
  _arcCnt = clampi(_arcCnt, 0, ARC_CNT_MAX);
  const bool arcTrip = (_arcCnt >= ARC_CNT_TRIP);

  if (tempC >= heatTripTempC()) _heatFrames++;
  else _heatFrames = clampi(_heatFrames - HEAT_FRAMES_DEC, 0, HEAT_FRAMES_TRIP);
  const bool heatTrip = (_heatFrames >= HEAT_FRAMES_TRIP);

  if (arcTrip)  _arcHoldUntil  = now + ARC_HOLD_MS;
  if (heatTrip) _heatHoldUntil = now + HEAT_HOLD_MS;

  const bool arcActive  = arcTrip  || (now < _arcHoldUntil);
  const bool heatActive = heatTrip || (now < _heatHoldUntil);

  const bool underVoltRaw = (vProtect >= MAINS_PRESENT_ON_V && vProtect < VOLT_UNDERVOLT_MAX_V);
  const bool overVoltRaw  = (vProtect >= VOLT_OVERVOLT_TRIP_V);
  const bool extremeUnder = (vProtect > 0.0f && vProtect <= EXTREME_UNDERVOLT_FAST_V);
  const bool extremeOver  = (vProtect >= EXTREME_OVERVOLT_FAST_V);

  bool underVoltValid = false;
  bool overVoltValid = false;

  if (underVoltRaw) {
    if (_underVoltSince == 0) _underVoltSince = now;
    const uint32_t need = extremeUnder ? EXTREME_VOLTAGE_VALIDATE_MS : VOLTAGE_EVENT_VALIDATE_MS;
    underVoltValid = ((now - _underVoltSince) >= need);
  } else {
    _underVoltSince = 0;
  }

  if (overVoltRaw) {
    if (_overVoltSince == 0) _overVoltSince = now;
    const uint32_t need = extremeOver ? EXTREME_VOLTAGE_VALIDATE_MS : VOLTAGE_EVENT_VALIDATE_MS;
    overVoltValid = ((now - _overVoltSince) >= need);
  } else {
    _overVoltSince = 0;
  }

  const bool overloadRaw = (irmsA >= OVERLOAD_WARN_A);
  if (overloadRaw) {
    if (_overloadSince == 0) _overloadSince = now;
  } else {
    _overloadSince = 0;
  }
  const bool overloadActive = overloadRaw && ((now - _overloadSince) >= OVERLOAD_TRIP_MS);

  const bool sustainedOverloadRaw = (irmsA >= SUSTAINED_OVERLOAD_TRIP_A);
  if (sustainedOverloadRaw) {
    if (_sustainedOverloadSince == 0) _sustainedOverloadSince = now;
  } else {
    _sustainedOverloadSince = 0;
  }
  const bool sustainedOverloadActive = sustainedOverloadRaw && ((now - _sustainedOverloadSince) >= SUSTAINED_OVERLOAD_TRIP_MS);

  if (underVoltValid || overVoltValid) {
    const FaultState kind = underVoltValid ? STATE_UNDERVOLTAGE : STATE_OVERVOLTAGE;
    if (!_voltageLockout || _voltageLockoutKind != kind) {
      _tripOffEdge = true;
    }
    _voltageLockout = true;
    _voltageLockoutKind = kind;
    _voltageRecoverySince = 0;
  } else if (_voltageLockout) {
    if (_voltageRecoverySince == 0) _voltageRecoverySince = now;
    if ((now - _voltageRecoverySince) >= VOLTAGE_RECLOSE_STABLE_MS) {
      _voltageLockout = false;
      _voltageLockoutKind = STATE_NORMAL;
      _voltageRecoverySince = 0;
      _autoOnEdge = true;
    }
  }

  if (sustainedOverloadActive && !_prevSustainedTrip) {
    _tripOffEdge = true;
  }
  _prevSustainedTrip = sustainedOverloadActive;

  if (!_loadOn) {
    if (irmsA >= LOAD_ON_DETECT_A) {
      if (_loadOnSince == 0) _loadOnSince = now;
      if ((now - _loadOnSince) >= LOAD_ON_DETECT_MS) {
        _loadOn = true;
        _loadOffSince = 0;
      }
    } else {
      _loadOnSince = 0;
    }
  } else {
    if (irmsA <= LOAD_OFF_DETECT_A) {
      if (_loadOffSince == 0) _loadOffSince = now;
      if ((now - _loadOffSince) >= LOAD_OFF_DETECT_MS) {
        _loadOn = false;
        _loadOnSince = 0;
      }
    } else {
      _loadOffSince = 0;
    }
  }

  FaultState st = STATE_NORMAL;
  if (heatActive) st = STATE_HEATING;
  else if (arcActive) st = STATE_ARCING;
  else if (_voltageLockout) st = _voltageLockoutKind;
  else if (sustainedOverloadActive) st = STATE_SUSTAINED_OVERLOAD;
  else if (overloadActive) st = STATE_OVERLOAD;
  else st = STATE_NORMAL;

  _webLockout = (st == STATE_ARCING) ||
                (st == STATE_HEATING) ||
                (st == STATE_UNDERVOLTAGE) ||
                (st == STATE_OVERVOLTAGE) ||
                (st == STATE_OVERLOAD) ||
                (st == STATE_SUSTAINED_OVERLOAD);

  return st;
}
