#include "VoltageSensor.h"
#include <math.h>

VoltageSensor::VoltageSensor(int pin)
  : _pin(pin), _sensitivity(580.0f) {}

void VoltageSensor::begin() {
  pinMode(_pin, INPUT);

#if defined(ARDUINO_ARCH_ESP32)
  adcAttachPin(_pin);
  analogSetPinAttenuation(_pin, ADC_11db);
#endif
}

void VoltageSensor::setSensitivity(float factor) {
  _sensitivity = factor;
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
  if (tauS < 0.5f) tauS = 0.5f;
  if (tauS > 30.0f) tauS = 30.0f;
  if (jumpV < 1.0f) jumpV = 1.0f;
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
    (void)analogRead(_pin); // settle discard
    return -1.0f;
  }

  constexpr int SAMPLES_PER_CALL = 6;
  for (int i = 0; i < SAMPLES_PER_CALL; i++) {
    int a = analogRead(_pin);
    int b = analogRead(_pin);
    int c = analogRead(_pin);
    const int val = _median3(a, b, c);

    _count++;
    _sum   += val;
    _sumSq += (double)val * (double)val;
  }

  if ((uint32_t)(now - _startTime) < _windowUs) {
    return -1.0f;
  }

  _sampling = false;
  if (_count < 32) return 0.0f;

  const double mean = _sum / (double)_count;
  double var = (_sumSq / (double)_count) - (mean * mean);
  if (var < 0.0) var = 0.0;

  const float vrms_counts = (float)sqrt(var);
  const float vrms_adc_v  = vrms_counts * (_adcFullScaleV / 4095.0f);

  float vrms_main = vrms_adc_v * _sensitivity;

  if (_vActive) {
    if (vrms_main < _vOff) {
      _vActive = false;
      vrms_main = 0.0f;
    }
  } else {
    if (vrms_main > _vOn) _vActive = true;
    else                  vrms_main = 0.0f;
  }

  if (!_avgInit) {
    _avgInit = true;
    _avgVrms = vrms_main;
    return _avgVrms;
  }

  const float dtS = (float)_windowUs / 1000000.0f;
  const float slowAlpha = fminf(1.0f, dtS / _avgTauS);
  const float fastAlpha = 0.45f;

  const float delta = fabsf(vrms_main - _avgVrms);

  if (vrms_main <= 0.0f) {
    // Snap down faster for outage detection.
    _avgVrms += 0.55f * (vrms_main - _avgVrms);
    if (_avgVrms < 0.5f) _avgVrms = 0.0f;
  } else if (delta >= _avgJumpV) {
    _avgVrms += fastAlpha * (vrms_main - _avgVrms);
  } else {
    _avgVrms += slowAlpha * (vrms_main - _avgVrms);
  }

  if (_avgVrms < 0.0f) _avgVrms = 0.0f;
  return _avgVrms;
}
