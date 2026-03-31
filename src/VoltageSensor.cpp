#include "VoltageSensor.h"
#include <math.h>

VoltageSensor::VoltageSensor(int pin)
  : _pin(pin), _cal{} {}

void VoltageSensor::begin() {
  pinMode(_pin, INPUT);
#if defined(ARDUINO_ARCH_ESP32)
  adcAttachPin(_pin);
  analogSetPinAttenuation(_pin, ADC_11db);
#endif
}

void VoltageSensor::setSensitivity(float factor) {
  _cal.sensitivity = factor;
}

void VoltageSensor::setLinearCalib(float slope, float intercept) {
  _cal.cubic3 = 0.0f;
  _cal.cubic2 = 0.0f;
  _cal.cubic1 = slope;
  _cal.cubic0 = intercept;
}

void VoltageSensor::setCubicCalib(float c3, float c2, float c1, float c0) {
  _cal.cubic3 = c3;
  _cal.cubic2 = c2;
  _cal.cubic1 = c1;
  _cal.cubic0 = c0;
}

void VoltageSensor::setAdcFullScaleVolts(float vfs) {
  if (vfs > 0.5f && vfs < 5.0f) _adcFullScaleV = vfs;
}

void VoltageSensor::setWindowMs(uint16_t ms) {
  if (ms < 50) ms = 50;
  if (ms > 2000) ms = 2000;
  _windowUs = (uint32_t)ms * 1000UL;
}

void VoltageSensor::setClampHysteresis(float v_off, float v_on) {
  if (v_off < 0) v_off = 0;
  if (v_on < v_off) v_on = v_off + 1.0f;
  _vOff = v_off;
  _vOn  = v_on;
}

void VoltageSensor::setLongAverage(float tauS, float jumpV) {
  if (tauS < 1.0f) tauS = 1.0f;
  if (tauS > 45.0f) tauS = 45.0f;
  if (jumpV < 2.0f) jumpV = 2.0f;
  _avgTauS  = tauS;
  _avgJumpV = jumpV;
}

float VoltageSensor::update() {
  const uint32_t now = micros();

  if (!_sampling) {
    _sampling  = true;
    _startTime = now;
    _count = 0;
    _sum   = 0.0;
    _sumSq = 0.0;
    _sampleMin = 4095;
    _sampleMax = 0;
    (void)analogRead(_pin);
    return -1.0f;
  }

  constexpr int FILTERED_SAMPLES_PER_CALL = 4;
  for (int i = 0; i < FILTERED_SAMPLES_PER_CALL; ++i) {
    const int s0 = analogRead(_pin);
    const int s1 = analogRead(_pin);
    const int s2 = analogRead(_pin);
    const int s3 = analogRead(_pin);
    const int s4 = analogRead(_pin);

    const int med  = _median5(s0, s1, s2, s3, s4);
    const int mean = (s0 + s1 + s2 + s3 + s4 + 2) / 5;
    const int val  = (3 * med + mean + 2) / 4;

    if (val < _sampleMin) _sampleMin = val;
    if (val > _sampleMax) _sampleMax = val;

    _count++;
    _sum   += val;
    _sumSq += (double)val * (double)val;
  }

  if ((uint32_t)(now - _startTime) < _windowUs) return -1.0f;

  _sampling = false;
  if (_count < 24) return _dispVrms;

  const double mean = _sum / (double)_count;
  double var = (_sumSq / (double)_count) - (mean * mean);
  if (var < 0.0) var = 0.0;

  const float vrmsCounts = (float)sqrt(var);
  const int p2pCounts = (_sampleMax >= _sampleMin) ? (_sampleMax - _sampleMin) : 0;

  const float vrmsAdcV   = vrmsCounts * (_adcFullScaleV / 4095.0f);
  const float vrmsUncal  = vrmsAdcV * _cal.sensitivity;
  float vrmsMain = eval_cubic_horner(fabsf(vrmsUncal), _cal.cubic3, _cal.cubic2, _cal.cubic1, _cal.cubic0);
  if (vrmsMain < 0.0f) vrmsMain = 0.0f;

  // Validate that the waveform actually looks like real mains.
  // When unplugged, the ZMPT/ESP32 path can still produce enough variance to map
  // into the undervoltage band unless we also check the waveform activity/shape.
  const float activityRatio = (vrmsCounts > 0.25f) ? (float)p2pCounts / vrmsCounts : 0.0f;
  const bool signalTooSmall = (vrmsCounts < 6.0f) || (p2pCounts < 80);
  const bool signalShapeInvalid = (!signalTooSmall) && (activityRatio < 1.65f);
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

  _rawVrms = vrmsMain;
  const float dtS = (float)_windowUs / 1000000.0f;

  if (!_protInit) {
    _protInit = true;
    _protVrms = _rawVrms;
  } else {
    const float dProt = fabsf(_rawVrms - _protVrms);
    const bool highEvent = (_rawVrms >= (VOLT_SURGE_TRIP_V - 5.0f));

    if (_rawVrms <= _vOff) {
      if (_lowWindows < 255) _lowWindows++;
    } else {
      _lowWindows = 0;
    }

    const bool hardZero = (_rawVrms <= _vOff && _lowWindows >= 2) || (_noSignalWindows >= 2);

    if (hardZero) {
      _protVrms = 0.0f;
    } else if (highEvent || dProt >= _avgJumpV) {
      _protVrms = _rawVrms;
    } else {
      const float alphaProt = fminf(1.0f, dtS / 1.0f);
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
  if (_protVrms <= 0.0f || _noSignalWindows >= 2) {
    _dispVrms = 0.0f;
  } else if (delta >= _avgJumpV) {
    _dispVrms = _protVrms;
  } else {
    const float slowAlpha = fminf(1.0f, dtS / _avgTauS);
    _dispVrms += slowAlpha * (_protVrms - _dispVrms);
  }

  if (_dispVrms < 0.0f) _dispVrms = 0.0f;
  return _dispVrms;
}
