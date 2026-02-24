#include "ADS8684.h"

bool ADS8684::begin() {
  if (_cfg.pin_cs < 0 || _cfg.pin_sck < 0 || _cfg.pin_miso < 0 || _cfg.pin_mosi < 0) return false;

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
  devcfg.spics_io_num = _cfg.pin_cs;
  devcfg.queue_size = 1;

  err = spi_bus_add_device(_cfg.host, &devcfg, &_dev);
  if (err != ESP_OK) return false;

  // Bumped to 50ms to ensure the ADC has fully booted before we talk to it
  delay(50); 
  return selectAux();
}

esp_err_t ADS8684::xfer32(uint16_t cmd, uint16_t* out_data) {
  // BULLETPROOF BUFFERING:
  // Using explicit arrays bypasses ESP32's 32-bit register shifting quirks entirely.
  // The SPI driver sends tx[0] first, then tx[1], tx[2], tx[3].
  uint8_t tx[4] = { uint8_t(cmd >> 8), uint8_t(cmd & 0xFF), 0x00, 0x00 };
  uint8_t rx[4] = { 0 };

  spi_transaction_t t = {};
  t.length = 32;
  t.tx_buffer = tx;
  t.rx_buffer = rx;
  // Notice we removed the USE_TXDATA and USE_RXDATA flags

  esp_err_t err = spi_device_polling_transmit(_dev, &t);
  if (err != ESP_OK) return err;

  // The SPI driver fills rx[] in the exact order the bytes arrive over the wire.
  // rx[0] is the MSB received, rx[1] is the LSB.
  if (out_data) {
    *out_data = (uint16_t(rx[0]) << 8) | uint16_t(rx[1]);
  }
  return ESP_OK;
}

bool ADS8684::selectAux() {
  uint16_t dummy = 0;
  if (xfer32(0xE000, &dummy) != ESP_OK) return false;
  if (xfer32(0xE000, &dummy) != ESP_OK) return false;
  _auxSelected = true;
  return true;
}

uint16_t ADS8684::readRaw() {
  uint16_t data = 0;
  // Force 0xE000 every time to guarantee it never slips back to auto-scan
  (void)xfer32(0xE000, &data);
  return data;
}

size_t ADS8684::readRawBurst(uint16_t* dst, size_t n, float* measured_fs_hz) {
  if (!dst || n == 0) return 0;
  
  uint16_t data = 0;
  
  // PIPELINE PRIMER: 
  // If the ADC was asleep or lost sync, this forces it immediately into AUX mode.
  xfer32(0xE000, &data); 

  const int64_t t0 = esp_timer_get_time();

  for (size_t i = 0; i < n; i++) {
    // Aggressive Mode Forcing: We send 0xE000 instead of NO_OP (0x0000). 
    // This constantly tells the ADC "Stay in AUX mode!"
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