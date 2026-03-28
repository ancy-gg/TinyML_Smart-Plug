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
  // Keep AUX-based low-level ADS path intact.
  // Use the raw burst path only; cleanup happens later in DSP.
  size_t got = _adc.readRawBurst(dst, n, measuredFsHz);
  if (got == n) return got;

  // Retry once using the same raw path. The ADS driver itself already
  // handles primary/fallback SPI clock selection during begin().
  return _adc.readRawBurst(dst, n, measuredFsHz);
}
