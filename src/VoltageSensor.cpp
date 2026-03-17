#include "VoltageSensor.h"
#include <math.h>

static inline float hornerLinear(float x, float a, float b) {
  return fmaf(a, x, b);
}

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
  _cal.regSlope = slope;
  _cal.regIntercept = intercept;
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
    (void)analogRead(_pin);
    return -1.0f;
  }

  constexpr int SAMPLES_PER_CALL = 8;
  for (int i = 0; i < SAMPLES_PER_CALL; i++) {
    const int a = analogRead(_pin);
    const int b = analogRead(_pin);
    const int c = analogRead(_pin);
    const int val = _median3(a, b, c);
    _count++;
    _sum   += val;
    _sumSq += (double)val * (double)val;
  }

  if ((uint32_t)(now - _startTime) < _windowUs) {
    return -1.0f;
  }

  _sampling = false;
  if (_count < 32) return _dispVrms;

  const double mean = _sum / (double)_count;
  double var = (_sumSq / (double)_count) - (mean * mean);
  if (var < 0.0) var = 0.0;

  const float vrms_counts = (float)sqrt(var);
  const float vrms_adc_v   = vrms_counts * (_adcFullScaleV / 4095.0f);
  const float vrms_uncal   = vrms_adc_v * _cal.sensitivity;
  float vrms_main          = hornerLinear(vrms_uncal, _cal.regSlope, _cal.regIntercept);
  if (vrms_main < 0.0f) vrms_main = 0.0f;

  if (_vActive) {
    if (vrms_main < _vOff) {
      _vActive = false;
      vrms_main = 0.0f;
    }
  } else {
    if (vrms_main > _vOn) _vActive = true;
    else                  vrms_main = 0.0f;
  }

  _rawVrms = vrms_main;
  const float dtS = (float)_windowUs / 1000000.0f;

  // Protection voltage: keep stable during noise, but jump immediately on real events.
  if (!_protInit) {
    _protInit = true;
    _protVrms = _rawVrms;
  } else {
    const float dProt = fabsf(_rawVrms - _protVrms);
    const bool hardJump = (_rawVrms <= _vOff) ||
                          (dProt >= _avgJumpV) ||
                          (_rawVrms >= (VOLT_SURGE_TRIP_V - 5.0f));

    if (hardJump) {
      _protVrms = _rawVrms;
    } else {
      const float alphaProt = fminf(1.0f, dtS / 1.2f);
      _protVrms += alphaProt * (_rawVrms - _protVrms);
    }

    if (_protVrms < 0.25f) _protVrms = 0.0f;
  }

  // Display voltage: instant on large changes, very long memory when stable.
  if (!_dispInit) {
    _dispInit = true;
    _dispVrms = _protVrms;
    return _dispVrms;
  }

  const float delta = fabsf(_protVrms - _dispVrms);
  if (_protVrms <= 0.0f) {
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
