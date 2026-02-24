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

  // 1. Start a new sampling batch
  if (!_sampling) {
    _sampling = true;
    _startTime = now;
    _sampleCount = 0;
    _sum = 0;
    _sqSum = 0;
    return -1.0f; // Not ready yet
  }

  // 2. Take ONE sample right now
  uint32_t val = analogRead(_pin);
  _sum += val;
  _sqSum += (val * val);
  _sampleCount++;

  // 3. Check if we have sampled long enough (33.3ms)
  if ((now - _startTime) >= _sampleDurationUs) {
    _sampling = false; // Reset for next time

    if (_sampleCount == 0) return 0.0f; // Prevent divide by zero

    // Calculate Mean (This is the exact DC Offset)
    double mean = (double)_sum / _sampleCount;
    
    // Calculate Mean of Squares
    double meanOfSquares = (double)_sqSum / _sampleCount;
    
    // Variance = Mean of Squares - Square of Mean (This removes the DC!)
    double variance = meanOfSquares - (mean * mean);
    
    // If noise causes tiny negative float errors, clamp to 0
    if (variance < 0.0) variance = 0.0; 

    // True RMS in ADC codes
    double rmsRaw = sqrt(variance);

    // Convert raw ADC RMS to actual Volts
    double vRMS_Sensor = rmsRaw * (3.3 / 4095.0);
    
    return (float)(vRMS_Sensor * _sensitivity);
  }

  // 4. Still sampling, return -1 to indicate "wait"
  return -1.0f;
}