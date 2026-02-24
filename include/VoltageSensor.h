#ifndef VOLTAGE_SENSOR_H
#define VOLTAGE_SENSOR_H

#include <Arduino.h>
#include "SmartPlugConfig.h"

class VoltageSensor {
public:
  VoltageSensor(int pin);
  void begin();
  void setSensitivity(float factor);

  // Returns -1.0 if still sampling, or the True RMS value if ready
  float update(); 

private:
  int _pin;
  float _sensitivity;

  bool _sampling = false;
  uint32_t _startTime = 0;
  
  // Accumulators for True RMS
  uint32_t _sampleCount = 0;
  uint64_t _sum = 0;
  uint64_t _sqSum = 0;
  
  const uint32_t _sampleDurationUs = (uint32_t)(2.0f * (1000000.0f / MAINS_F0_HZ));
};

#endif