#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684
  #include "CurrentBackendADS8684.h"
  using SelectedCurrentBackend = CurrentBackendADS8684;
#else
  #include "CurrentBackendMCP3204.h"
  using SelectedCurrentBackend = CurrentBackendMCP3204;
#endif

class CurrentSensor {
public:
  CurrentSensor() = default;

  bool begin();
  void setCalib(const CurrentCalib& cal) { _cal = cal; }
  const CurrentCalib& calib() const { return _cal; }
  size_t capture(uint16_t* dst, size_t n, float* measuredFsHz = nullptr);

private:
  CurrentCalib _cal;
  SelectedCurrentBackend _backend;
};
