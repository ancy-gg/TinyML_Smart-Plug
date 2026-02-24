#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

class VoltageSensor {
public:
  VoltageSensor(int pin);
  void begin();
  void setSensitivity(float factor);

  // Returns -1.0 if still sampling, or the RMS value if ready
  float update(); 

private:
  int _pin;
  float _sensitivity;

  // Non-blocking state variables
  bool _sampling = false;
  uint32_t _startTime = 0;
  int _maxVal = 0;
  int _minVal = 4095;
  
  // We need to sample for at least 2 mains cycles (33.3ms at 60Hz)
  const uint32_t _sampleDurationUs = (uint32_t)(2.0f * (1000000.0f / MAINS_F0_HZ));
};