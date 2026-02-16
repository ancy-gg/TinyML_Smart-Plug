#pragma once
#include <Arduino.h>
#include "ADS8684.h"
#include "SmartPlugConfig.h"

// CurrentSensor now = ADS8684 AUX reader (16-bit raw)
class CurrentSensor {
public:
  CurrentSensor(); // uses SmartPlugConfig pins

  bool begin();
  void setCalib(const CurrentCalib& cal) { _cal = cal; }
  const CurrentCalib& calib() const { return _cal; }

  // Capture N raw samples (0..65535). Returns count.
  size_t capture(uint16_t* dst, size_t n, float* measuredFsHz = nullptr);

private:
  ADS8684 _adc;
  CurrentCalib _cal;
};