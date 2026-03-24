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
#if defined(ADS_CAPTURE_OVERSAMPLE)
  const uint8_t os = (ADS_CAPTURE_OVERSAMPLE < 1) ? 1 : ADS_CAPTURE_OVERSAMPLE;
#else
  const uint8_t os = 1;
#endif

  size_t got = (os > 1)
    ? _adc.readRawBurstAveraged(dst, n, os, measuredFsHz)
    : _adc.readRawBurst(dst, n, measuredFsHz);

  if (got == n) return got;

  // fallback path if averaged capture fails
  return _adc.readRawBurst(dst, n, measuredFsHz);
}