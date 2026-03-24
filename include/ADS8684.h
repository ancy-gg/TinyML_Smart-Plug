#pragma once
#include <Arduino.h>
#include <driver/spi_master.h>
#include <esp_timer.h>
#include "SmartPlugConfig.h"

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
  size_t readRawBurstAveraged(uint16_t* dst, size_t n, uint8_t oversample, float* measured_fs_hz = nullptr);
  float rawToVolts(uint16_t code) const;

private:
  Config _cfg {};
  spi_device_handle_t _dev = nullptr;
  bool _auxSelected = false;
  float _vref = ADS_VREF_V;
  int _activeClockHz = 0;

  esp_err_t xfer32(uint16_t cmd, uint16_t* out_data);
  bool selectAux();
  bool addDevice(int clock_hz);
  bool probeActivity();
  bool looksInactive_(const uint16_t* dst, size_t n, int minChanges, uint16_t minSpan) const;
  size_t readRawBurstInternal_(uint16_t* dst, size_t n, uint8_t oversample, float* measured_fs_hz);
};