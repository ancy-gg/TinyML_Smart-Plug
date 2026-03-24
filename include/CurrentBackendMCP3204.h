#pragma once
#include <Arduino.h>
#include <driver/spi_master.h>

class CurrentBackendMCP3204 {
public:
  CurrentBackendMCP3204() = default;
  bool begin();
  size_t capture(uint16_t* dst, size_t n, float* measuredFsHz = nullptr);

private:
  bool initMcp3204();
  spi_device_handle_t _dev = nullptr;
};
