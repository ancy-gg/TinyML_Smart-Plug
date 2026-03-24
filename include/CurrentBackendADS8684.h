#pragma once
#include <Arduino.h>
#include "ADS8684.h"

class CurrentBackendADS8684 {
public:
  CurrentBackendADS8684();
  bool begin();
  size_t capture(uint16_t* dst, size_t n, float* measuredFsHz = nullptr);

private:
  ADS8684 _adc;
};
