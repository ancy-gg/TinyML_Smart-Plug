#include "VoltageSensor.h"
#include <math.h>

VoltageSensor::VoltageSensor(int pin)
  : _pin(pin), _sensitivity(580.0f) {}

void VoltageSensor::begin() {
  pinMode(_pin, INPUT);

#if defined(ARDUINO_ARCH_ESP32)
  // Safe on most Arduino-ESP32 cores. If your core complains, remove.
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

float VoltageSensor::update() {
  const uint32_t now = micros();

  // Start a new window
  if (!_sampling) {
    _sampling  = true;
    _startTime = now;
    _count = 0;
    _sum   = 0.0;
    _sumSq = 0.0;

    // ADC settle discard
    (void)analogRead(_pin);
    return -1.0f;
  }

  // Small burst per call (robust vs spikes).
  // If you ever feel CPU load is high, reduce SAMPLES_PER_CALL to 2 or 3.
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

  // Window complete?
  if ((now - _startTime) < _windowUs) {
    return -1.0f;
  }

  _sampling = false;

  if (_count < 32) return 0.0f;

  // True RMS via variance => removes DC offset automatically
  const double mean = _sum / (double)_count;
  double var = (_sumSq / (double)_count) - (mean * mean);
  if (var < 0.0) var = 0.0;

  const float vrms_counts = (float)sqrt(var);
  const float vrms_adc_v  = vrms_counts * (_adcFullScaleV / 4095.0f);

// Work in ADC-pin Vrms first (calibration-independent)
const float vrms_adc_clean = vrms_adc_v; // already DC-removed by variance

// Clamp/hysteresis in ADC-volts (tune once and forget)
static bool active_adc = false;
const float off_adc_v = 0.020f; // 20mVrms at ADC pin (tune)
const float on_adc_v  = 0.035f; // 35mVrms at ADC pin (tune)

float vrms_adc_out = vrms_adc_clean;

if (active_adc) {
  if (vrms_adc_out < off_adc_v) { active_adc = false; vrms_adc_out = 0.0f; }
} else {
  if (vrms_adc_out > on_adc_v)  { active_adc = true; }
  else                          { vrms_adc_out = 0.0f; }
}

// Now scale to mains volts
const float vrms_clean = vrms_adc_out * _sensitivity;
return vrms_clean;
}