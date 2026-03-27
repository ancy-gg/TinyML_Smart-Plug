#include "CurrentBackendADS8684.h"
#include "SmartPlugConfig.h"

namespace {
static inline uint16_t median3u16(uint16_t a, uint16_t b, uint16_t c) {
  if (a > b) { const uint16_t t = a; a = b; b = t; }
  if (b > c) { const uint16_t t = b; b = c; c = t; }
  if (a > b) { const uint16_t t = a; a = b; b = t; }
  return b;
}

static inline uint16_t meanNu16(const uint16_t* src, size_t n) {
  uint32_t acc = 0;
  for (size_t i = 0; i < n; ++i) acc += (uint32_t)src[i];
  return (uint16_t)((acc + (uint32_t)(n / 2U)) / (uint32_t)n);
}
}

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
  if (!dst || n == 0) {
    if (measuredFsHz) *measuredFsHz = 0.0f;
    return 0;
  }

  const size_t decim = (ADS_OUTPUT_DECIMATE < 1) ? 1 : (size_t)ADS_OUTPUT_DECIMATE;
  if (decim <= 1) {
    return _adc.readRawBurst(dst, n, measuredFsHz);
  }

  static uint16_t rawBuf[ADS_BURST_OUTPUT_CHUNK_SAMPLES * ADS_OUTPUT_DECIMATE];
  size_t outWritten = 0;
  float lastRawFsHz = 0.0f;

  while (outWritten < n) {
    const size_t outChunk = ((n - outWritten) > ADS_BURST_OUTPUT_CHUNK_SAMPLES)
      ? ADS_BURST_OUTPUT_CHUNK_SAMPLES
      : (n - outWritten);
    const size_t rawNeed = outChunk * decim;

    float rawFsHz = 0.0f;
    const size_t gotRaw = _adc.readRawBurst(rawBuf, rawNeed, &rawFsHz);
    const size_t gotOut = gotRaw / decim;
    if (gotOut == 0) break;

    for (size_t i = 0; i < gotOut; ++i) {
      const uint16_t* s = &rawBuf[i * decim];
      uint16_t y = 0;
      if (ADS_OUTPUT_USE_MEDIAN3 && decim == 3) {
        y = median3u16(s[0], s[1], s[2]);
      } else {
        y = meanNu16(s, decim);
      }
      dst[outWritten + i] = y;
    }

    outWritten += gotOut;
    lastRawFsHz = rawFsHz;

    if (gotRaw != rawNeed) break;
  }

  if (measuredFsHz) {
    *measuredFsHz = (lastRawFsHz > 0.0f) ? (lastRawFsHz / (float)decim) : 0.0f;
  }
  return outWritten;
}
