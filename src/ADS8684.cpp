#include "ADS8684.h"
#include <Arduino.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <soc/gpio_struct.h>

#ifndef ADS8684_SHIFT_RIGHT_1
#define ADS8684_SHIFT_RIGHT_1 1
#endif

static constexpr int ADS_SPI_RECOVERY_HZ = 2000000;
static constexpr uint8_t ADS_SELECT_AUX_FLUSH = 4;

static inline uint16_t unpack_word(const uint8_t rx[4]) {
  // Keep the legacy right-shift that previously made the AUX path line up on
  // this board. The no-shift variant caused startup inactivity and the sensing
  // task never came up.
  uint16_t w = (uint16_t(rx[2]) << 8) | uint16_t(rx[3]);
#if ADS8684_SHIFT_RIGHT_1
  w >>= 1;
#endif
  return w;
}

static inline __attribute__((always_inline)) void cs_high(int pin) {
  if (pin < 0) return;
  if (pin < 32) GPIO.out_w1ts = (1UL << pin);
  else GPIO.out1_w1ts.val = (1UL << (pin - 32));
}
static inline __attribute__((always_inline)) void cs_low(int pin) {
  if (pin < 0) return;
  if (pin < 32) GPIO.out_w1tc = (1UL << pin);
  else GPIO.out1_w1tc.val = (1UL << (pin - 32));
}
static inline void cs_hold_enable(int pin)  { if (pin >= 0) gpio_hold_en((gpio_num_t)pin); }
static inline void cs_hold_disable(int pin) { if (pin >= 0) gpio_hold_dis((gpio_num_t)pin); }

static inline bool sameClock_(int a, int b) {
  return a == b;
}

bool ADS8684::addDevice(int clock_hz) {
  if (_dev) {
    spi_bus_remove_device(_dev);
    _dev = nullptr;
  }

  spi_device_interface_config_t devcfg = {};
  devcfg.clock_speed_hz = clock_hz;
  devcfg.mode = 1;
  devcfg.spics_io_num = -1;
  devcfg.queue_size = 1;
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

  pinMode(_cfg.pin_cs, OUTPUT);
  digitalWrite(_cfg.pin_cs, HIGH);
  gpio_set_pull_mode((gpio_num_t)_cfg.pin_cs, GPIO_PULLUP_ONLY);
  gpio_set_drive_capability((gpio_num_t)_cfg.pin_cs, GPIO_DRIVE_CAP_3);

  cs_hold_disable(_cfg.pin_cs);
  cs_high(_cfg.pin_cs);
  cs_hold_enable(_cfg.pin_cs);

  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = _cfg.pin_mosi;
  buscfg.miso_io_num = _cfg.pin_miso;
  buscfg.sclk_io_num = _cfg.pin_sck;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = 32;

  esp_err_t err = spi_bus_initialize(_cfg.host, &buscfg, SPI_DMA_CH_AUTO);
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;

  // Start conservatively. If the digital timing is marginal, 4 MHz is usually
  // much more forgiving than 8 MHz, but still fast enough for this project.
  const int clocks[] = { ADS_SPI_FALLBACK_HZ, _cfg.spi_clock_hz, ADS_SPI_RECOVERY_HZ };
  bool haveAux = false;

  for (size_t i = 0; i < (sizeof(clocks) / sizeof(clocks[0])); ++i) {
    const int hz = clocks[i];
    if (hz <= 0) continue;
    if (i > 0 && sameClock_(hz, clocks[i - 1])) continue;
    if (!addDevice(hz)) continue;
    delay((i == 0) ? 20 : 12);

    if (!selectAux()) continue;
    haveAux = true;

    // Runtime capture is the real source of truth. Do not brick the current
    // pipeline just because the startup probe happened during a quiet instant.
    if (probeActivity()) return true;
  }

  return haveAux;
}

esp_err_t ADS8684::xfer32(uint16_t cmd, uint16_t* out_data) {
  if (_dev == nullptr) return ESP_ERR_INVALID_STATE;

  spi_transaction_t t = {};
  t.length = 32;
  t.rxlength = 32;
  t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
  t.tx_data[0] = uint8_t(cmd >> 8);
  t.tx_data[1] = uint8_t(cmd & 0xFF);
  t.tx_data[2] = 0x00;
  t.tx_data[3] = 0x00;

  cs_hold_disable(_cfg.pin_cs);
  cs_low(_cfg.pin_cs);
  esp_err_t err = spi_device_polling_transmit(_dev, &t);
  cs_high(_cfg.pin_cs);
  cs_hold_enable(_cfg.pin_cs);

  if (err != ESP_OK) return err;
  if (out_data) *out_data = unpack_word(t.rx_data);
  return ESP_OK;
}

bool ADS8684::selectAux() {
  uint16_t dummy = 0;
  if (xfer32(0xE000, &dummy) != ESP_OK) return false;
  for (uint8_t i = 0; i < ADS_SELECT_AUX_FLUSH; ++i) {
    if (xfer32(0x0000, &dummy) != ESP_OK) return false;
  }
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

size_t ADS8684::readRawBurstInternal_(uint16_t* dst, size_t n, uint8_t oversample, float* measured_fs_hz) {
  if (measured_fs_hz) *measured_fs_hz = 0.0f;
  if (!dst || n == 0 || _dev == nullptr) return 0;
  if (oversample < 1) oversample = 1;

  (void)spi_device_acquire_bus(_dev, portMAX_DELAY);

  spi_transaction_t t = {};
  t.length = 32;
  t.rxlength = 32;
  t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
  t.tx_data[0] = 0x00;
  t.tx_data[1] = 0x00;
  t.tx_data[2] = 0x00;
  t.tx_data[3] = 0x00;

  cs_hold_disable(_cfg.pin_cs);

  for (uint8_t i = 0; i < ADS_SELECT_AUX_FLUSH; ++i) {
    cs_low(_cfg.pin_cs);
    (void)spi_device_polling_transmit(_dev, &t);
    cs_high(_cfg.pin_cs);
  }

  const int64_t t0 = esp_timer_get_time();
  size_t count = 0;

  for (; count < n; ++count) {
    uint32_t acc = 0;

    for (uint8_t o = 0; o < oversample; ++o) {
      cs_low(_cfg.pin_cs);
      esp_err_t err = spi_device_polling_transmit(_dev, &t);
      cs_high(_cfg.pin_cs);

      if (err != ESP_OK) {
        cs_high(_cfg.pin_cs);
        cs_hold_enable(_cfg.pin_cs);
        spi_device_release_bus(_dev);
        const int64_t t1 = esp_timer_get_time();
        if (measured_fs_hz && t1 > t0 && count > 0) {
          *measured_fs_hz = (count * 1000000.0f) / float(t1 - t0);
        }
        return count;
      }

      acc += uint32_t(unpack_word(t.rx_data));
    }

    dst[count] = uint16_t((acc + uint32_t(oversample / 2U)) / uint32_t(oversample));
  }

  const int64_t t1 = esp_timer_get_time();

  cs_high(_cfg.pin_cs);
  cs_hold_enable(_cfg.pin_cs);
  spi_device_release_bus(_dev);

  if (measured_fs_hz && t1 > t0 && count > 0) {
    *measured_fs_hz = (count * 1000000.0f) / float(t1 - t0);
  }

  return count;
}

bool ADS8684::probeActivity() {
  uint16_t tmp[64];
  float fs = 0.0f;
  const size_t got = readRawBurstInternal_(tmp, 64, 1, &fs);
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

  size_t got = readRawBurstInternal_(dst, n, 1, measured_fs_hz);
  if (got == n && !looksInactive_(dst, n, 2, 2)) return got;

  const int clocks[] = { _activeClockHz, ADS_SPI_FALLBACK_HZ, ADS_SPI_RECOVERY_HZ, _cfg.spi_clock_hz };
  for (size_t i = 0; i < (sizeof(clocks) / sizeof(clocks[0])); ++i) {
    const int hz = clocks[i];
    if (hz <= 0) continue;
    if (i > 0 && sameClock_(hz, clocks[i - 1])) continue;
    if (!sameClock_(hz, _activeClockHz) && !addDevice(hz)) continue;
    delay(2);
    _auxSelected = false;
    if (!selectAux()) continue;
    got = readRawBurstInternal_(dst, n, 1, measured_fs_hz);
    if (got == n && !looksInactive_(dst, n, 2, 2)) return got;
  }

  return got;
}

size_t ADS8684::readRawBurstAveraged(uint16_t* dst, size_t n, uint8_t oversample, float* measured_fs_hz) {
  if (!_auxSelected && !selectAux()) return 0;

  size_t got = readRawBurstInternal_(dst, n, oversample, measured_fs_hz);
  if (got == n && !looksInactive_(dst, n, 2, 2)) return got;

  const int clocks[] = { _activeClockHz, ADS_SPI_FALLBACK_HZ, ADS_SPI_RECOVERY_HZ, _cfg.spi_clock_hz };
  for (size_t i = 0; i < (sizeof(clocks) / sizeof(clocks[0])); ++i) {
    const int hz = clocks[i];
    if (hz <= 0) continue;
    if (i > 0 && sameClock_(hz, clocks[i - 1])) continue;
    if (!sameClock_(hz, _activeClockHz) && !addDevice(hz)) continue;
    delay(2);
    _auxSelected = false;
    if (!selectAux()) continue;
    got = readRawBurstInternal_(dst, n, oversample, measured_fs_hz);
    if (got == n && !looksInactive_(dst, n, 2, 2)) return got;
  }

  return got;
}

float ADS8684::rawToVolts(uint16_t code) const {
  return (float(code) * _vref) / 65535.0f;
}
