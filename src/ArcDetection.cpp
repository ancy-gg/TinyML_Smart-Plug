#include "ArcDetection.h"
#include <math.h>
#include <string.h>

#include "TinyML_RF.h"
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

static inline int clampi(int x, int lo, int hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}

static inline float median3f(float a, float b, float c) {
  if (a > b) { const float t = a; a = b; b = t; }
  if (b > c) { const float t = b; b = c; c = t; }
  if (a > b) { const float t = a; a = b; b = t; }
  return b;
}

struct BiquadLPF {
  float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
  float a1 = 0.0f, a2 = 0.0f;
  float z1 = 0.0f, z2 = 0.0f;

  inline float step(float x) {
    const float y = b0 * x + z1;
    z1 = b1 * x - a1 * y + z2;
    z2 = b2 * x - a2 * y;
    return y;
  }
};

static bool makeLowpassBiquad(float fs_hz, float cutoff_hz, float q, BiquadLPF& biq) {
  if (fs_hz < 10.0f || cutoff_hz <= 0.0f) return false;

  const float nyq = 0.5f * fs_hz;
  float fc = cutoff_hz;
  const float maxCut = fmaxf(10.0f, CURRENT_SOFT_AAF_MAX_FRAC_NYQUIST * nyq);
  if (fc > maxCut) fc = maxCut;
  if (fc < 5.0f) fc = 5.0f;

  const float w0 = 2.0f * 3.14159265358979f * (fc / fs_hz);
  const float cw = cosf(w0);
  const float sw = sinf(w0);
  const float alpha = sw / (2.0f * fmaxf(0.05f, q));

  const float b0 = (1.0f - cw) * 0.5f;
  const float b1 = 1.0f - cw;
  const float b2 = (1.0f - cw) * 0.5f;
  const float a0 = 1.0f + alpha;
  const float a1 = -2.0f * cw;
  const float a2 = 1.0f - alpha;

  if (fabsf(a0) < 1e-12f) return false;

  biq.b0 = b0 / a0;
  biq.b1 = b1 / a0;
  biq.b2 = b2 / a0;
  biq.a1 = a1 / a0;
  biq.a2 = a2 / a0;
  biq.z1 = 0.0f;
  biq.z2 = 0.0f;
  return true;
}

static inline float codeToCurrentA(uint16_t code, const CurrentCalib& cal) {
  const float v_adc = (float(code) * cal.adcFullScaleV) / 65535.0f;
  const float v_sensor = (cal.dividerRatio > 1e-9f) ? (v_adc / cal.dividerRatio) : 0.0f;
  const float amps_uncal = ((v_sensor - cal.offsetV) / cal.voltsPerAmp) * cal.ampsScale;
  return amps_uncal;
}


static inline bool makeBandBins(float f0, float f1, float binHz, int half, int& k0, int& k1) {
  if (half <= 1 || binHz <= 0.0f) return false;

  const float nyquist = binHz * float(half - 1);
  if (f0 >= nyquist) return false;

  float a = f0;
  float b = f1;
  if (a < binHz) a = binHz;
  if (b > nyquist) b = nyquist;
  if (b < a) return false;

  k0 = clampi((int)floorf(a / binHz), 1, half - 1);
  k1 = clampi((int)floorf(b / binHz), k0, half - 1);
  return (k1 >= k0);
}

static inline float interpZeroCrossIndex(float a, float b, int i0) {
  const float denom = (b - a);
  const float frac = (fabsf(denom) > 1e-12f) ? ((0.0f - a) / denom) : 0.5f;
  return (float)i0 + clampf(frac, 0.0f, 1.0f);
}

enum ZcRegion : uint8_t {
  ZC_MID = 0,
  ZC_POS = 1,
  ZC_NEG = 2
};

static inline ZcRegion classifyRegion(float x, float hys) {
  if (x >= hys) return ZC_POS;
  if (x <= -hys) return ZC_NEG;
  return ZC_MID;
}

bool ArcDetection::compute(const uint16_t* raw, size_t n, float fs_hz,
                           const CurrentCalib& cal, float mainsHz,
                           ArcDetectionResult& out) {
  out = ArcDetectionResult{};
  out.fs_hz = fs_hz;

  if (!raw || n != N_SAMP || fs_hz < 1000.0f) return false;
  if (!dsp_is_power_of_two((int)n)) return false;
  if ((int)n > CONFIG_DSP_MAX_FFT_SIZE) return false;

  static float sig[N_SAMP];
  static float sigMed[N_SAMP];
  static float sigFilt[N_SAMP];
  static float sigClean[N_SAMP];
  static float sigBase[N_SAMP];
  static float resid[N_SAMP];
  static float fft_cf[2 * N_SAMP];
  static float win[N_SAMP];
  static bool dspReady = false;
  static bool winReady = false;

  double mean = 0.0;
  int changes = 0;
  uint16_t mnCode = 0xFFFF;
  uint16_t mxCode = 0;
  uint16_t prev = raw[0];

  for (size_t i = 0; i < n; ++i) {
    const uint16_t c = raw[i];
    if (c < mnCode) mnCode = c;
    if (c > mxCode) mxCode = c;
    if (i && c != prev) changes++;
    prev = c;
    sig[i] = codeToCurrentA(c, cal);
  }

  if (changes < CURRENT_MIN_ACTIVITY_CHANGES || (uint16_t)(mxCode - mnCode) < CURRENT_MIN_CODE_SPAN) {
    return false;
  }

  sigMed[0] = sig[0];
  for (size_t i = 1; i + 1 < n; ++i) sigMed[i] = median3f(sig[i - 1], sig[i], sig[i + 1]);
  sigMed[n - 1] = sig[n - 1];

  for (size_t i = 0; i < n; ++i) mean += sigMed[i];
  mean /= (double)n;
  for (size_t i = 0; i < n; ++i) sig[i] = sigMed[i] - (float)mean;

  if (CURRENT_SOFT_AAF_ENABLE) {
    BiquadLPF softAaf;
    if (makeLowpassBiquad(fs_hz, CURRENT_SOFT_AAF_CUTOFF_HZ, CURRENT_SOFT_AAF_Q, softAaf)) {
      for (size_t i = 0; i < n; ++i) sigFilt[i] = softAaf.step(sig[i]);
    } else {
      memcpy(sigFilt, sig, n * sizeof(float));
    }
  } else {
    memcpy(sigFilt, sig, n * sizeof(float));
  }

  const float dt = 1.0f / fs_hz;
  const float rcClean = 1.0f / (6.2831853f * CURRENT_LPF_HZ);
  const float rcBase  = 1.0f / (6.2831853f * CURRENT_BASE_LPF_HZ);
  const float aClean = dt / (rcClean + dt);
  const float aBase  = dt / (rcBase + dt);

  float yClean = 0.0f;
  float yBase  = 0.0f;
  double accIrmsWide = 0.0;
  double accResidSq  = 0.0;

  for (size_t i = 0; i < n; ++i) {
    yClean += aClean * (sigFilt[i] - yClean);
    yBase  += aBase  * (sigFilt[i] - yBase);
    sigClean[i] = yClean;
    sigBase[i]  = yBase;
    resid[i]    = yClean - yBase;
    accIrmsWide += (double)sigFilt[i] * (double)sigFilt[i];
    accResidSq  += (double)resid[i] * (double)resid[i];
  }

  const float irmsWide = sqrtf((float)(accIrmsWide / (double)n));
  const float residRms = sqrtf((float)(accResidSq / (double)n));
  const uint16_t codeSpan = (uint16_t)(mxCode - mnCode);
  const bool strongActivity = (changes >= CURRENT_MIN_ACTIVITY_CHANGES) || (codeSpan >= CURRENT_MIN_CODE_SPAN);
  const bool deepIdle = (changes < CURRENT_MIN_ACTIVITY_CHANGES) && (codeSpan < LOW_CURRENT_CODE_SPAN) &&
                        (irmsWide < CURRENT_IDLE_SUPPRESS_A);

  float irms = irmsWide;
  if (deepIdle) irms = 0.0f;
  else if (irms < IRMS_GATE_OFF_A && !strongActivity) irms = 0.0f;

  if (irms > 0.0f) {
    irms = eval_cubic_horner(irms, cal.cubic3, cal.cubic2, cal.cubic1, cal.cubic0);
    if (irms < 0.0f) irms = 0.0f;
  }

  out.irms_a = irms;
  out.current_valid = strongActivity || (irms > 0.0f);

  const bool weakArcWindow = strongActivity && (residRms >= (PULSE_ANALYSIS_MIN_RESID_A * ARC_WEAK_EVENT_RESID_MUL));
  if (irms < FEATURE_MIN_IRMS_A && !weakArcWindow) {
    out.feat_valid = false;
    return true;
  }

  if (!dspReady) {
    if (dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE) != ESP_OK) {
      out.feat_valid = false;
      return true;
    }
    dspReady = true;
  }

  if (!winReady) {
    dsps_wind_hann_f32(win, (int)n);
    winReady = true;
  }

  float crossAll[128];
  int crossAllN = 0;
  float crossPos[64];
  int crossPosN = 0;

  const float zcHys = fmaxf(ZC_HYS_MIN_A, ZC_HYS_FRAC * fmaxf(irms, 0.1f));
  ZcRegion armedSide = classifyRegion(sigClean[0], zcHys);

  for (size_t i = 1; i < n && crossAllN < 128; ++i) {
    const float a = sigClean[i - 1];
    const float b = sigClean[i];
    const ZcRegion r = classifyRegion(b, zcHys);
    if (r == ZC_POS || r == ZC_NEG) armedSide = r;

    if (armedSide == ZC_NEG && a <= 0.0f && b > 0.0f) {
      const float idx = interpZeroCrossIndex(a, b, (int)(i - 1));
      crossAll[crossAllN++] = idx;
      if (crossPosN < 64) crossPos[crossPosN++] = idx;
      armedSide = ZC_MID;
      continue;
    }

    if (armedSide == ZC_POS && a >= 0.0f && b < 0.0f) {
      const float idx = interpZeroCrossIndex(a, b, (int)(i - 1));
      crossAll[crossAllN++] = idx;
      armedSide = ZC_MID;
      continue;
    }
  }

  if (crossAllN < 3) {
    out.feat_valid = false;
    return true;
  }

  if (crossAllN >= 4) {
    double mu = 0.0;
    for (int i = 1; i < crossAllN; ++i) mu += (double)(crossAll[i] - crossAll[i - 1]);
    mu /= (double)(crossAllN - 1);

    double vv = 0.0;
    for (int i = 1; i < crossAllN; ++i) {
      const double d = (double)(crossAll[i] - crossAll[i - 1]) - mu;
      vv += d * d;
    }
    vv /= (double)(crossAllN - 1);
    out.zcv = (float)(sqrt(vv) * (1000.0 / fs_hz));
  }

  const float dwellThr = fmaxf(ZC_DWELL_THR_MIN_A, ZC_DWELL_THR_FRAC * fmaxf(irms, 0.10f));
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

  const int cycleCount = crossPosN - 1;
  if (cycleCount <= 0) {
    out.feat_valid = false;
    return true;
  }

  float cyclePeaks[24];
  float cycleRmsVals[24];
  int peakN = 0;
  float nmseAcc = 0.0f;
  int nmsePairs = 0;

  static constexpr int RSZ = 96;
  float prevCycle[RSZ];
  bool havePrevCycle = false;

  for (int c = 0; c < cycleCount && c < 24; ++c) {
    const int a = clampi((int)floorf(crossPos[c]), 0, (int)n - 2);
    const int b = clampi((int)floorf(crossPos[c + 1]), a + 2, (int)n - 1);
    const int len = b - a;
    if (len < 16) continue;

    float peak = 0.0f;
    double cycSq = 0.0;

    for (int i = a; i < b; ++i) {
      const float av = fabsf(sigClean[i]);
      if (av > peak) peak = av;
      cycSq += (double)sigClean[i] * (double)sigClean[i];
    }

    cyclePeaks[peakN] = peak;
    cycleRmsVals[peakN] = (len > 0) ? sqrtf((float)(cycSq / (double)len)) : 0.0f;
    peakN++;

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

  if (peakN >= 2) {
    float tmp[24];
    for (int i = 0; i < peakN; ++i) tmp[i] = cycleRmsVals[i];
    for (int i = 0; i < peakN - 1; ++i) {
      for (int j = i + 1; j < peakN; ++j) {
        if (tmp[j] < tmp[i]) { const float t = tmp[i]; tmp[i] = tmp[j]; tmp[j] = t; }
      }
    }
    const float baseline = (peakN & 1) ? tmp[peakN / 2]
                                       : 0.5f * (tmp[(peakN / 2) - 1] + tmp[peakN / 2]);
    float minCycle = cycleRmsVals[0];
    for (int i = 1; i < peakN; ++i) {
      if (cycleRmsVals[i] < minCycle) minCycle = cycleRmsVals[i];
    }
    out.cycle_rms_drop_ratio = clampf((baseline - minCycle) / fmaxf(baseline, 0.05f), 0.0f, 1.5f);
  }

  if (nmsePairs > 0) out.cycle_nmse = nmseAcc / (float)nmsePairs;

  for (size_t i = 0; i < n; ++i) {
    fft_cf[2 * i + 0] = sigFilt[i] * win[i];
    fft_cf[2 * i + 1] = 0.0f;
  }

  if (dsps_fft2r_fc32(fft_cf, (int)n) != ESP_OK) {
    out.feat_valid = false;
    return true;
  }
  if (dsps_bit_rev_fc32(fft_cf, (int)n) != ESP_OK) {
    out.feat_valid = false;
    return true;
  }

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
    if (sig[k] > fundP) { fundP = sig[k]; k1 = k; }
  }

  const double avgNoise = pTotNoDc / (double)(half - 1);
  const double snr = (avgNoise > 0.0) ? ((double)fundP / avgNoise) : 0.0;
  const bool haveFund = (fundP >= FUND_MAG_MIN) && (snr >= FUND_SNR_MIN);

  if (!haveFund && irms < FEATURE_REQUIRE_FUND_BELOW_A) {
    out.feat_valid = false;
    return true;
  }

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
  } else {
    out.thd_i = 0.0f;
  }

  int kSpec0 = 0, kSpec1 = 0;
  if (makeBandBins(SPEC_ENT_LO_HZ, SPEC_ENT_HI_HZ, binHz, half, kSpec0, kSpec1)) {
    double psum = 0.0;
    for (int k = kSpec0; k <= kSpec1; ++k) psum += (double)sig[k];
    if (psum > 1e-18) {
      double H = 0.0;
      const int bins = (kSpec1 - kSpec0 + 1);
      for (int k = kSpec0; k <= kSpec1; ++k) {
        const double p = (double)sig[k] / psum;
        if (p > 1e-18) H += -p * log(p);
      }
      if (bins > 1) out.spec_entropy = clampf((float)(H / log((double)bins)), 0.0f, 1.0f);
    }
  }

  int kUm0 = 0, kUm1 = 0, kHf0 = 0, kHf1 = 0;
  double pUM = 0.0;
  double pHF = 0.0;

  if (makeBandBins(UPPERMID_LO_HZ, UPPERMID_HI_HZ, binHz, half, kUm0, kUm1)) {
    for (int k = kUm0; k <= kUm1; ++k) pUM += (double)sig[k];
  }
  if (makeBandBins(HF_BAND_LO_HZ, HF_BAND_HI_HZ, binHz, half, kHf0, kHf1)) {
    for (int k = kHf0; k <= kHf1; ++k) pHF += (double)sig[k];
  }
  out.hf_band_energy_ratio = (float)(pHF / (pHF + pUM + 1e-12));

  int kMb0 = 0, kMb1 = 0;
  if (makeBandBins(MIDBAND_LO_HZ, MIDBAND_HI_HZ, binHz, half, kMb0, kMb1)) {
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
  } else {
    out.midband_residual_rms = 0.0f;
  }

  int negDipComparisons = 0;
  int negDipCount = 0;
  float preDipSpikeAcc = 0.0f;
  int preDipSpikeN = 0;

  if (peakN >= 5) {
    for (int i = 4; i < peakN; ++i) {
      float baseR[3] = { cycleRmsVals[i - 4], cycleRmsVals[i - 3], cycleRmsVals[i - 2] };
      float baseP[3] = { cyclePeaks[i - 4], cyclePeaks[i - 3], cyclePeaks[i - 2] };
      for (int a = 0; a < 2; ++a) {
        for (int b = a + 1; b < 3; ++b) {
          if (baseR[b] < baseR[a]) { const float t = baseR[a]; baseR[a] = baseR[b]; baseR[b] = t; }
          if (baseP[b] < baseP[a]) { const float t = baseP[a]; baseP[a] = baseP[b]; baseP[b] = t; }
        }
      }

      const float baselineR = baseR[1];
      const float baselineP = baseP[1];
      if (baselineR < 0.08f || baselineP < 0.05f) continue;

      const float prevR = cycleRmsVals[i - 1];
      const float curR  = cycleRmsVals[i];
      const float prevP = cyclePeaks[i - 1];

      negDipComparisons++;

      const bool prevLoaded = prevR >= (0.70f * baselineR);
      const bool curDrop = curR <= (0.78f * baselineR);
      if (prevLoaded && curDrop) {
        negDipCount++;
        const float spikeRatio = clampf((prevP - baselineP) / fmaxf(baselineP, 0.05f), 0.0f, 2.0f);
        preDipSpikeAcc += spikeRatio;
        preDipSpikeN++;
      }
    }
  }

  out.neg_dip_event_ratio = (negDipComparisons > 0)
      ? clampf((float)negDipCount / (float)negDipComparisons, 0.0f, 1.0f)
      : 0.0f;
  out.pre_dip_spike_ratio = (preDipSpikeN > 0)
      ? clampf(preDipSpikeAcc / (float)preDipSpikeN, 0.0f, 2.0f)
      : 0.0f;

  out.feat_valid = true;
  return true;
}

int ArcDetection::predict(float cycle_nmse, float zcv, float zc_dwell_ratio,
                          float cycle_rms_drop_ratio, float peak_fluct_cv,
                          float midband_residual_rms, float hf_band_energy_ratio,
                          float spec_entropy, float neg_dip_event_ratio, float pre_dip_spike_ratio,
                          float v_rms, float i_rms, float temp_c) const {
  (void)v_rms; (void)i_rms; (void)temp_c;
#if defined(ARC_MODEL_FEATURE_VERSION) && (ARC_MODEL_FEATURE_VERSION >= 2)
  double input_features[10] = {
    (double)cycle_nmse, (double)zcv, (double)zc_dwell_ratio,
    (double)cycle_rms_drop_ratio, (double)peak_fluct_cv,
    (double)midband_residual_rms, (double)hf_band_energy_ratio,
    (double)spec_entropy, (double)neg_dip_event_ratio, (double)pre_dip_spike_ratio
  };
  double output_probs[2] = {0.0, 0.0};
  arc_rf_predict(input_features, output_probs);
  return (output_probs[1] >= ARC_THRESHOLD) ? 1 : 0;
#else
  float score = 0.0f;
  if (cycle_nmse >= 0.10f) score += 1.4f;
  if (zcv >= 0.12f) score += 1.1f;
  if (zc_dwell_ratio >= 0.14f) score += 0.7f;
  if (cycle_rms_drop_ratio >= 0.10f) score += 1.3f;
  if (peak_fluct_cv >= 0.08f) score += 0.8f;
  if (midband_residual_rms >= 0.06f) score += 1.0f;
  if (hf_band_energy_ratio >= 0.20f) score += 0.8f;
  if (spec_entropy >= 0.62f) score += 0.6f;
  if (neg_dip_event_ratio >= 0.18f) score += 1.6f;
  if (pre_dip_spike_ratio >= 0.10f) score += 1.1f;
  return (score >= 3.4f) ? 1 : 0;
#endif
}

int ArcDetection::computeAndPredict(const uint16_t* raw, size_t n, float fs_hz,
                                    const CurrentCalib& cal, float mainsHz,
                                    float v_rms, float temp_c,
                                    ArcDetectionResult& out) {
  if (!compute(raw, n, fs_hz, cal, mainsHz, out)) return 0;
  if (!out.current_valid || !out.feat_valid || out.irms_a < ARC_MIN_IRMS_A) {
    out.model_pred = 0;
    return 0;
  }
  out.model_pred = (uint8_t)predict(out.cycle_nmse, out.zcv, out.zc_dwell_ratio,
                                    out.cycle_rms_drop_ratio, out.peak_fluct_cv,
                                    out.midband_residual_rms, out.hf_band_energy_ratio,
                                    out.spec_entropy, out.neg_dip_event_ratio, out.pre_dip_spike_ratio,
                                    v_rms, out.irms_a, temp_c);
  return out.model_pred;
}
