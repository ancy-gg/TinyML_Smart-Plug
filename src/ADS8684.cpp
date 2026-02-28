#include "ADS8684.h"
#include <Arduino.h>
#include <esp_timer.h>
#include <driver/gpio.h>

#ifndef ADS8684_SHIFT_RIGHT_1
#define ADS8684_SHIFT_RIGHT_1 1
#endif

static inline uint16_t unpack_word(const uint8_t rx[4]) {
  uint16_t w = (uint16_t(rx[2]) << 8) | uint16_t(rx[3]);
#if ADS8684_SHIFT_RIGHT_1
  w >>= 1;
#endif
  return w;
}

static inline void cs_high(int pin) { if (pin >= 0) gpio_set_level((gpio_num_t)pin, 1); }
static inline void cs_low(int pin)  { if (pin >= 0) gpio_set_level((gpio_num_t)pin, 0); }

bool ADS8684::begin() {
  if (_cfg.pin_cs < 0 || _cfg.pin_sck < 0 || _cfg.pin_miso < 0 || _cfg.pin_mosi < 0) return false;

  // Force CS as GPIO output + pull-up (important for RX-as-CS)
  pinMode(_cfg.pin_cs, OUTPUT);
  digitalWrite(_cfg.pin_cs, HIGH);
  gpio_set_pull_mode((gpio_num_t)_cfg.pin_cs, GPIO_PULLUP_ONLY);

  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = _cfg.pin_mosi;
  buscfg.miso_io_num = _cfg.pin_miso;
  buscfg.sclk_io_num = _cfg.pin_sck;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = 32;

  esp_err_t err = spi_bus_initialize(_cfg.host, &buscfg, SPI_DMA_CH_AUTO);
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;

  spi_device_interface_config_t devcfg = {};
  devcfg.clock_speed_hz = _cfg.spi_clock_hz;
  devcfg.mode = 1;
  devcfg.spics_io_num = -1;  // MANUAL CS (we own CS pin)
  devcfg.queue_size = 1;

  err = spi_bus_add_device(_cfg.host, &devcfg, &_dev);
  if (err != ESP_OK) return false;

  delay(20);
  return selectAux();
}

esp_err_t ADS8684::xfer32(uint16_t cmd, uint16_t* out_data) {
  if (_dev == nullptr) return ESP_ERR_INVALID_STATE;

  uint8_t tx[4] = { uint8_t(cmd >> 8), uint8_t(cmd & 0xFF), 0x00, 0x00 };
  uint8_t rx[4] = { 0, 0, 0, 0 };

  spi_transaction_t t = {};
  t.length = 32;
  t.tx_buffer = tx;
  t.rx_buffer = rx;

  cs_low(_cfg.pin_cs);
  esp_err_t err = spi_device_polling_transmit(_dev, &t);
  cs_high(_cfg.pin_cs);
  if (err != ESP_OK) return err;

  if (out_data) *out_data = unpack_word(rx);
  return ESP_OK;
}

bool ADS8684::selectAux() {
  uint16_t dummy = 0;
  if (xfer32(0xE000, &dummy) != ESP_OK) return false; // MAN_AUX
  (void)xfer32(0x0000, &dummy);                       // prime
  _auxSelected = true;
  return true;
}

uint16_t ADS8684::readRaw() {
  uint16_t data = 0;
  (void)xfer32(0x0000, &data);
  (void)xfer32(0x0000, &data);
  return data;
}

size_t ADS8684::readRawBurst(uint16_t* dst, size_t n, float* measured_fs_hz) {
  if (!dst || n == 0) return 0;
  if (_dev == nullptr) return 0;

  (void)spi_device_acquire_bus(_dev, portMAX_DELAY);

  // Constant tx (NOP)
  static const uint8_t tx0[4] = {0x00, 0x00, 0x00, 0x00};
  uint8_t rx[4] = {0,0,0,0};

  spi_transaction_t t = {};
  t.length = 32;
  t.tx_buffer = tx0;
  t.rx_buffer = rx;

  // IMPORTANT: keep CS LOW for the entire burst (more reliable on RX pin)
  cs_low(_cfg.pin_cs);

  // Prime (discard)
  (void)spi_device_polling_transmit(_dev, &t);

  const int64_t t0 = esp_timer_get_time();

  for (size_t i = 0; i < n; i++) {
    esp_err_t err = spi_device_polling_transmit(_dev, &t);
    if (err != ESP_OK) {
      cs_high(_cfg.pin_cs);
      spi_device_release_bus(_dev);
      return i;
    }
    dst[i] = unpack_word(rx);
  }

  const int64_t t1 = esp_timer_get_time();

  cs_high(_cfg.pin_cs);
  spi_device_release_bus(_dev);

  const float dt_s = float(t1 - t0) / 1e6f;
  if (measured_fs_hz && dt_s > 0.0f) *measured_fs_hz = float(n) / dt_s;

  return n;
}

float ADS8684::rawToVolts(uint16_t code) const {
  return (float(code) * _vref) / 65535.0f;
}