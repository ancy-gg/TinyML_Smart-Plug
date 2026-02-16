#pragma once
#include <Arduino.h>
#include <driver/spi_master.h>
#include <esp_timer.h>
#include "SmartPlugConfig.h"

// AUX-only ADS8684 (16-bit)
class ADS8684 {
public:
    struct Config {
    spi_host_device_t host;
    int pin_cs;
    int pin_sck;
    int pin_miso;
    int pin_mosi;
    int spi_clock_hz;
    };

  explicit ADS8684(const Config& cfg) : _cfg(cfg) {}

  bool begin();
  uint16_t readRaw();
  size_t readRawBurst(uint16_t* dst, size_t n, float* measured_fs_hz = nullptr);

  float rawToVolts(uint16_t code) const;

private:
  Config _cfg {};
  spi_device_handle_t _dev = nullptr;
  bool _auxSelected = false;

  float _vref = ADS_VREF_V;

  esp_err_t xfer32(uint16_t cmd, uint16_t* out_data);
  bool selectAux();
};