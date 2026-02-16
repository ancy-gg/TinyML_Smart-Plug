#include "ArcFeatures.h"
#include <math.h>
#include <arduinoFFT.h>

static inline float clampf(float x, float lo, float hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}

static inline float codeToCurrentA(uint16_t code, const CurrentCalib& cal) {
  // ADS8684 AUX volts 0..4.096
  const float v_aux = (float(code) * 4.096f) / 65535.0f;
  // undo divider to recover sensor output
  const float v_sensor = v_aux / cal.dividerRatio;
  // sensor model
  return (v_sensor - cal.offsetV) / cal.voltsPerAmp;
}

bool ArcFeatures::compute(const uint16_t* raw, size_t n, float fs_hz,
                          const CurrentCalib& cal, float mainsHz,
                          ArcFeatOut& out) {
  if (!raw || n != 4096) return false;

  out.fs_hz = fs_hz;

  // static buffers to avoid stack + heap churn
  static float iSig[4096];
  static float vReal[4096];
  static float vImag[4096];

  // 1) Convert once + compute mean
  double mean = 0.0;
  for (size_t i = 0; i < n; i++) {
    const float a = codeToCurrentA(raw[i], cal);
    iSig[i] = a;
    mean += a;
  }
  mean /= (double)n;

  // 2) RMS + zero crossing variation
  double acc = 0.0;

  int crossings[80];
  int cCount = 0;

  float prev = iSig[0] - (float)mean;
  for (size_t i = 1; i < n; i++) {
    const float cur = iSig[i] - (float)mean;
    const double d = (double)cur;
    acc += d * d;

    if (cCount < 80) {
      if ((prev <= 0 && cur > 0) || (prev >= 0 && cur < 0)) {
        crossings[cCount++] = (int)i;
      }
    }
    prev = cur;
  }
  out.irms_a = (float)sqrt(acc / (double)n);

  if (cCount >= 6) {
    double sum = 0.0, sum2 = 0.0;
    int m = 0;
    for (int k = 1; k < cCount; k++) {
      const int dt = crossings[k] - crossings[k - 1];
      const double dtMs = (double(dt) / double(fs_hz)) * 1000.0;
      sum += dtMs;
      sum2 += dtMs * dtMs;
      m++;
    }
    const double mu = sum / (double)m;
    const double var = (sum2 / (double)m) - (mu * mu);
    out.zcv_ms = (float)sqrt(var > 0 ? var : 0);
  } else {
    out.zcv_ms = 0.0f;
  }

  // 3) Prepare FFT buffers
  for (size_t i = 0; i < n; i++) {
    vReal[i] = iSig[i] - (float)mean;
    vImag[i] = 0.0f;
  }

  ArduinoFFT<float> FFT(vReal, vImag, n, fs_hz);
  FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  const float binHz = fs_hz / (float)n;
  int k1 = (int)lround(mainsHz / binHz);
  if (k1 < 1) k1 = 1;
  if (k1 > (int)(n/2 - 2)) k1 = (int)(n/2 - 2);

  float fund = vReal[k1];
  if (fund < 1e-9f) fund = 1e-9f;

  // THD harmonics 2..10
  double harm2 = 0.0;
  for (int h = 2; h <= 10; h++) {
    int kh = h * k1;
    if (kh >= (int)(n/2)) break;
    const double a = (double)vReal[kh];
    harm2 += a * a;
  }
  out.thd_pct = (float)(sqrt(harm2) / (double)fund * 100.0);

  // Spectral entropy up to 50kHz (or nyquist)
  const int maxBin = (int)min((double)(n/2), floor(50000.0 / binHz));
  double psum = 0.0;

  for (int b = 1; b < maxBin; b++) {
    const double pw = (double)vReal[b] * (double)vReal[b];
    psum += pw;
    vImag[b] = (float)pw; // reuse storage
  }

  if (psum < 1e-18 || maxBin <= 2) {
    out.entropy = 0.0f;
  } else {
    double H = 0.0;
    for (int b = 1; b < maxBin; b++) {
      const double p = (double)vImag[b] / psum;
      if (p > 1e-18) H += -p * log(p);
    }
    H /= log((double)(maxBin - 1));
    out.entropy = clampf((float)H, 0.0f, 1.0f);
  }

  return true;
}