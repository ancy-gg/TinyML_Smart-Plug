#include "ADS8684.h"
#include <Arduino.h>
#include <esp_timer.h>
#include "driver/gpio.h"

#define ADS8684_DEBUG_BYTES 1
// Set to 1 temporarily if you want to see RX bytes (debug only)
#ifndef ADS8684_DEBUG_BYTES
#define ADS8684_DEBUG_BYTES 0
#endif

bool ADS8684::begin() {
  if (_cfg.pin_cs < 0 || _cfg.pin_sck < 0 || _cfg.pin_miso < 0 || _cfg.pin_mosi < 0) return false;

  // CS idle HIGH
  pinMode(_cfg.pin_cs, OUTPUT);
  digitalWrite(_cfg.pin_cs, HIGH);

  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = _cfg.pin_mosi;
  buscfg.miso_io_num = _cfg.pin_miso;
  buscfg.sclk_io_num = _cfg.pin_sck;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = 32; // bytes (we only use 4)

  esp_err_t err = spi_bus_initialize(_cfg.host, &buscfg, SPI_DMA_CH_AUTO);
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;

  spi_device_interface_config_t devcfg = {};
  devcfg.clock_speed_hz = _cfg.spi_clock_hz;
  devcfg.mode = 1;                 // ADS8684 requires SPI mode 1
  devcfg.spics_io_num = _cfg.pin_cs;
  devcfg.queue_size = 1;

  err = spi_bus_add_device(_cfg.host, &devcfg, &_dev);

  gpio_pulldown_en((gpio_num_t)_cfg.pin_miso);
  gpio_pullup_dis((gpio_num_t)_cfg.pin_miso);
  
  if (err != ESP_OK) return false;

  delay(50);

  return selectAux();
}

esp_err_t ADS8684::xfer32(uint16_t cmd, uint16_t* out_data) {
  if (_dev == nullptr) return ESP_ERR_INVALID_STATE;

  // 32 clocks:
  // - first 16 clocks: command phase (SDO may output zeros)
  // - second 16 clocks: conversion data (MSB first)
  uint8_t tx[4] = { uint8_t(cmd >> 8), uint8_t(cmd & 0xFF), 0x00, 0x00 };
  uint8_t rx[4] = { 0, 0, 0, 0 };

  spi_transaction_t t = {};
  t.length = 32;         // bits
  t.tx_buffer = tx;
  t.rx_buffer = rx;

  esp_err_t err = spi_device_polling_transmit(_dev, &t);
  if (err != ESP_OK) return err;

#if ADS8684_DEBUG_BYTES
  Serial.printf("[ADS SPI] TX:%02X %02X %02X %02X | RX:%02X %02X %02X %02X\n",
                tx[0], tx[1], tx[2], tx[3], rx[0], rx[1], rx[2], rx[3]);
#endif

  // IMPORTANT: conversion code is in rx[2], rx[3]
  if (out_data) {
    *out_data = (uint16_t(rx[2]) << 8) | uint16_t(rx[3]);
  }
  return ESP_OK;
}

bool ADS8684::selectAux() {
  uint16_t dummy = 0;

  // Send MAN_AUX twice to make sure mode is latched and pipeline is aligned
  if (xfer32(0xE000, &dummy) != ESP_OK) return false;
  if (xfer32(0xE000, &dummy) != ESP_OK) return false;

  _auxSelected = true;
  return true;
}

uint16_t ADS8684::readRaw() {
  uint16_t data = 0;

  // Pipeline behavior: first transfer starts/updates conversion,
  // second transfer returns that conversion.
  (void)xfer32(0xE000, &data);
  (void)xfer32(0xE000, &data);

  return data;
}

size_t ADS8684::readRawBurst(uint16_t* dst, size_t n, float* measured_fs_hz) {
  if (!dst || n == 0) return 0;

  uint16_t data = 0;

  // Prime pipeline (discard)
  (void)xfer32(0xE000, &data);

  const int64_t t0 = esp_timer_get_time();

  for (size_t i = 0; i < n; i++) {
    if (xfer32(0xE000, &data) != ESP_OK) return i;
    dst[i] = data;
  }

  const int64_t t1 = esp_timer_get_time();
  const float dt_s = float(t1 - t0) / 1e6f;
  if (measured_fs_hz && dt_s > 0.0f) *measured_fs_hz = float(n) / dt_s;

  return n;
}

float ADS8684::rawToVolts(uint16_t code) const {
  return (float(code) * _vref) / 65535.0f;
}