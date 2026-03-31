#pragma once
#include <Arduino.h>
#include <driver/spi_master.h>
#include "MainConfiguration.h"

class CurrentSensor {
public:
  CurrentSensor() = default;
  bool begin();
  void setCalib(const CurrentCalib& cal) { _cal = cal; }
  const CurrentCalib& calib() const { return _cal; }
  size_t capture(uint16_t* dst, size_t n, float* measuredFsHz = nullptr);

private:
  bool initMcp3204_();
  CurrentCalib _cal;
  spi_device_handle_t _dev = nullptr;
};
