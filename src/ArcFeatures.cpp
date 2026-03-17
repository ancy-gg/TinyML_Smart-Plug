#include "ArcFeatures.h"
#include <math.h>
#include <string.h>

#include "esp_dsp.h"
#include "dsps_fft2r.h"
#include "dsps_wind_hann.h"
#include "dsp_common.h"
#include "esp_err.h"

#ifndef CONFIG_DSP_MAX_FFT_SIZE
#define CONFIG_DSP_MAX_FFT_SIZE 4096
#endif

static inline float clampf(float x, float lo, float hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}

static inline float codeToCurrentA(uint16_t code, const CurrentCalib& cal) {
  const float v_aux = (float(code) * ADS_VREF_V) / 65535.0f;
  const float v_sensor = v_aux / cal.dividerRatio;
  const float amps_uncal = ((v_sensor - cal.offsetV) / cal.voltsPerAmp) * cal.ampsScale;
  return eval_cubic_horner(amps_uncal, cal.cubic3, cal.cubic2, cal.cubic1, cal.cubic0);
}

static inline float median3f(float a, float b, float c) {
  if (a > b) { float t = a; a = b; b = t; }
  if (b > c) { float t = b; b = c; c = t; }
  if (a > b) { float t = a; a = b; b = t; }
  return b;
}

static inline int clampi(int x, int lo, int hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}

static float computeEntropyFromBands(const float* e, int n) {
  double sum = 0.0;
  for (int i = 0; i < n; ++i) sum += e[i];
  if (sum <= 1e-18) return 0.0f;

  double H = 0.0;
  for (int i = 0; i < n; ++i) {
    const double p = e[i] / sum;
    if (p > 1e-18) H += -p * log(p);
  }
  return clampf((float)(H / log((double)n)), 0.0f, 1.0f);
}

bool ArcFeatures::compute(const uint16_t* raw, size_t n, float fs_hz,
                          const CurrentCalib& cal, float mainsHz,
                          ArcFeatOut& out) {
  out = ArcFeatOut{};
  out.fs_hz = fs_hz;

  if (!raw || n != N_SAMP || fs_hz < 1000.0f) return false;
  if (!dsp_is_power_of_two((int)n)) return false;
  if ((int)n > CONFIG_DSP_MAX_FFT_SIZE) return false;

  static float sig[N_SAMP];         // full-band centered
  static float sigClean[N_SAMP];    // gentle LP for RMS / cycle features
  static float sigBase[N_SAMP];     // fundamental-ish baseline
  static float resid[N_SAMP];       // impulsive residual
  static float fft_cf[2 * N_SAMP];  // complex interleaved
  static float win[N_SAMP];
  static bool dspReady = false;
  static bool winReady = false;

  if (!dspReady) {
    if (dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE) != ESP_OK) return false;
    dspReady = true;
  }
  if (!winReady) {
    dsps_wind_hann_f32(win, (int)n);
    winReady = true;
  }

  double mean = 0.0;
  int sat = 0;
  int changes = 0;
  uint16_t prev = raw[0];
  for (size_t i = 0; i < n; ++i) {
    const uint16_t c = raw[i];
    if (c < 16 || c > 65519) sat++;
    if (i && c != prev) changes++;
    prev = c;

    const float a = codeToCurrentA(c, cal);
    sig[i] = a;
    mean += a;
  }
  mean /= (double)n;

  if (sat > 64) return false;
  if (changes < 16) return false;

  for (size_t i = 0; i < n; ++i) sig[i] -= (float)mean;

  const float dt = 1.0f / fs_hz;
  const float rcClean = 1.0f / (6.2831853f * CURRENT_LPF_HZ);
  const float rcBase  = 1.0f / (6.2831853f * CURRENT_BASE_LPF_HZ);
  const float aClean = dt / (rcClean + dt);
  const float aBase  = dt / (rcBase + dt);

  float yClean = sig[0];
  float yBase = sig[0];
  double accIrms = 0.0;
  double accResidSq = 0.0;
  for (size_t i = 0; i < n; ++i) {
    yClean += aClean * (sig[i] - yClean);
    yBase  += aBase  * (sig[i] - yBase);
    sigClean[i] = yClean;
    sigBase[i]  = yBase;
    resid[i] = yClean - yBase;
    accIrms += (double)yClean * (double)yClean;
    accResidSq += (double)resid[i] * (double)resid[i];
  }

  const float irmsRaw = sqrtf((float)(accIrms / (double)n));

  static bool gateInited = false;
  static float r0 = 0.0f, r1 = 0.0f, r2 = 0.0f;
  static bool irmsGateOn = false;
  if (!gateInited) {
    gateInited = true;
    r0 = r1 = r2 = irmsRaw;
  } else {
    r0 = r1;
    r1 = r2;
    r2 = irmsRaw;
  }
  const float irmsMed = median3f(r0, r1, r2);
  if (!irmsGateOn) {
    if (irmsMed >= IRMS_GATE_ON_A) irmsGateOn = true;
  } else if (irmsMed <= IRMS_GATE_OFF_A) {
    irmsGateOn = false;
  }

  out.irms_a = irmsGateOn ? irmsMed : 0.0f;
  out.current_valid = true;

  if (irmsRaw < FEATURE_MIN_IRMS_A) {
    out.feat_valid = true;
    return true;
  }

  // --- zero crossings and cycle boundaries on smoothed current ---
  float crossAll[128];
  int crossAllN = 0;
  float crossPos[64];
  int crossPosN = 0;
  const float zcHys = fmaxf(ZC_HYS_MIN_A, ZC_HYS_FRAC * fmaxf(irmsRaw, 0.1f));

  for (size_t i = 1; i < n && crossAllN < 128; ++i) {
    const float a = sigClean[i - 1];
    const float b = sigClean[i];

    if (a <= -zcHys && b >= zcHys) {
      const float denom = (b - a);
      const float frac = (fabsf(denom) > 1e-9f) ? ((0.0f - a) / denom) : 0.5f;
      const float idx = (float)(i - 1) + clampf(frac, 0.0f, 1.0f);
      crossAll[crossAllN++] = idx;
      if (crossPosN < 64) crossPos[crossPosN++] = idx;
    } else if (a >= zcHys && b <= -zcHys) {
      const float denom = (b - a);
      const float frac = (fabsf(denom) > 1e-9f) ? ((0.0f - a) / denom) : 0.5f;
      const float idx = (float)(i - 1) + clampf(frac, 0.0f, 1.0f);
      crossAll[crossAllN++] = idx;
    }
  }

  if (crossAllN >= 4) {
    double mu = 0.0;
    for (int i = 1; i < crossAllN; ++i) {
      mu += (double)(crossAll[i] - crossAll[i - 1]);
    }
    mu /= (double)(crossAllN - 1);

    double vv = 0.0;
    for (int i = 1; i < crossAllN; ++i) {
      const double d = (double)(crossAll[i] - crossAll[i - 1]) - mu;
      vv += d * d;
    }
    vv /= (double)(crossAllN - 1);
    out.zcv = (float)(sqrt(vv) * (1000.0 / fs_hz));
  }

  if (crossAllN >= 3) {
    const float dwellThr = fmaxf(ZC_DWELL_THR_MIN_A, ZC_DWELL_THR_FRAC * irmsRaw);
    const float avgHalfSamp = (crossAllN >= 4)
      ? (float)((crossAll[crossAllN - 1] - crossAll[0]) / (float)(crossAllN - 1))
      : (fs_hz / (mainsHz * 2.0f));
    const int halfWin = clampi((int)lroundf(avgHalfSamp * 0.12f), 3, 100);

    int total = 0;
    int dwell = 0;
    for (int ci = 0; ci < crossAllN; ++ci) {
      const int c = (int)lroundf(crossAll[ci]);
      const int a = clampi(c - halfWin, 0, (int)n - 1);
      const int b = clampi(c + halfWin, 0, (int)n - 1);
      for (int i = a; i <= b; ++i) {
        total++;
        if (fabsf(sigClean[i]) <= dwellThr) dwell++;
      }
    }
    out.zc_dwell_ratio = (total > 0) ? ((float)dwell / (float)total) : 0.0f;
  }

  const int cycleCount = crossPosN - 1;
  if (cycleCount <= 0) {
    out.feat_valid = true;
    return true;
  }

  float cyclePeaks[24];
  int peakN = 0;
  float pulsePerCycleAcc = 0.0f;
  int pulseCycles = 0;
  float nmseAcc = 0.0f;
  int nmsePairs = 0;
  static constexpr int RSZ = 96;
  float prevCycle[RSZ];
  bool havePrevCycle = false;

  const float residRms = sqrtf((float)(accResidSq / (double)n));
  const float pulseThr = fmaxf(PULSE_THRESH_MIN_A, PULSE_THRESH_RMS_MUL * residRms);
  const int pulseMinW = clampi((int)lroundf((PULSE_MIN_WIDTH_US * 1e-6f) * fs_hz), 1, 32);
  const int pulseMaxW = clampi((int)lroundf((PULSE_MAX_WIDTH_US * 1e-6f) * fs_hz), pulseMinW, 512);

  for (int c = 0; c < cycleCount && c < 24; ++c) {
    const int a = clampi((int)floorf(crossPos[c]), 0, (int)n - 2);
    const int b = clampi((int)floorf(crossPos[c + 1]), a + 2, (int)n - 1);
    const int len = b - a;
    if (len < 16) continue;

    float peak = 0.0f;
    int pulseCount = 0;
    bool inPulse = false;
    int pulseStart = a;
    for (int i = a; i < b; ++i) {
      const float av = fabsf(sigClean[i]);
      if (av > peak) peak = av;

      const bool over = fabsf(resid[i]) >= pulseThr;
      if (over && !inPulse) {
        inPulse = true;
        pulseStart = i;
      } else if (!over && inPulse) {
        const int w = i - pulseStart;
        if (w >= pulseMinW && w <= pulseMaxW) pulseCount++;
        inPulse = false;
      }
    }
    if (inPulse) {
      const int w = b - pulseStart;
      if (w >= pulseMinW && w <= pulseMaxW) pulseCount++;
    }

    pulsePerCycleAcc += pulseCount;
    pulseCycles++;
    cyclePeaks[peakN++] = peak;

    float curCycle[RSZ];
    for (int j = 0; j < RSZ; ++j) {
      const float pos = (float)a + ((float)j * (float)(len - 1) / (float)(RSZ - 1));
      const int i0 = clampi((int)floorf(pos), a, b - 1);
      const int i1 = clampi(i0 + 1, a, b - 1);
      const float frac = pos - (float)i0;
      curCycle[j] = sigClean[i0] + frac * (sigClean[i1] - sigClean[i0]);
    }

    if (havePrevCycle) {
      double mse = 0.0;
      double eRef = 0.0;
      for (int j = 0; j < RSZ; ++j) {
        const double d = (double)curCycle[j] - (double)prevCycle[j];
        mse += d * d;
        eRef += 0.5 * ((double)curCycle[j] * (double)curCycle[j] + (double)prevCycle[j] * (double)prevCycle[j]);
      }
      nmseAcc += (float)(mse / (eRef + 1e-9));
      nmsePairs++;
    }
    memcpy(prevCycle, curCycle, sizeof(prevCycle));
    havePrevCycle = true;
  }

  if (pulseCycles > 0) out.pulse_count_per_cycle = pulsePerCycleAcc / (float)pulseCycles;
  if (peakN >= 2) {
    double mu = 0.0;
    for (int i = 0; i < peakN; ++i) mu += cyclePeaks[i];
    mu /= (double)peakN;
    double vv = 0.0;
    for (int i = 0; i < peakN; ++i) {
      const double d = (double)cyclePeaks[i] - mu;
      vv += d * d;
    }
    vv /= (double)peakN;
    out.peak_fluct_cv = (float)(sqrt(vv) / (mu + 1e-9));
  }
  if (nmsePairs > 0) out.cycle_nmse = nmseAcc / (float)nmsePairs;

  // --- FFT-domain features ---
  for (size_t i = 0; i < n; ++i) {
    fft_cf[2 * i + 0] = sig[i] * win[i];
    fft_cf[2 * i + 1] = 0.0f;
  }

  if (dsps_fft2r_fc32(fft_cf, (int)n) != ESP_OK) return true;
  if (dsps_bit_rev_fc32(fft_cf, (int)n) != ESP_OK) return true;

  const int half = (int)(n / 2);
  double pTotNoDc = 0.0;
  for (int k = 0; k < half; ++k) {
    const float re = fft_cf[2 * k + 0];
    const float im = fft_cf[2 * k + 1];
    const float P = re * re + im * im;
    sig[k] = P;
    if (k > 0) pTotNoDc += P;
  }

  const float binHz = fs_hz / (float)n;
  int kNom = (int)lroundf(mainsHz / binHz);
  kNom = clampi(kNom, 1, half - 2);

  int k1 = kNom;
  float fundP = 0.0f;
  for (int dk = -2; dk <= 2; ++dk) {
    const int k = clampi(kNom + dk, 1, half - 1);
    if (sig[k] > fundP) {
      fundP = sig[k];
      k1 = k;
    }
  }

  const double avgNoise = pTotNoDc / (double)(half - 1);
  const double snr = (avgNoise > 0.0) ? ((double)fundP / avgNoise) : 0.0;
  const bool haveFund = (fundP >= FUND_MAG_MIN) && (snr >= FUND_SNR_MIN);

  if (haveFund) {
    double harmP = 0.0;
    for (int h = 2; h <= 10; ++h) {
      const int khNom = h * k1;
      if (khNom >= half) break;
      float hmP = 0.0f;
      for (int dk = -1; dk <= 1; ++dk) {
        const int kh = clampi(khNom + dk, 1, half - 1);
        hmP = fmaxf(hmP, sig[kh]);
      }
      harmP += hmP;
    }
    out.thd_i = (fundP > 1e-12f) ? (float)(sqrt(harmP / (double)fundP) * 100.0) : 0.0f;
  }

  const int kSpec0 = clampi((int)floorf(SPEC_ENT_LO_HZ / binHz), 1, half - 1);
  const int kSpec1 = clampi((int)floorf(SPEC_ENT_HI_HZ / binHz), kSpec0 + 1, half - 1);
  double psum = 0.0;
  for (int k = kSpec0; k <= kSpec1; ++k) psum += (double)sig[k];
  if (psum > 1e-18) {
    double H = 0.0;
    for (int k = kSpec0; k <= kSpec1; ++k) {
      const double p = (double)sig[k] / psum;
      if (p > 1e-18) H += -p * log(p);
    }
    out.spec_entropy = clampf((float)(H / log((double)(kSpec1 - kSpec0 + 1))), 0.0f, 1.0f);
  }

  const int kUm0 = clampi((int)floorf(UPPERMID_LO_HZ / binHz), 1, half - 1);
  const int kUm1 = clampi((int)floorf(UPPERMID_HI_HZ / binHz), kUm0 + 1, half - 1);
  const int kHf0 = clampi((int)floorf(HF_BAND_LO_HZ / binHz), 1, half - 1);
  const int kHf1 = clampi((int)floorf(HF_BAND_HI_HZ / binHz), kHf0 + 1, half - 1);
  double pUM = 0.0;
  double pHF = 0.0;
  for (int k = kUm0; k <= kUm1; ++k) pUM += (double)sig[k];
  for (int k = kHf0; k <= kHf1; ++k) pHF += (double)sig[k];
  out.hf_band_energy_ratio = (float)(pHF / (pHF + pUM + 1e-12));

  const int kMb0 = clampi((int)floorf(MIDBAND_LO_HZ / binHz), 1, half - 1);
  const int kMb1 = clampi((int)floorf(MIDBAND_HI_HZ / binHz), kMb0 + 1, half - 1);
  double pResidual = 0.0;
  for (int k = kMb0; k <= kMb1; ++k) {
    bool skip = false;
    if (haveFund) {
      for (int h = 1; h <= 3; ++h) {
        const int kh = h * k1;
        if (kh >= half) break;
        if (abs(k - kh) <= 2) { skip = true; break; }
      }
    }
    if (!skip) pResidual += (double)sig[k];
  }
  out.midband_residual_rms = (float)(sqrt(pResidual + 1e-12) / (double)n);

  // --- compact wavelet packet energy entropy (3-level Haar on 512 points) ---
  static constexpr int WP_N = 512;
  static float wpA[WP_N];
  static float wpB[WP_N];
  int step = (int)(n / WP_N);
  if (step < 1) step = 1;
  for (int i = 0; i < WP_N; ++i) {
    const int idx = clampi(i * step, 0, (int)n - 1);
    wpA[i] = sigClean[idx];
  }

  int segLen = WP_N;
  float* in = wpA;
  float* outBuf = wpB;
  for (int level = 0; level < 3; ++level) {
    const int nodeCount = 1 << level;
    const int nodeLen = segLen / nodeCount;
    for (int node = 0; node < nodeCount; ++node) {
      const int base = node * nodeLen;
      const int halfLen = nodeLen / 2;
      for (int j = 0; j < halfLen; ++j) {
        const float s0 = in[base + 2 * j];
        const float s1 = in[base + 2 * j + 1];
        outBuf[base + j] = (s0 + s1) * 0.70710678f;
        outBuf[base + halfLen + j] = (s0 - s1) * 0.70710678f;
      }
    }
    float* tmp = in;
    in = outBuf;
    outBuf = tmp;
  }
  float leafE[8] = {0};
  for (int leaf = 0; leaf < 8; ++leaf) {
    for (int j = 0; j < 64; ++j) {
      const float s = in[leaf * 64 + j];
      leafE[leaf] += s * s;
    }
  }
  out.wpe_entropy = computeEntropyFromBands(leafE, 8);

  out.feat_valid = true;
  return true;
}
