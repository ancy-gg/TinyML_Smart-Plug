#include "CurrentSensor.h"
#include <math.h>
#include <esp_timer.h>

#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_MCP3204
  #include <driver/spi_master.h>
#endif

static inline uint16_t scale12to16(uint16_t x12) {
  x12 &= 0x0FFF;
  return uint16_t((x12 << 4) | (x12 >> 8));
}

#if CURRENT_CAPTURE_BACKEND == CUR_BACKEND_ADS8684

CurrentSensor::CurrentSensor()
: _adc(ADS8684::Config{
    .host = SPI2_HOST,
    .pin_cs = PIN_ADC_CS,
    .pin_sck = PIN_ADC_SCK,
    .pin_miso = PIN_ADC_MISO,
    .pin_mosi = PIN_ADC_MOSI,
    .spi_clock_hz = ADS_SPI_HZ
  }) {}

bool CurrentSensor::begin() {
  return _adc.begin();
}

size_t CurrentSensor::capture(uint16_t* dst, size_t n, float* measuredFsHz) {
  return _adc.readRawBurst(dst, n, measuredFsHz);
}

#elif CURRENT_CAPTURE_BACKEND == CUR_BACKEND_MCP3204

CurrentSensor::CurrentSensor() {}

bool CurrentSensor::initMcp3204() {
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
  devcfg.clock_speed_hz = MCP3204_SPI_HZ;
  devcfg.spics_io_num = PIN_ADC_CS;
  devcfg.queue_size = 1;
  devcfg.flags = SPI_DEVICE_NO_DUMMY;

  e = spi_bus_add_device(SPI2_HOST, &devcfg, &_dev);
  if (e == ESP_ERR_INVALID_STATE && _dev != nullptr) return true;
  return (e == ESP_OK);
}

bool CurrentSensor::begin() {
  if (!initMcp3204()) return false;

  // Settle / discard initial samples.
  spi_transaction_t t = {};
  t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
  t.length = 24;
  t.tx_data[0] = uint8_t(0x06 | ((MCP3204_CHANNEL & 0x04) >> 2));
  t.tx_data[1] = uint8_t((MCP3204_CHANNEL & 0x03) << 6);
  t.tx_data[2] = 0x00;

  for (int i = 0; i < 32; ++i) {
    (void)spi_device_polling_transmit(_dev, &t);
  }
  return true;
}

size_t CurrentSensor::capture(uint16_t* dst, size_t n, float* measuredFsHz) {
  if (!dst || n == 0 || !_dev) return 0;

  spi_transaction_t t = {};
  t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
  t.length = 24;
  t.tx_data[0] = uint8_t(0x06 | ((MCP3204_CHANNEL & 0x04) >> 2));
  t.tx_data[1] = uint8_t((MCP3204_CHANNEL & 0x03) << 6);
  t.tx_data[2] = 0x00;

  const int64_t t0 = esp_timer_get_time();
  size_t count = 0;

  (void)spi_device_acquire_bus(_dev, portMAX_DELAY);
  while (count < n) {
    esp_err_t e = spi_device_polling_transmit(_dev, &t);
    if (e != ESP_OK) break;

    const uint16_t raw12 = uint16_t(((t.rx_data[1] & 0x0F) << 8) | t.rx_data[2]);
    dst[count++] = scale12to16(raw12);
  }
  spi_device_release_bus(_dev);

  const int64_t t1 = esp_timer_get_time();
  if (measuredFsHz && t1 > t0) {
    *measuredFsHz = (count * 1000000.0f) / float(t1 - t0);
  }
  return count;
}

#else

CurrentSensor::CurrentSensor() {}

bool CurrentSensor::begin() {
  pinMode(PIN_CUR_ADC_ANALOG, INPUT);
  analogReadResolution(12);

  #if defined(ADC_11db)
    analogSetPinAttenuation(PIN_CUR_ADC_ANALOG, ADC_11db);
  #elif defined(ADC_ATTEN_DB_11)
    analogSetPinAttenuation(PIN_CUR_ADC_ANALOG, ADC_ATTEN_DB_11);
  #endif

  for (int i = 0; i < 32; ++i) {
    (void)analogRead(PIN_CUR_ADC_ANALOG);
  }
  _started = true;
  return true;
}

size_t CurrentSensor::capture(uint16_t* dst, size_t n, float* measuredFsHz) {
  if (!dst || n == 0 || !_started) return 0;

  int overs = ESP32_ADC_OVERSAMPLE;
  if (overs < 1) overs = 1;

  const float subRate = FS_TARGET_HZ * float(overs);
  const int32_t subPeriodUs = (subRate > 1.0f) ? (int32_t)lroundf(1000000.0f / subRate) : 0;

  const int64_t t0 = esp_timer_get_time();
  int64_t nextT = t0;
  size_t count = 0;

  while (count < n) {
    uint32_t acc = 0;

    for (int o = 0; o < overs; ++o) {
      if (subPeriodUs > 0) {
        const int64_t now = esp_timer_get_time();
        if (nextT > now) {
          while (esp_timer_get_time() < nextT) { /* busy wait */ }
        }
        nextT += subPeriodUs;
      }

      int raw12 = analogRead(PIN_CUR_ADC_ANALOG);
      if (raw12 < 0) raw12 = 0;
      if (raw12 > 4095) raw12 = 4095;
      acc += (uint32_t)raw12;
    }

    const uint16_t avg12 = uint16_t((acc + (uint32_t)(overs / 2)) / (uint32_t)overs);
    dst[count++] = scale12to16(avg12);
  }

  const int64_t t1 = esp_timer_get_time();
  if (measuredFsHz && t1 > t0) {
    *measuredFsHz = (count * 1000000.0f) / float(t1 - t0);
  }
  return count;
}

#endif