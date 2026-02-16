#include "CurrentSensor.h"

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