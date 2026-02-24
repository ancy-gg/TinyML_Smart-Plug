#include "VoltageSensor.h"

VoltageSensor::VoltageSensor(int pin) : _pin(pin), _sensitivity(580.0f) {}

void VoltageSensor::begin() {
  pinMode(_pin, INPUT);
}

void VoltageSensor::setSensitivity(float factor) {
  _sensitivity = factor;
}

float VoltageSensor::update() {
  uint32_t now = micros();

  // 1. Start a new sampling batch if not already running
  if (!_sampling) {
    _sampling = true;
    _startTime = now;
    _maxVal = 0;
    _minVal = 4095;
    return -1.0f; // Not ready yet
  }

  // 2. Take ONE sample right now (very fast)
  int val = analogRead(_pin);
  if (val > _maxVal) _maxVal = val;
  if (val < _minVal) _minVal = val;

  // 3. Check if we have sampled long enough (33ms)
  if ((now - _startTime) >= _sampleDurationUs) {
    // Done! Calculate RMS
    _sampling = false; // Reset for next time

    if (_maxVal - _minVal < 5) return 0.0f; // Noise filter

    float vPeakToPeak = (_maxVal - _minVal) * (3.3f / 4095.0f);
    float vRMS_Sensor = (vPeakToPeak * 0.5f) * 0.70710678f;
    return vRMS_Sensor * _sensitivity;
  }

  // 4. Still sampling, return -1 to indicate "wait"
  return -1.0f;
}