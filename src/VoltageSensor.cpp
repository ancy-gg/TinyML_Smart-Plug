#include "VoltageSensor.h"
#include <math.h>

static inline float clampf_vs(float x, float lo, float hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}

static float medianSorted_(float* v, int n) {
  if (n <= 0) return 0.0f;
  for (int i = 0; i < n - 1; ++i) {
    for (int j = i + 1; j < n; ++j) {
      if (v[j] < v[i]) { const float t = v[i]; v[i] = v[j]; v[j] = t; }
    }
  }
  return v[n / 2];
}

static float trimmedMean_(float* v, int n) {
  if (n <= 0) return 0.0f;
  for (int i = 0; i < n - 1; ++i) {
    for (int j = i + 1; j < n; ++j) {
      if (v[j] < v[i]) { const float t = v[i]; v[i] = v[j]; v[j] = t; }
    }
  }
  if (n >= 5) return (v[1] + v[2] + v[3]) / 3.0f;
  if (n == 4) return (v[1] + v[2]) * 0.5f;
  float s = 0.0f;
  for (int i = 0; i < n; ++i) s += v[i];
  return s / (float)n;
}

VoltageSensor::VoltageSensor(int pin) : _pin(pin), _cal{} {}

void VoltageSensor::begin() {
  pinMode(_pin, INPUT);
#if defined(ARDUINO_ARCH_ESP32)
  adcAttachPin(_pin);
  analogSetPinAttenuation(_pin, ADC_11db);
#endif
}

void VoltageSensor::setSensitivity(float factor) { _cal.sensitivity = factor; }
void VoltageSensor::setLinearCalib(float slope, float intercept) { _cal.cubic3 = 0.0f; _cal.cubic2 = 0.0f; _cal.cubic1 = slope; _cal.cubic0 = intercept; }
void VoltageSensor::setCubicCalib(float c3, float c2, float c1, float c0) { _cal.cubic3 = c3; _cal.cubic2 = c2; _cal.cubic1 = c1; _cal.cubic0 = c0; }
void VoltageSensor::setAdcFullScaleVolts(float vfs) { if (vfs > 0.5f && vfs < 5.0f) _adcFullScaleV = vfs; }
void VoltageSensor::setWindowMs(uint16_t ms) { if (ms < 120) ms = 120; if (ms > 800) ms = 800; _windowUs = (uint32_t)ms * 1000UL; }
void VoltageSensor::setClampHysteresis(float v_off, float v_on) { if (v_off < 0) v_off = 0; if (v_on < v_off) v_on = v_off + 1.0f; _vOff = v_off; _vOn = v_on; }
void VoltageSensor::setLongAverage(float tauS, float jumpV) { if (tauS < 0.6f) tauS = 0.6f; if (tauS > 8.0f) tauS = 8.0f; if (jumpV < 6.0f) jumpV = 6.0f; _avgTauS = tauS; _avgJumpV = jumpV; }

float VoltageSensor::update() {
  const uint32_t now = micros();
  if (!_sampling) {
    _sampling = true;
    _startTime = now;
    _count = 0;
    _sum = 0.0;
    _sumSq = 0.0;
    _sampleMin = 4095;
    _sampleMax = 0;
    (void)analogRead(_pin);
    return -1.0f;
  }

  constexpr int FILTERED_SAMPLES_PER_CALL = 10;
  for (int i = 0; i < FILTERED_SAMPLES_PER_CALL; ++i) {
    const int s0 = analogRead(_pin), s1 = analogRead(_pin), s2 = analogRead(_pin), s3 = analogRead(_pin), s4 = analogRead(_pin);
    const int med  = _median5(s0, s1, s2, s3, s4);
    const int mean = (s0 + s1 + s2 + s3 + s4 + 2) / 5;
    const int val  = (4 * med + mean + 2) / 5;

    if (val < _sampleMin) _sampleMin = val;
    if (val > _sampleMax) _sampleMax = val;
    _count++;
    _sum += val;
    _sumSq += (double)val * (double)val;
  }

  if ((uint32_t)(now - _startTime) < _windowUs) return -1.0f;
  _sampling = false;
  if (_count < 60) return _dispVrms;

  const double mean = _sum / (double)_count;
  double var = (_sumSq / (double)_count) - (mean * mean);
  if (var < 0.0) var = 0.0;

  const float vrmsCounts = (float)sqrt(var);
  const int p2pCounts = (_sampleMax >= _sampleMin) ? (_sampleMax - _sampleMin) : 0;
  const float activityRatio = (vrmsCounts > 0.25f) ? (float)p2pCounts / vrmsCounts : 0.0f;

  const float vrmsAdcV  = vrmsCounts * (_adcFullScaleV / 4095.0f);
  const float vrmsUncal = vrmsAdcV * _cal.sensitivity;
  float vrmsMain = eval_cubic_horner(fabsf(vrmsUncal), _cal.cubic3, _cal.cubic2, _cal.cubic1, _cal.cubic0);
  if (vrmsMain < 0.0f) vrmsMain = 0.0f;

  const bool signalTooSmall = (vrmsCounts < 6.5f) || (p2pCounts < 90);
  const bool signalShapeInvalid = (!signalTooSmall) && (activityRatio < 1.75f);
  const bool noRealMains = signalTooSmall || signalShapeInvalid;

  if (noRealMains) {
    if (_noSignalWindows < 255) _noSignalWindows++;
    vrmsMain = 0.0f;
    _vActive = false;
  } else {
    _noSignalWindows = 0;
    if (_vActive) {
      if (vrmsMain < _vOff) {
        _vActive = false;
        vrmsMain = 0.0f;
      }
    } else if (vrmsMain > _vOn) {
      _vActive = true;
    } else {
      vrmsMain = 0.0f;
    }
  }

  const float instantVrms = vrmsMain;
  const bool instantZeroLike  = noRealMains || (instantVrms <= VOLTAGE_SNAP_ZERO_V);
  const bool instantMainsLike = (!noRealMains) && (instantVrms >= VOLTAGE_SNAP_RESTORE_V);

  const bool wasZeroLike =
      (_rawVrms <= _vOff) &&
      (!_protInit || (_protVrms <= _vOff)) &&
      (!_dispInit || (_dispVrms <= _vOff));

  const bool wasLiveLike =
      (_rawVrms >= VOLT_NORMAL_MIN_V) ||
      (_protInit && (_protVrms >= VOLT_NORMAL_MIN_V)) ||
      (_dispInit && (_dispVrms >= VOLT_NORMAL_MIN_V));

  if (instantZeroLike && wasLiveLike) {
    _rawVrms = 0.0f;
    _lowWindows = 1;
    _faultVoteCount = 0;
    _healthyVoteCount = 0;
    _winPos = 0;
    _winCount = 0;
    for (uint8_t i = 0; i < WINDOW_RING; ++i) {
      _winVrms[i] = 0.0f;
      _winValid[i] = 0;
    }
    _protInit = true;
    _protVrms = 0.0f;
    _dispInit = true;
    _dispVrms = 0.0f;
    return _dispVrms;
  }

  if (instantMainsLike && wasZeroLike) {
    _rawVrms = instantVrms;
    _winVrms[0] = instantVrms;
    _winValid[0] = 1;
    _winPos = 1 % WINDOW_RING;
    _winCount = 1;
    _lowWindows = 0;
    _faultVoteCount = 0;
    _healthyVoteCount = 1;
    _protInit = true;
    _protVrms = _rawVrms;
    _dispInit = true;
    _dispVrms = _protVrms;
    return _dispVrms;
  }

  _winVrms[_winPos] = instantVrms;
  _winValid[_winPos] = (uint8_t)(!noRealMains && instantVrms > 0.0f);
  _winPos = (uint8_t)((_winPos + 1U) % WINDOW_RING);
  if (_winCount < WINDOW_RING) _winCount++;

  float validVals[WINDOW_RING];
  int validN = 0;
  for (uint8_t i = 0; i < _winCount; ++i) {
    if (_winValid[i]) validVals[validN++] = _winVrms[i];
  }

  float filteredRaw = 0.0f;
  if (validN >= 3) {
    float tmp[WINDOW_RING];
    for (int i = 0; i < validN; ++i) tmp[i] = validVals[i];
    const float med = medianSorted_(tmp, validN);

    float nearVals[WINDOW_RING];
    int nearN = 0;
    for (int i = 0; i < validN; ++i) {
      if (fabsf(validVals[i] - med) <= fmaxf(12.0f, 0.08f * med)) {
        nearVals[nearN++] = validVals[i];
      }
    }
    if (nearN >= 2) filteredRaw = trimmedMean_(nearVals, nearN);
    else filteredRaw = med;
  } else if (validN > 0 && _rawVrms > 0.0f) {
    filteredRaw = _rawVrms;
  }

  if (filteredRaw <= _vOff || _noSignalWindows >= 2) {
    if (_lowWindows < 255) _lowWindows++;
  } else {
    _lowWindows = 0;
  }

  const bool hardZero = (_lowWindows >= 2) || (_noSignalWindows >= 2) || (validN == 0);
  _rawVrms = hardZero ? 0.0f : filteredRaw;

  const float dtS = (float)_windowUs / 1000000.0f;
  const bool uvCandidate = (_rawVrms >= VOLT_UV_CANDIDATE_RAW_MIN_V) && (_rawVrms < VOLT_NORMAL_MIN_V);
  const bool ovCandidate = (_rawVrms >= VOLT_OV_DELAY_V);
  const bool healthyWindow = (_rawVrms >= VOLT_RECOVER_MIN_V) && (_rawVrms <= VOLT_RECOVER_MAX_V);

  if (!_protInit) {
    _protInit = true;
    _protVrms = _rawVrms;
  } else if (hardZero) {
    _protVrms = 0.0f;
    _faultVoteCount = 0;
    _healthyVoteCount = 0;
  } else {
    if (uvCandidate || ovCandidate) {
      if (_faultVoteCount < 255) _faultVoteCount++;
      _healthyVoteCount = 0;
    } else if (healthyWindow) {
      if (_healthyVoteCount < 255) _healthyVoteCount++;
      _faultVoteCount = 0;
    } else {
      _faultVoteCount = 0;
      _healthyVoteCount = 0;
    }

    const float dProt = fabsf(_rawVrms - _protVrms);
    const bool snapReconnect = (_protVrms <= _vOff) && (_rawVrms >= VOLT_NORMAL_MIN_V) && (_healthyVoteCount >= 1);
    const bool coherentFault = (uvCandidate || ovCandidate) && (_faultVoteCount >= 2);
    const bool healthyRecovery = healthyWindow && (_healthyVoteCount >= 2);

    if (snapReconnect || coherentFault || healthyRecovery || dProt >= _avgJumpV) {
      _protVrms = _rawVrms;
    } else {
      const float alphaProt = fminf(1.0f, dtS / 1.1f);
      _protVrms += alphaProt * (_rawVrms - _protVrms);
      if (_protVrms < 0.25f) _protVrms = 0.0f;
    }
  }

  if (!_dispInit) {
    _dispInit = true;
    _dispVrms = _protVrms;
    return _dispVrms;
  }

  const float delta = fabsf(_protVrms - _dispVrms);
  if (_protVrms <= 0.0f || hardZero) {
    _dispVrms = 0.0f;
  } else if ((_dispVrms <= _vOff && _protVrms >= VOLT_NORMAL_MIN_V) || delta >= 14.0f) {
    _dispVrms = _protVrms;
  } else {
    const float tau = (_protVrms >= _dispVrms) ? _avgTauS : (_avgTauS * 0.8f);
    const float alphaDisp = clampf_vs(dtS / fmaxf(0.2f, tau), 0.0f, 1.0f);
    _dispVrms += alphaDisp * (_protVrms - _dispVrms);
  }

  if (_dispVrms < 0.25f) _dispVrms = 0.0f;
  return _dispVrms;
}
