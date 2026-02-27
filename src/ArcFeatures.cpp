#include "ArcFeatures.h"
#include <math.h>
#include <arduinoFFT.h>

static inline float clampf(float x, float lo, float hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}

static inline float codeToCurrentA(uint16_t code, const CurrentCalib& cal) {
  const float v_aux = (float(code) * ADS_VREF_V) / 65535.0f; // 0..Vref
  const float v_sensor = v_aux / cal.dividerRatio;          // undo divider
  return (v_sensor - cal.offsetV) / cal.voltsPerAmp;
}

static inline float median3(float a, float b, float c) {
  if (a > b) { float t = a; a = b; b = t; }
  if (b > c) { float t = b; b = c; c = t; }
  if (a > b) { float t = a; a = b; b = t; }
  return b;
}

bool ArcFeatures::compute(const uint16_t* raw, size_t n, float fs_hz,
                          const CurrentCalib& cal, float mainsHz,
                          ArcFeatOut& out) {
  if (!raw || n != N_SAMP) return false;

  out.fs_hz = fs_hz;

  static float vReal[N_SAMP];
  static float vImag[N_SAMP];

  // --------------------
  // 1) Convert + mean
  // --------------------
  double mean = 0.0;
  int glitch = 0;
  for (size_t i = 0; i < n; i++) {
    const uint16_t c = raw[i];
    if (c < 32 || c > 65503) glitch++;
    if (glitch > 8) return false;

    const float a = codeToCurrentA(c, cal);
    vReal[i] = a;     // temp store raw current
    mean += a;
  }
  mean /= (double)n;

  // --------------------
  // 2) RMS of centered signal
  // --------------------
  double acc = 0.0;
  for (size_t i = 0; i < n; i++) {
    const float s = vReal[i] - (float)mean;
    vReal[i] = s;
    vImag[i] = 0.0f;
    acc += (double)s * (double)s;
  }
  const float irms_raw = (float)sqrt(acc / (double)n);

  // Median-of-3 across frames removes single-frame spikes
  static bool  histInit = false;
  static float irms_h0 = 0.0f, irms_h1 = 0.0f, irms_h2 = 0.0f;
  if (!histInit) {
    histInit = true;
    irms_h0 = irms_h1 = irms_h2 = irms_raw;
  } else {
    irms_h0 = irms_h1;
    irms_h1 = irms_h2;
    irms_h2 = irms_raw;
  }
  const float irms_med = median3(irms_h0, irms_h1, irms_h2);

  // --------------------
  // 2a) Noise floor tracking (FIX)
  // --------------------
  // Floor must NOT chase small loads upward.
  // We use a slow-leaking minimum tracker.

  static bool  floorInit = false;
  static float idleFloor = 0.0f;

  // Tunables (safe defaults)
  static constexpr float IDLE_FLOOR_INIT_MAX_A = 0.03f;      // cap initial floor
  static constexpr float IDLE_FLOOR_MAX_A      = 0.10f;      // hard cap
  static constexpr float IDLE_FLOOR_LEAK_A_PER_SEC = 0.0008f; // 0.8 mA/s upward leak
  static constexpr float IDLE_GATE_MARGIN_A    = 0.006f;     // 6 mA above floor
  static constexpr uint32_t NONIDLE_HOLD_MS    = 600;        // avoid dropouts

  // Estimate feature frame rate
  const float frameHz = (fs_hz > 1000.0f) ? (fs_hz / (float)n) : 30.0f;
  const float leakPerFrame = IDLE_FLOOR_LEAK_A_PER_SEC / frameHz;

  if (!floorInit) {
    floorInit = true;
    idleFloor = fminf(irms_med, IDLE_FLOOR_INIT_MAX_A);
  } else {
    if (irms_med < idleFloor) {
      idleFloor = irms_med; // follow down immediately
    } else {
      idleFloor = fminf(idleFloor + leakPerFrame, IDLE_FLOOR_MAX_A);
    }
  }

  // Clean current (additive subtract is safer for small loads)
  const float irms_clean = fmaxf(0.0f, irms_med - idleFloor);

  // --------------------
  // 2b) Idle/non-idle gate (hysteresis + hold)
  // --------------------
  static uint8_t  aboveGate = 0;
  static uint32_t holdUntil = 0;

  const float gateOn  = fmaxf(IDLE_IRMS_A, idleFloor + IDLE_GATE_MARGIN_A);
  const float gateOff = gateOn * 0.60f; // hysteresis
  const uint32_t nowMs = millis();

  if (irms_clean >= gateOn) {
    if (aboveGate < 3) aboveGate++;
    if (aboveGate >= 2) holdUntil = nowMs + NONIDLE_HOLD_MS;
  } else {
    if (aboveGate > 0) aboveGate--;
  }

  const bool heldNonIdle = (nowMs < holdUntil);

  if (!heldNonIdle && irms_clean < gateOff) {
    out.irms_a = 0.0f;
    out.thd_pct = 0.0f;
    out.entropy = 0.0f;
    out.zcv_ms  = 0.0f;
    return true;
  }

  out.irms_a = irms_clean;

  // --------------------
  // 3) ZCV
  // --------------------
  int crossings[80];
  int cCount = 0;
  const float hys = fmaxf(ZC_HYS_MIN_A, ZC_HYS_FRAC * irms_med);

  int state = 0;
  if (vReal[0] >  hys) state =  1;
  if (vReal[0] < -hys) state = -1;

  for (size_t i = 1; i < n && cCount < 80; i++) {
    const float s = vReal[i];
    if (state <= 0 && s >  hys) { crossings[cCount++] = (int)i; state =  1; }
    else if (state >= 0 && s < -hys) { crossings[cCount++] = (int)i; state = -1; }
  }

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

  // --------------------
  // 4) FFT + THD + Entropy
  // --------------------
  ArduinoFFT<float> FFT(vReal, vImag, n, fs_hz);
  FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  const float binHz = fs_hz / (float)n;
  int kNom = (int)lround(mainsHz / binHz);
  if (kNom < 1) kNom = 1;
  if (kNom > (int)(n/2 - 2)) kNom = (int)(n/2 - 2);

  int k1 = kNom;
  float fund = 0.0f;
  for (int dk = -2; dk <= 2; dk++) {
    int k = kNom + dk;
    if (k >= 1 && k < (int)(n/2)) {
      if (vReal[k] > fund) { fund = vReal[k]; k1 = k; }
    }
  }

  const int b0 = max(1, (int)ceil(500.0f / binHz));
  const int b1 = min((int)(n/2 - 1), (int)floor(3000.0f / binHz));
  double noiseSum = 0.0;
  int noiseN = 0;

  for (int b = b0; b <= b1; b++) {
    bool skip = false;
    for (int h = 1; h <= 10; h++) {
      int kh = h * k1;
      if (kh >= (int)(n/2)) break;
      if (abs(b - kh) <= 2) { skip = true; break; }
    }
    if (!skip) { noiseSum += (double)vReal[b]; noiseN++; }
  }

  const float noiseMag = (noiseN > 0) ? (float)(noiseSum / (double)noiseN) : 0.0f;

  if (fund < FUND_MAG_MIN || (noiseMag > 0.0f && fund < (FUND_SNR_MIN * noiseMag))) {
    out.thd_pct = 0.0f;
    out.entropy = 0.0f;
    out.zcv_ms  = 0.0f;
    return true;
  }

  double harm2 = 0.0;
  for (int h = 2; h <= 10; h++) {
    int khNom = h * k1;
    if (khNom >= (int)(n/2)) break;
    float hm = 0.0f;
    for (int dk = -1; dk <= 1; dk++) {
      int kh = khNom + dk;
      if (kh >= 1 && kh < (int)(n/2)) hm = max(hm, vReal[kh]);
    }
    harm2 += (double)hm * (double)hm;
  }
  out.thd_pct = (float)(sqrt(harm2) / (double)fund * 100.0);

  const int maxBin = (int)min((double)(n/2), floor((double)ENTROPY_MAX_HZ / (double)binHz));
  double psum = 0.0;

  for (int b = 1; b < maxBin; b++) {
    const double pw = (double)vReal[b] * (double)vReal[b];
    psum += pw;
    vImag[b] = (float)pw;
  }

  if (psum < 1e-18 || maxBin <= 2) {
    out.entropy = 0.0f;
  } else {
    double H = 0.0;
    for (int b = 1; b < maxBin; b++) {
      double p = (double)vImag[b] / psum;
      if (p > 1e-18) H += -p * log(p);
    }
    H /= log((double)(maxBin - 1));
    out.entropy = clampf((float)H, 0.0f, 1.0f);
  }

  return true;
}