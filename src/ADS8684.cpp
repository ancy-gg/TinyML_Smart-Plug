#include "ADS8684.h"

static inline uint16_t u16_from_rx(const uint8_t rx[4]) {
  return (uint16_t(rx[0]) << 8) | uint16_t(rx[1]);
}

bool ADS8684::begin() {
  if (_cfg.pin_cs < 0 || _cfg.pin_sck < 0 || _cfg.pin_miso < 0 || _cfg.pin_mosi < 0) return false;

  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = _cfg.pin_mosi;
  buscfg.miso_io_num = _cfg.pin_miso;
  buscfg.sclk_io_num = _cfg.pin_sck;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = 4;

  esp_err_t err = spi_bus_initialize(_cfg.host, &buscfg, SPI_DMA_CH_AUTO);
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;

  spi_device_interface_config_t devcfg = {};
  devcfg.clock_speed_hz = _cfg.spi_clock_hz;
  devcfg.mode = 1; // ADS868x timing works reliably in mode 1
  devcfg.spics_io_num = _cfg.pin_cs;
  devcfg.queue_size = 1;

  err = spi_bus_add_device(_cfg.host, &devcfg, &_dev);
  if (err != ESP_OK) return false;

  delay(20); // let internal ref settle
  return selectAux();
}

esp_err_t ADS8684::xfer32(uint16_t cmd, uint16_t* out_data) {
  spi_transaction_t t = {};
  t.length = 32;
  t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;

  // 16-bit cmd then 16 dummy bits
  t.tx_data[0] = uint8_t(cmd >> 8);
  t.tx_data[1] = uint8_t(cmd & 0xFF);
  t.tx_data[2] = 0x00;
  t.tx_data[3] = 0x00;

  esp_err_t err = spi_device_polling_transmit(_dev, &t);
  if (err != ESP_OK) return err;

  // ---- THE DIAGNOSTIC TRIPWIRE ----
  // This will print the raw hex bytes to the terminal once every 4096 samples
  static int counter = 0;
  if (counter++ % 4096 == 0) {
    Serial.printf("\n[SPI RAW BYTES] rx0:%02X  rx1:%02X  rx2:%02X  rx3:%02X", 
                   t.rx_data[0], t.rx_data[1], t.rx_data[2], t.rx_data[3]);
  }
  // ---------------------------------

  // Temporarily return rx[0] and rx[1] just to keep the code compiling
  if (out_data) *out_data = (uint16_t(t.rx_data[0]) << 8) | uint16_t(t.rx_data[1]);
  return ESP_OK;
}

bool ADS8684::selectAux() {
  // MAN_AUX = 0xE000 (The TRUE command for the AUX pin)
  // Needs one dummy read to prime the pipeline
  uint16_t dummy = 0;
  if (xfer32(0xE000, &dummy) != ESP_OK) return false;
  if (xfer32(0x0000, &dummy) != ESP_OK) return false;
  _auxSelected = true;
  return true;
}

uint16_t ADS8684::readRaw() {
  if (!_auxSelected) {
    if (!selectAux()) return 0;
  }
  uint16_t data = 0;
  (void)xfer32(0x0000, &data);
  return data;
}

size_t ADS8684::readRawBurst(uint16_t* dst, size_t n, float* measured_fs_hz) {
  if (!dst || n == 0) return 0;
  if (!_auxSelected) {
    if (!selectAux()) return 0;
  }

  const int64_t t0 = esp_timer_get_time();
  uint16_t data = 0;

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