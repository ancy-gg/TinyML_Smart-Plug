#include "CurrentSensor.h"

bool CurrentSensor::begin() {
  return _backend.begin();
}

size_t CurrentSensor::capture(uint16_t* dst, size_t n, float* measuredFsHz) {
  return _backend.capture(dst, n, measuredFsHz);
}
