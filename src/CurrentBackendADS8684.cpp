#include "CurrentBackendADS8684.h"
#include "SmartPlugConfig.h"

CurrentBackendADS8684::CurrentBackendADS8684()
: _adc(ADS8684::Config{
    .host = SPI2_HOST,
    .pin_cs = PIN_ADC_CS,
    .pin_sck = PIN_ADC_SCK,
    .pin_miso = PIN_ADC_MISO,
    .pin_mosi = PIN_ADC_MOSI,
    .spi_clock_hz = ADS_SPI_HZ
  }) {}

bool CurrentBackendADS8684::begin() {
  return _adc.begin();
}

size_t CurrentBackendADS8684::capture(uint16_t* dst, size_t n, float* measuredFsHz) {
  return _adc.readRawBurstAveraged(dst, n, ADS_CAPTURE_OVERSAMPLE, measuredFsHz);
}
