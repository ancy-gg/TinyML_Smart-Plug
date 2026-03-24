#include "ADS8684.h"
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#ifndef ADS8684_SHIFT_RIGHT_1
#define ADS8684_SHIFT_RIGHT_1 1
#endif

static inline uint16_t unpack_word_at(const uint8_t* rx, size_t frameIndex) {
  const uint8_t* p = rx + (frameIndex * 4U);
  uint16_t w = (uint16_t(p[2]) << 8) | uint16_t(p[3]);
#if ADS8684_SHIFT_RIGHT_1
  w >>= 1;
#endif
  return w;
}

bool ADS8684::ensureIoBuffers_(size_t bytes) {
  if (_bufBytes >= bytes && _txBuf && _rxBuf) return true;

  if (_txBuf) free(_txBuf);
  if (_rxBuf) free(_rxBuf);
  _txBuf = nullptr;
  _rxBuf = nullptr;
  _bufBytes = 0;

  _txBuf = (uint8_t*)malloc(bytes);
  _rxBuf = (uint8_t*)malloc(bytes);
  if (!_txBuf || !_rxBuf) {
    if (_txBuf) free(_txBuf);
    if (_rxBuf) free(_rxBuf);
    _txBuf = nullptr;
    _rxBuf = nullptr;
    return false;
  }

  _bufBytes = bytes;
  return true;
}

bool ADS8684::addDevice(int clock_hz) {
  if (_dev) {
    spi_bus_remove_device(_dev);
    _dev = nullptr;
  }

  spi_device_interface_config_t devcfg = {};
  devcfg.clock_speed_hz = clock_hz;
  devcfg.mode = 1;
  devcfg.spics_io_num = _cfg.pin_cs;
  devcfg.queue_size = 2;
  devcfg.flags = SPI_DEVICE_NO_DUMMY;
  devcfg.cs_ena_pretrans = 0;
  devcfg.cs_ena_posttrans = 0;

  esp_err_t err = spi_bus_add_device(_cfg.host, &devcfg, &_dev);
  if (err != ESP_OK) return false;
  _activeClockHz = clock_hz;
  return true;
}

bool ADS8684::begin() {
  if (_cfg.pin_cs < 0 || _cfg.pin_sck < 0 || _cfg.pin_miso < 0 || _cfg.pin_mosi < 0) return false;

  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = _cfg.pin_mosi;
  buscfg.miso_io_num = _cfg.pin_miso;
  buscfg.sclk_io_num = _cfg.pin_sck;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = int((ADS_BURST_OUTPUT_CHUNK_SAMPLES * ADS_CAPTURE_OVERSAMPLE + 2U) * 4U);

  esp_err_t err = spi_bus_initialize(_cfg.host, &buscfg, SPI_DMA_CH_AUTO);
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;

  if (!addDevice(_cfg.spi_clock_hz)) return false;
  delay(20);
  if (!selectAux() || !probeActivity()) {
    if (!addDevice(ADS_SPI_FALLBACK_HZ)) return false;
    delay(10);
    if (!selectAux() || !probeActivity()) return false;
  }
  return true;
}

esp_err_t ADS8684::xfer32(uint16_t cmd, uint16_t* out_data) {
  if (_dev == nullptr) return ESP_ERR_INVALID_STATE;

  spi_transaction_t t = {};
  t.length = 32;
  t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
  t.tx_data[0] = uint8_t(cmd >> 8);
  t.tx_data[1] = uint8_t(cmd & 0xFF);
  t.tx_data[2] = 0x00;
  t.tx_data[3] = 0x00;

  esp_err_t err = spi_device_polling_transmit(_dev, &t);
  if (err != ESP_OK) return err;
  if (out_data) *out_data = unpack_word_at(t.rx_data, 0);
  return ESP_OK;
}

bool ADS8684::selectAux() {
  uint16_t dummy = 0;
  if (xfer32(0xE000, &dummy) != ESP_OK) return false;
  (void)xfer32(0x0000, &dummy);
  (void)xfer32(0x0000, &dummy);
  _auxSelected = true;
  return true;
}

bool ADS8684::looksInactive_(const uint16_t* dst, size_t n, int minChanges, uint16_t minSpan) const {
  if (!dst || n == 0) return true;

  uint16_t mn = 0xFFFF, mx = 0;
  int changes = 0;
  for (size_t i = 0; i < n; ++i) {
    if (dst[i] < mn) mn = dst[i];
    if (dst[i] > mx) mx = dst[i];
    if (i && dst[i] != dst[i - 1]) changes++;
  }
  return (changes < minChanges) || ((uint16_t)(mx - mn) < minSpan);
}

size_t ADS8684::captureBurst_(uint16_t* dst, size_t n, uint8_t oversample, float* measured_fs_hz) {
  if (!dst || n == 0 || _dev == nullptr) return 0;
  if (oversample < 1) oversample = 1;

  const size_t maxChunkOut = ADS_BURST_OUTPUT_CHUNK_SAMPLES > 0 ? ADS_BURST_OUTPUT_CHUNK_SAMPLES : 256U;
  const size_t maxBytes = ((maxChunkOut * size_t(oversample)) + 2U) * 4U;
  if (!ensureIoBuffers_(maxBytes)) return 0;

  size_t outCount = 0;
  const int64_t t0 = esp_timer_get_time();

  while (outCount < n) {
    const size_t chunkOut = (n - outCount > maxChunkOut) ? maxChunkOut : (n - outCount);
    const size_t rawFrames = chunkOut * size_t(oversample);
    const size_t totalFrames = rawFrames + 2U;
    const size_t bytes = totalFrames * 4U;

    memset(_txBuf, 0, bytes);
    memset(_rxBuf, 0, bytes);

    spi_transaction_t t = {};
    t.length = totalFrames * 32U;
    t.tx_buffer = _txBuf;
    t.rx_buffer = _rxBuf;

    const esp_err_t err = spi_device_polling_transmit(_dev, &t);
    if (err != ESP_OK) break;

    size_t frame = 2U;
    if (oversample == 1U) {
      for (size_t i = 0; i < chunkOut; ++i) {
        dst[outCount++] = unpack_word_at(_rxBuf, frame++);
      }
    } else {
      for (size_t i = 0; i < chunkOut; ++i) {
        uint32_t acc = 0;
        for (uint8_t o = 0; o < oversample; ++o) {
          acc += uint32_t(unpack_word_at(_rxBuf, frame++));
        }
        dst[outCount++] = uint16_t((acc + uint32_t(oversample / 2U)) / uint32_t(oversample));
      }
    }
  }

  const int64_t t1 = esp_timer_get_time();
  if (measured_fs_hz && t1 > t0 && outCount > 0) {
    *measured_fs_hz = (outCount * 1000000.0f) / float(t1 - t0);
  }
  return outCount;
}

bool ADS8684::probeActivity() {
  uint16_t tmp[64];
  float fs = 0.0f;
  const size_t got = captureBurst_(tmp, 64, 1, &fs);
  if (got != 64) return false;
  return !looksInactive_(tmp, got, 4, 2);
}

uint16_t ADS8684::readRaw() {
  uint16_t data = 0;
  if (!_auxSelected && !selectAux()) return 0;
  (void)xfer32(0x0000, &data);
  (void)xfer32(0x0000, &data);
  return data;
}

size_t ADS8684::readRawBurst(uint16_t* dst, size_t n, float* measured_fs_hz) {
  if (!_auxSelected && !selectAux()) return 0;

  size_t got = captureBurst_(dst, n, 1, measured_fs_hz);
  if (got == n && looksInactive_(dst, n, 2, 2)) {
    _auxSelected = false;
    if (!selectAux()) return got;
    got = captureBurst_(dst, n, 1, measured_fs_hz);
  }
  return got;
}

size_t ADS8684::readRawBurstAveraged(uint16_t* dst, size_t n, uint8_t oversample, float* measured_fs_hz) {
  if (!_auxSelected && !selectAux()) return 0;

  size_t got = captureBurst_(dst, n, oversample, measured_fs_hz);
  if (got == n && looksInactive_(dst, n, 2, 2)) {
    _auxSelected = false;
    if (!selectAux()) return got;
    got = captureBurst_(dst, n, oversample, measured_fs_hz);
  }
  return got;
}

float ADS8684::rawToVolts(uint16_t code) const {
  return (float(code) * _vref) / 65535.0f;
}
