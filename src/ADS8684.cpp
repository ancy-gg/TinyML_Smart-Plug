#include "ADS8684.h"
#include <Arduino.h>
#include <esp_timer.h>

// =================== USER TUNABLES ===================
// TI forum indicates SPI Mode 1 (CPOL=0, CPHA=1). :contentReference[oaicite:2]{index=2}
// If you still see a consistent "x2" code, keep MODE=1 and enable SHIFT_RIGHT_1 below.
#ifndef ADS8684_SPI_MODE
#define ADS8684_SPI_MODE 1
#endif

// If your code looks ~2x expected (e.g., 1.85V reads ~60000), enable this.
// This compensates a 1-bit left-shift capture (drops the lost LSB).
#ifndef ADS8684_SHIFT_RIGHT_1
#define ADS8684_SHIFT_RIGHT_1 1
#endif

// Debug: prints one line every N transfers (no flooding)
#ifndef ADS8684_DEBUG_BYTES
#define ADS8684_DEBUG_BYTES 0
#endif
#ifndef ADS8684_DEBUG_EVERY_N
#define ADS8684_DEBUG_EVERY_N 2000
#endif
// =====================================================

static inline uint16_t unpack_conv_word(const uint8_t rx[4]) {
  // Conversion result appears after the first 16 SCLKs; we capture it in rx[2], rx[3]. :contentReference[oaicite:3]{index=3}
  uint16_t w = (uint16_t(rx[2]) << 8) | uint16_t(rx[3]);
#if ADS8684_SHIFT_RIGHT_1
  w >>= 1;
#endif
  return w;
}

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
  devcfg.mode = ADS8684_SPI_MODE;
  devcfg.spics_io_num = _cfg.pin_cs;
  devcfg.queue_size = 1;

  err = spi_bus_add_device(_cfg.host, &devcfg, &_dev);
  if (err != ESP_OK) return false;

  delay(50);

  // Enter MAN_AUX once. :contentReference[oaicite:4]{index=4}
  return selectAux();
}

esp_err_t ADS8684::xfer32(uint16_t cmd, uint16_t* out_data) {
  if (_dev == nullptr) return ESP_ERR_INVALID_STATE;

  uint8_t tx[4] = { uint8_t(cmd >> 8), uint8_t(cmd & 0xFF), 0x00, 0x00 };
  uint8_t rx[4] = { 0, 0, 0, 0 };

  spi_transaction_t t = {};
  t.length = 32; // bits
  t.tx_buffer = tx;
  t.rx_buffer = rx;

  esp_err_t err = spi_device_polling_transmit(_dev, &t);
  if (err != ESP_OK) return err;

#if ADS8684_DEBUG_BYTES
  static uint32_t n = 0;
  n++;
  if ((n % ADS8684_DEBUG_EVERY_N) == 0) {
    Serial.printf("[ADS SPI] TX:%02X %02X %02X %02X | RX:%02X %02X %02X %02X | WORD:0x%04X\n",
                  tx[0], tx[1], tx[2], tx[3],
                  rx[0], rx[1], rx[2], rx[3],
                  unpack_conv_word(rx));
  }
#endif

  if (out_data) *out_data = unpack_conv_word(rx);
  return ESP_OK;
}

bool ADS8684::selectAux() {
  uint16_t dummy = 0;

  // Select AUX mode (MAN_AUX = 0xE000). :contentReference[oaicite:5]{index=5}
  if (xfer32(0xE000, &dummy) != ESP_OK) return false;

  // After mode select, keep SDI low (NO_OP = 0x0000) to continue in selected mode. :contentReference[oaicite:6]{index=6}
  // Also primes the pipeline.
  (void)xfer32(0x0000, &dummy);

  _auxSelected = true;
  return true;
}

uint16_t ADS8684::readRaw() {
  uint16_t data = 0;

  // In MAN_AUX, use NO_OP frames for continuous conversions. :contentReference[oaicite:7]{index=7}
  // First NO_OP returns previous conversion; second returns current.
  (void)xfer32(0x0000, &data);
  (void)xfer32(0x0000, &data);

  return data;
}

size_t ADS8684::readRawBurst(uint16_t* dst, size_t n, float* measured_fs_hz) {
  if (!dst || n == 0) return 0;

  uint16_t data = 0;

  // Prime pipeline (discard)
  (void)xfer32(0x0000, &data);

  const int64_t t0 = esp_timer_get_time();

  for (size_t i = 0; i < n; i++) {
    if (xfer32(0x0000, &data) != ESP_OK) return i;
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