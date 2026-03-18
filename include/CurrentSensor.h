#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684
  #include "ADS8684.h"
#elif CURRENT_CAPTURE_BACKEND == CUR_BACKEND_MCP3204
  #include <driver/spi_master.h>
#endif

// CurrentSensor = swappable current waveform source.
// Always returns normalized 16-bit samples (0..65535) regardless of backend.
class CurrentSensor {
public:
  CurrentSensor();

  bool begin();
  void setCalib(const CurrentCalib& cal) { _cal = cal; }
  const CurrentCalib& calib() const { return _cal; }

  // Capture N normalized raw samples. Returns count captured.
  size_t capture(uint16_t* dst, size_t n, float* measuredFsHz = nullptr);

private:
  CurrentCalib _cal;

#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684
  ADS8684 _adc;

#elif CURRENT_CAPTURE_BACKEND == CUR_BACKEND_MCP3204
  spi_device_handle_t _dev = nullptr;
  bool initMcp3204();

#elif CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ESP32_ADC
  bool _started = false;
#endif
};