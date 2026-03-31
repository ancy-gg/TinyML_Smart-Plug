#include "CurrentSensor.h"
#include <esp_timer.h>
#include <string.h>

static constexpr uint8_t kMcpMedianSamples =
    (MCP3204_MEDIAN_SAMPLES < 1) ? 1 :
    ((MCP3204_MEDIAN_SAMPLES > 3) ? 3 : MCP3204_MEDIAN_SAMPLES);

static inline uint16_t scale12to16(uint16_t x12) {
  x12 &= 0x0FFF;
  return uint16_t((x12 << 4) | (x12 >> 8));
}

static inline uint16_t median3u16(uint16_t a, uint16_t b, uint16_t c) {
  if (a > b) { const uint16_t t = a; a = b; b = t; }
  if (b > c) { const uint16_t t = b; b = c; c = t; }
  if (a > b) { const uint16_t t = a; a = b; b = t; }
  return b;
}

namespace {
  static inline void mcpBuildTxn(spi_transaction_t& t) {
    memset(&t, 0, sizeof(t));
    t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
    t.length = 24;
    t.tx_data[0] = uint8_t(0x06 | ((MCP3204_CHANNEL & 0x04) >> 2));
    t.tx_data[1] = uint8_t((MCP3204_CHANNEL & 0x03) << 6);
    t.tx_data[2] = 0x00;
  }

  static inline uint16_t mcpExtract12(const spi_transaction_t& t) {
    return uint16_t(((t.rx_data[1] & 0x0F) << 8) | t.rx_data[2]);
  }
}

bool CurrentSensor::initMcp3204_() {
  pinMode(PIN_ADC_CS, OUTPUT);
  digitalWrite(PIN_ADC_CS, HIGH);
  delayMicroseconds(50);

  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = PIN_ADC_MOSI;
  buscfg.miso_io_num = PIN_ADC_MISO;
  buscfg.sclk_io_num = PIN_ADC_SCK;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = 4;

  esp_err_t e = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
  if (e != ESP_OK && e != ESP_ERR_INVALID_STATE) return false;

  spi_device_interface_config_t devcfg = {};
  devcfg.mode = 0;
  devcfg.clock_speed_hz = (int)MCP3204_SPI_HZ;
  devcfg.spics_io_num = PIN_ADC_CS;
  devcfg.queue_size = 1;
  devcfg.flags = SPI_DEVICE_NO_DUMMY;

  e = spi_bus_add_device(SPI2_HOST, &devcfg, &_dev);
  if (e == ESP_ERR_INVALID_STATE && _dev != nullptr) return true;
  return (e == ESP_OK);
}

bool CurrentSensor::begin() {
  if (!initMcp3204_()) return false;

  spi_transaction_t t;
  mcpBuildTxn(t);

  (void)spi_device_acquire_bus(_dev, portMAX_DELAY);
  digitalWrite(PIN_ADC_CS, HIGH);
  delayMicroseconds(100);
  for (uint16_t i = 0; i < MCP3204_STARTUP_FLUSH; ++i) {
    (void)spi_device_polling_transmit(_dev, &t);
  }
  spi_device_release_bus(_dev);
  return true;
}

size_t CurrentSensor::capture(uint16_t* dst, size_t n, float* measuredFsHz) {
  if (measuredFsHz) *measuredFsHz = 0.0f;
  if (!dst || n == 0 || !_dev) return 0;

  spi_transaction_t t;
  mcpBuildTxn(t);

  static uint8_t warmupBurstsRemaining = MCP3204_WARMUP_BURSTS;
  const uint8_t overs = (MCP3204_OVERSAMPLE < 1) ? 1 : MCP3204_OVERSAMPLE;
  const int64_t t0 = esp_timer_get_time();
  size_t count = 0;

  (void)spi_device_acquire_bus(_dev, portMAX_DELAY);

  if (warmupBurstsRemaining > 0) {
    digitalWrite(PIN_ADC_CS, HIGH);
    delayMicroseconds(40);
    for (int i = 0; i < 128; ++i) (void)spi_device_polling_transmit(_dev, &t);
    warmupBurstsRemaining--;
  }

  for (uint8_t i = 0; i < MCP3204_BURST_FLUSH; ++i) {
    (void)spi_device_polling_transmit(_dev, &t);
  }

  while (count < n) {
    uint32_t acc = 0;
    uint8_t good = 0;

    for (uint8_t o = 0; o < overs; ++o) {
      uint16_t s0 = 0, s1 = 0, s2 = 0;
      uint8_t localGood = 0;

      if (spi_device_polling_transmit(_dev, &t) == ESP_OK) { s0 = mcpExtract12(t); localGood++; }
      if (kMcpMedianSamples >= 2) {
        if (spi_device_polling_transmit(_dev, &t) == ESP_OK) { s1 = mcpExtract12(t); localGood++; }
      } else s1 = s0;
      if (kMcpMedianSamples >= 3) {
        if (spi_device_polling_transmit(_dev, &t) == ESP_OK) { s2 = mcpExtract12(t); localGood++; }
      } else s2 = s1;

      if (localGood == 0) break;

      uint16_t sample12 = s0;
      if (localGood == 2) sample12 = uint16_t((uint32_t(s0) + uint32_t(s1) + 1U) / 2U);
      else if (localGood >= 3) sample12 = median3u16(s0, s1, s2);

      acc += (uint32_t)sample12;
      good++;
    }

    if (good == 0) break;
    const uint16_t avg12 = uint16_t((acc + (uint32_t)(good / 2)) / (uint32_t)good);
    dst[count++] = scale12to16(avg12);
  }

  spi_device_release_bus(_dev);

  const int64_t t1 = esp_timer_get_time();
  if (measuredFsHz && t1 > t0 && count > 0) {
    *measuredFsHz = (count * 1000000.0f) / float(t1 - t0);
  }
  return count;
}
