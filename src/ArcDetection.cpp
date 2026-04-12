#include "ArcDetection.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>

#include "TinyMLTreeEnsemble_RF.h"
#include "esp_dsp.h"
#include "dsps_fft2r.h"
#include "dsps_wind_hann.h"
#include "dsp_common.h"
#include "esp_err.h"

#ifndef CONFIG_DSP_MAX_FFT_SIZE
#define CONFIG_DSP_MAX_FFT_SIZE N_SAMP
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

static float medianCopy_(const float* src, int n) {
  if (!src || n <= 0) return 0.0f;
  float buf[128];
  if (n > (int)(sizeof(buf) / sizeof(buf[0]))) n = (int)(sizeof(buf) / sizeof(buf[0]));
  for (int i = 0; i < n; ++i) buf[i] = src[i];
  for (int i = 1; i < n; ++i) {
    const float v = buf[i];
    int j = i - 1;
    while (j >= 0 && buf[j] > v) {
      buf[j + 1] = buf[j];
      --j;
    }
    buf[j + 1] = v;
  }
  if (n & 1) return buf[n / 2];
  return 0.5f * (buf[n / 2 - 1] + buf[n / 2]);
}

static float robustIntervalJitterMs_(const float* xs, int n, float fs_hz) {
  if (!xs || n < 3 || fs_hz <= 0.0f) return 0.0f;
  float diffs[127];
  int m = 0;
  for (int i = 1; i < n && m < (int)(sizeof(diffs) / sizeof(diffs[0])); ++i) {
    const float d = xs[i] - xs[i - 1];
    if (d > 1.0f) diffs[m++] = d;
  }
  if (m < 2) return 0.0f;
  const float med = medianCopy_(diffs, m);
  if (med <= 0.0f) return 0.0f;
  float devs[127];
  for (int i = 0; i < m; ++i) devs[i] = fabsf(diffs[i] - med);
  const float mad = medianCopy_(devs, m);
  const float robustSigmaSamples = 1.4826f * mad;
  return robustSigmaSamples * (1000.0f / fs_hz);
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


static float goertzelTonePower_(const float* x, size_t n, float fs_hz, float tone_hz) {
  if (!x || n == 0 || fs_hz <= 0.0f || tone_hz <= 0.0f || tone_hz >= (0.5f * fs_hz)) return 0.0f;
  const float omega = (2.0f * 3.14159265358979f * tone_hz) / fs_hz;
  const float coeff = 2.0f * cosf(omega);
  float s1 = 0.0f;
  float s2 = 0.0f;
  for (size_t i = 0; i < n; ++i) {
    const float s0 = x[i] + coeff * s1 - s2;
    s2 = s1;
    s1 = s0;
  }
  const float power = (s1 * s1) + (s2 * s2) - (coeff * s1 * s2);
  return fmaxf(power, 0.0f);
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

static void collectZeroCrossings_(const float* sigClean, size_t n, float zcHys,
                                  float* crossAll, int crossAllCap, int& crossAllN,
                                  float* crossPos, int crossPosCap, int& crossPosN) {
  crossAllN = 0;
  crossPosN = 0;
  if (!sigClean || n < 2 || !crossAll || !crossPos) return;

  ZcRegion armedSide = classifyRegion(sigClean[0], zcHys);
  for (size_t i = 1; i < n && crossAllN < crossAllCap; ++i) {
    const float a = sigClean[i - 1];
    const float b = sigClean[i];
    const ZcRegion r = classifyRegion(b, zcHys);
    if (r == ZC_POS || r == ZC_NEG) armedSide = r;

    if (armedSide == ZC_NEG && a <= 0.0f && b > 0.0f) {
      const float idx = interpZeroCrossIndex(a, b, (int)(i - 1));
      crossAll[crossAllN++] = idx;
      if (crossPosN < crossPosCap) crossPos[crossPosN++] = idx;
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
}


struct RollingBaselineTracker {
  bool initialized = false;
  uint32_t freezeUntilMs = 0;
  uint32_t idleSinceMs = 0;
  float irmsMean = 0.0f;
  float irmsVar = 0.0f;
  float hfMean = 0.0f;
  float lastIrms = 0.0f;

  inline void reset() {
    initialized = false;
    freezeUntilMs = 0;
    idleSinceMs = 0;
    irmsMean = 0.0f;
    irmsVar = 0.0f;
    hfMean = 0.0f;
    lastIrms = 0.0f;
  }

  inline float baselineIrms() const {
    if (initialized) return fmaxf(irmsMean, EDGE_SPIKE_MIN_BASELINE_A);
    return fmaxf(lastIrms, EDGE_SPIKE_MIN_BASELINE_A);
  }

  inline float baselineStd() const {
    const float base = baselineIrms();
    const float floorA = fmaxf(BASELINE_STD_FLOOR_A, BASELINE_STD_FLOOR_FRAC * base);
    return initialized ? fmaxf(sqrtf(fmaxf(irmsVar, 0.0f)), floorA) : floorA;
  }

  inline float baselineHf() const {
    return initialized ? hfMean : 0.0f;
  }
};

static float s_prevFluxNorm[N_SAMP / 2] = {0.0f};
static bool s_havePrevFluxNorm = false;
static RollingBaselineTracker s_baseline;
static int8_t s_ctxFamily = CONTEXT_FAMILY_UNKNOWN;
static float s_ctxConfidence = 0.0f;

void ArcDetection::resetRuntime() {
  memset(s_prevFluxNorm, 0, sizeof(s_prevFluxNorm));
  s_havePrevFluxNorm = false;
  s_baseline.reset();
  s_ctxFamily = CONTEXT_FAMILY_UNKNOWN;
  s_ctxConfidence = 0.0f;
}

void ArcDetection::setContext(int8_t family, float confidence) {
  s_ctxFamily = ((family >= 0) && (family < FAMILY_COUNT)) ? family : CONTEXT_FAMILY_UNKNOWN;
  s_ctxConfidence = clampf(confidence, 0.0f, 1.0f);
}

static inline void sanitizeArcDetectionResult_(ArcDetectionResult& out) {
  if (!isfinite(out.fs_hz) || out.fs_hz < 0.0f) out.fs_hz = 0.0f;
  if (!isfinite(out.irms_a) || out.irms_a < 0.0f) out.irms_a = 0.0f;

  out.abs_irms_zscore_vs_baseline =
      tinymlClampFeatureValue(TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE, out.abs_irms_zscore_vs_baseline);
  out.delta_irms_abs =
      tinymlClampFeatureValue(TINYML_FEATURE_DELTA_IRMS_ABS, out.delta_irms_abs);
  out.halfcycle_asymmetry =
      tinymlClampFeatureValue(TINYML_FEATURE_HALFCYCLE_ASYMMETRY, out.halfcycle_asymmetry);
  out.suspicious_run_energy =
      tinymlClampFeatureValue(TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY, out.suspicious_run_energy);
  out.delta_hf_energy =
      tinymlClampFeatureValue(TINYML_FEATURE_DELTA_HF_ENERGY, out.delta_hf_energy);
  out.delta_flux =
      tinymlClampFeatureValue(TINYML_FEATURE_DELTA_FLUX, out.delta_flux);
  out.midband_residual_ratio =
      tinymlClampFeatureValue(TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO, out.midband_residual_ratio);
  out.zcv =
      tinymlClampFeatureValue(TINYML_FEATURE_ZCV, out.zcv);
  out.spectral_flux_midhf =
      tinymlClampFeatureValue(TINYML_FEATURE_SPECTRAL_FLUX_MIDHF, out.spectral_flux_midhf);
  out.peak_fluct_cv =
      tinymlClampFeatureValue(TINYML_FEATURE_PEAK_FLUCT_CV, out.peak_fluct_cv);
  out.residual_crest_factor =
      tinymlClampFeatureValue(TINYML_FEATURE_RESIDUAL_CREST_FACTOR, out.residual_crest_factor);
  out.thd_i =
      tinymlClampFeatureValue(TINYML_FEATURE_THD_I, out.thd_i);
  out.hf_energy_delta =
      tinymlClampFeatureValue(TINYML_FEATURE_HF_ENERGY_DELTA, out.hf_energy_delta);
  out.edge_spike_ratio =
      tinymlClampFeatureValue(TINYML_FEATURE_EDGE_SPIKE_RATIO, out.edge_spike_ratio);
  out.v_sag_pct =
      tinymlClampFeatureValue(TINYML_FEATURE_V_SAG_PCT, out.v_sag_pct);
  out.cycle_nmse =
      tinymlClampFeatureValue(TINYML_FEATURE_CYCLE_NMSE, out.cycle_nmse);
}

static inline bool shouldSkipHarmonicBin(int k, bool haveFund, int k1, int half) {
  if (!haveFund || k1 <= 0) return false;
  for (int h = 1; h <= 24; ++h) {
    const int kh = h * k1;
    if (kh >= half) break;
    if (abs(k - kh) <= SPECTRAL_FLUX_HARM_SKIP_BINS) return true;
  }
  return false;
}

static inline float bandPeakExcess(const float* spec, int half, int kCenter, int halfWidth = 1, int floorOffset = 4) {
  float peak = 0.0f;
  for (int dk = -halfWidth; dk <= halfWidth; ++dk) {
    const int k = clampi(kCenter + dk, 1, half - 1);
    peak = fmaxf(peak, spec[k]);
  }
  float noise = 0.0f;
  int cnt = 0;
  for (int dk = -1; dk <= 1; ++dk) {
    int kl = kCenter - floorOffset + dk;
    int kr = kCenter + floorOffset + dk;
    if (kl >= 1 && kl < half) { noise += spec[kl]; cnt++; }
    if (kr >= 1 && kr < half) { noise += spec[kr]; cnt++; }
  }
  if (cnt > 0) noise /= (float)cnt;
  return fmaxf(0.0f, peak - noise);
}

bool ArcDetection::compute(const uint16_t* raw, size_t n, float fs_hz,
                           const CurrentCalib& cal, float mainsHz,
                           ArcDetectionResult& out) {
  out = ArcDetectionResult{};
  out.fs_hz = fs_hz;

  if (!raw || fs_hz < 1000.0f) return false;
  if (!(n == ARC_RUNTIME_FAST_FFT_SAMPLES || n == ARC_RUNTIME_LEGACY_FFT_SAMPLES)) return false;
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
  static uint8_t harmonicSkip[N_SAMP / 2];
  static bool dspReady = false;
  static bool winReady = false;
  static int winLen = 0;

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
    // Analog front-end is already ~10 kHz/Q≈0.73. Run a conservative cascaded
    // digital LPF below that corner so feature extraction sees a cleaner band-
    // limited waveform even when the measured ADC rate wanders.
    BiquadLPF softAaf[CURRENT_SOFT_AAF_STAGES];
    bool aafReady = true;
    for (uint8_t st = 0; st < CURRENT_SOFT_AAF_STAGES; ++st) {
      if (!makeLowpassBiquad(fs_hz, CURRENT_SOFT_AAF_CUTOFF_HZ, CURRENT_SOFT_AAF_Q, softAaf[st])) {
        aafReady = false;
        break;
      }
    }
    if (aafReady) {
      for (size_t i = 0; i < n; ++i) {
        float y = sig[i];
        for (uint8_t st = 0; st < CURRENT_SOFT_AAF_STAGES; ++st) y = softAaf[st].step(y);
        sigFilt[i] = y;
      }
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

  if (!winReady || winLen != (int)n) {
    dsps_wind_hann_f32(win, (int)n);
    winReady = true;
    winLen = (int)n;
  }

  float crossAll[128];
  int crossAllN = 0;
  float crossPos[64];
  int crossPosN = 0;

  const float zcHysBase = fmaxf(ZC_HYS_MIN_A, ZC_HYS_FRAC * fmaxf(irms, 0.1f));
  collectZeroCrossings_(sigClean, n, zcHysBase, crossAll, 128, crossAllN, crossPos, 64, crossPosN);
  if ((crossAllN < 3 || crossPosN < 2) && irms <= ZC_HYS_LOWCURRENT_RETRY_MAX_A) {
    const float retryHys = fmaxf(ZC_HYS_MIN_A * ZC_HYS_LOWCURRENT_RETRY_SCALE,
                                 zcHysBase * ZC_HYS_LOWCURRENT_RETRY_SCALE);
    collectZeroCrossings_(sigClean, n, retryHys, crossAll, 128, crossAllN, crossPos, 64, crossPosN);
  }

  if (crossAllN < 3) {
    out.feat_valid = false;
    return true;
  }

  if (crossPosN >= 3) {
    out.zcv = robustIntervalJitterMs_(crossPos, crossPosN, fs_hz);
  } else if (crossAllN >= 4) {
    out.zcv = robustIntervalJitterMs_(crossAll, crossAllN, fs_hz);
  }


  const int cycleCount = crossPosN - 1;
  if (cycleCount <= 0) {
    out.feat_valid = false;
    return true;
  }

  float cyclePeaks[24];
  float cycleRmsVals[24];
  int peakN = 0;
  float nmsePairsVals[24];
  int nmsePairs = 0;

  static constexpr int RSZ = 64;
  float prevCycle[RSZ];
  int prevCycleLen = 0;
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

    const float cycRms = (len > 0) ? sqrtf((float)(cycSq / (double)len)) : 0.0f;
    if (cycRms <= 1e-5f) continue;

    cyclePeaks[peakN] = peak;
    cycleRmsVals[peakN] = cycRms;
    peakN++;

    float curCycle[RSZ];
    for (int j = 0; j < RSZ; ++j) {
      const float pos = (float)a + ((float)j * (float)(len - 1) / (float)(RSZ - 1));
      const int i0 = clampi((int)floorf(pos), a, b - 1);
      const int i1 = clampi(i0 + 1, a, b - 1);
      const float frac = pos - (float)i0;
      curCycle[j] = (sigClean[i0] + frac * (sigClean[i1] - sigClean[i0])) / cycRms;
    }

    if (havePrevCycle) {
      const int lenDelta = abs(len - prevCycleLen);
      const int lenLimit = clampi((int)(0.12f * (float)prevCycleLen), 3, 24);
      if (lenDelta <= lenLimit) {
        float bestRatio = INFINITY;
        for (int shift = -2; shift <= 2; ++shift) {
          double mse = 0.0;
          double eRef = 0.0;
          int aligned = 0;
          for (int j = 0; j < RSZ; ++j) {
            const int pj = j + shift;
            if (pj < 0 || pj >= RSZ) continue;
            const double cur = (double)curCycle[j];
            const double prv = (double)prevCycle[pj];
            const double d = cur - prv;
            mse += d * d;
            eRef += 0.5 * (cur * cur + prv * prv);
            aligned++;
          }
          if (aligned < (RSZ - 4)) continue;
          const float ratio = clampf((float)(mse / (eRef + 1e-9)), 0.0f, 1.5f);
          if (ratio < bestRatio) bestRatio = ratio;
        }
        if (isfinite(bestRatio) && nmsePairs < 24) nmsePairsVals[nmsePairs++] = bestRatio;
      }
    }

    memcpy(prevCycle, curCycle, sizeof(prevCycle));
    prevCycleLen = len;
    havePrevCycle = true;
  }

  double posHalfSq = 0.0;
  double negHalfSq = 0.0;
  uint32_t posHalfN = 0;
  uint32_t negHalfN = 0;
  for (size_t i = 0; i < n; ++i) {
    const float v = sigClean[i];
    if (v >= 0.0f) {
      posHalfSq += (double)v * (double)v;
      posHalfN++;
    } else {
      negHalfSq += (double)v * (double)v;
      negHalfN++;
    }
  }
  if (posHalfN > 8 && negHalfN > 8) {
    const float posRms = sqrtf((float)(posHalfSq / (double)posHalfN));
    const float negRms = sqrtf((float)(negHalfSq / (double)negHalfN));
    const float denom = fmaxf(0.01f, 0.5f * (posRms + negRms));
    out.halfcycle_asymmetry = clampf((fabsf(posRms - negRms) / denom) * FEATURE_PERCENT_SCALE, 0.0f, 200.0f);
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
    out.peak_fluct_cv = (float)(sqrt(vv) / (mu + 1e-9)) * FEATURE_PERCENT_SCALE;
  }

  if (nmsePairs > 0) {
    for (int i = 1; i < nmsePairs; ++i) {
      const float key = nmsePairsVals[i];
      int j = i - 1;
      while (j >= 0 && nmsePairsVals[j] > key) {
        nmsePairsVals[j + 1] = nmsePairsVals[j];
        --j;
      }
      nmsePairsVals[j + 1] = key;
    }
    const float medianNmse = nmsePairsVals[nmsePairs / 2];
    out.cycle_nmse = clampf(medianNmse * FEATURE_PERCENT_SCALE, 0.0f, 100.0f);
  }

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

  fundP = bandPeakExcess(sig, half, k1, 1, 4);
  const double avgNoise = pTotNoDc / (double)(half - 1);
  const double snr = (avgNoise > 0.0) ? ((double)fundP / avgNoise) : 0.0;
  const bool haveFund = (fundP >= FUND_MAG_MIN) && (snr >= FUND_SNR_MIN);
  const float trackedFundP = goertzelTonePower_(sigFilt, n, fs_hz, mainsHz);
  const float trackedFundGate = fmaxf(FUND_MAG_MIN, (float)avgNoise * TRACKED_FUND_SNR_MIN);
  const bool haveTrackedFund = (trackedFundP >= trackedFundGate);

  if (!haveFund && !haveTrackedFund && irms < FEATURE_REQUIRE_FUND_BELOW_A) {
    out.feat_valid = false;
    return true;
  }

  if (haveFund || haveTrackedFund || trackedFundP > 1e-9f) {
    double harmP = 0.0;
    const float nyquist = 0.5f * fs_hz;
    for (int h = 2; h <= 10; ++h) {
      const float harmHz = float(h) * mainsHz;
      if (harmHz >= (nyquist - 5.0f)) break;
      harmP += (double)goertzelTonePower_(sigFilt, n, fs_hz, harmHz);
    }
    const float thdFundP = fmaxf(trackedFundP, fundP);
    out.thd_i = (thdFundP > 1e-12f)
        ? clampf((float)(sqrt(harmP / (double)thdFundP) * 100.0), DB_THD_CLIP_MIN, DB_THD_CLIP_MAX)
        : 0.0f;
  } else {
    out.thd_i = 0.0f;
  }

  memset(harmonicSkip, 0, sizeof(harmonicSkip));
  if (haveFund && k1 > 0) {
    for (int h = 1; h <= 24; ++h) {
      const int kh = h * k1;
      if (kh >= half) break;
      const int a = clampi(kh - SPECTRAL_FLUX_HARM_SKIP_BINS, 1, half - 1);
      const int b = clampi(kh + SPECTRAL_FLUX_HARM_SKIP_BINS, 1, half - 1);
      for (int k = a; k <= b; ++k) harmonicSkip[k] = 1;
    }
  }

  int kUm0 = 0, kUm1 = 0, kHf0 = 0, kHf1 = 0;
  int kMb0 = 0, kMb1 = 0;
  int kFlux0 = 0, kFlux1 = 0;
  double pUM = 0.0;
  double pHF = 0.0;
  double pResidual = 0.0;
  double fluxMagSum = 0.0;

  const bool haveUpperMidBand = makeBandBins(UPPERMID_LO_HZ, UPPERMID_HI_HZ, binHz, half, kUm0, kUm1);
  const bool haveHfBand = makeBandBins(HF_BAND_LO_HZ, HF_BAND_HI_HZ, binHz, half, kHf0, kHf1);
  const bool haveMidBand = makeBandBins(MIDBAND_LO_HZ, MIDBAND_HI_HZ, binHz, half, kMb0, kMb1);
  const bool haveFluxBand = makeBandBins(SPECTRAL_FLUX_LO_HZ, SPECTRAL_FLUX_HI_HZ, binHz, half, kFlux0, kFlux1);

  for (int k = 1; k < half; ++k) {
    const double p = (double)sig[k];
    if (haveUpperMidBand && k >= kUm0 && k <= kUm1) pUM += p;
    if (haveHfBand && k >= kHf0 && k <= kHf1) pHF += p;

    if (harmonicSkip[k]) continue;
    if (haveMidBand && k >= kMb0 && k <= kMb1) pResidual += p;
    if (haveFluxBand && k >= kFlux0 && k <= kFlux1) fluxMagSum += sqrt((double)fmaxf(sig[k], 0.0f));
  }

  const float hfBandEnergyRatio = (float)(pHF / (pHF + pUM + 1e-12));
  const float midbandResidualRms = haveMidBand
      ? (float)(sqrt(pResidual + 1e-12) / (double)n)
      : 0.0f;

  float residPeak = 0.0f;
  for (size_t i = 0; i < n; ++i) residPeak = fmaxf(residPeak, fabsf(resid[i]));
  const float residualCrestLinear = (residRms > 1e-6f)
      ? clampf(residPeak / (residRms + 1e-6f), 0.0f, 100.0f)
      : 0.0f;
  out.residual_crest_factor = clampf(
      ratio_to_db20(residualCrestLinear),
      -20.0f,
      40.0f);

  if (haveFluxBand && fluxMagSum > 1e-12) {
    double fluxAcc = 0.0;
    for (int k = kFlux0; k <= kFlux1; ++k) {
      float cur = 0.0f;
      if (!harmonicSkip[k]) {
        cur = (float)(sqrt((double)fmaxf(sig[k], 0.0f)) / fluxMagSum);
      }
      if (s_havePrevFluxNorm) fluxAcc += fabs((double)cur - (double)s_prevFluxNorm[k]);
      s_prevFluxNorm[k] = cur;
    }
    out.spectral_flux_midhf = s_havePrevFluxNorm ? clampf((float)(0.5 * fluxAcc) * FEATURE_PERCENT_SCALE, 0.0f, 200.0f) : 0.0f;
    s_havePrevFluxNorm = true;
  } else {
    out.spectral_flux_midhf = 0.0f;
  }

  const uint32_t nowMs = millis();
  const float baselineIrms = s_baseline.baselineIrms();
  const float baselineStd = s_baseline.baselineStd();
  const float baselineHf = s_baseline.baselineHf();

  const int edgeWin = clampi((int)lroundf((EDGE_SPIKE_WINDOW_MS * 1e-3f) * fs_hz), 4, 24);
  int strongestEdgeIdx = 1;
  float strongestEdge = 0.0f;
  for (size_t i = 1; i < n; ++i) {
    const float d = fabsf(sigClean[i] - sigClean[i - 1]);
    if (d > strongestEdge) {
      strongestEdge = d;
      strongestEdgeIdx = (int)i;
    }
  }
  float edgeBurst = 0.0f;
  const int edgeA = clampi(strongestEdgeIdx - edgeWin, 0, (int)n - 1);
  const int edgeB = clampi(strongestEdgeIdx + edgeWin, 0, (int)n - 1);
  for (int i = edgeA; i <= edgeB; ++i) edgeBurst = fmaxf(edgeBurst, fabsf(resid[i]));
  const float edgeSpikeLinear = clampf(edgeBurst / fmaxf(baselineIrms, EDGE_SPIKE_MIN_BASELINE_A), 0.0f, 10.0f);
  out.edge_spike_ratio = clampf(
      ratio_to_db20(edgeSpikeLinear),
      DB_RATIO_CLIP_MIN,
      DB_RATIO_CLIP_MAX);

  const float midbandResidualLinear = clampf(
      midbandResidualRms / fmaxf(baselineIrms, EDGE_SPIKE_MIN_BASELINE_A),
      0.0f,
      10.0f);
  out.midband_residual_ratio = clampf(
      ratio_to_db20(midbandResidualLinear),
      DB_RATIO_CLIP_MIN,
      DB_RATIO_CLIP_MAX);

  const float hfBaselineRef = fmaxf(baselineHf, HF_DELTA_MIN_BASELINE_SHARE);
  const float hfEnergyRatio = (hfBandEnergyRatio + DB_POWER_RATIO_EPS) / (hfBaselineRef + DB_POWER_RATIO_EPS);
  out.hf_energy_delta = clampf(
      ratio_to_db10(hfEnergyRatio),
      DB_HF_DELTA_CLIP_MIN,
      DB_HF_DELTA_CLIP_MAX);

  const bool cycleNmseSoloArtifact =
      (out.cycle_nmse >= CYCLE_NMSE_SOLO_ARTIFACT_MIN_PCT) &&
      (out.irms_a <= CYCLE_NMSE_SOLO_ARTIFACT_MAX_IRMS_A) &&
      (out.spectral_flux_midhf <= CYCLE_NMSE_SOLO_ARTIFACT_MAX_FLUX_PCT) &&
      (out.residual_crest_factor <= CYCLE_NMSE_SOLO_ARTIFACT_MAX_RESIDUAL_CF_DB) &&
      (out.edge_spike_ratio <= CYCLE_NMSE_SOLO_ARTIFACT_MAX_EDGE_DB) &&
      (out.midband_residual_ratio <= CYCLE_NMSE_SOLO_ARTIFACT_MAX_MIDBAND_DB) &&
      (out.peak_fluct_cv <= CYCLE_NMSE_SOLO_ARTIFACT_MAX_PEAK_CV_PCT) &&
      (fabsf(out.hf_energy_delta) <= CYCLE_NMSE_SOLO_ARTIFACT_MAX_HF_DELTA_DB);
  if (cycleNmseSoloArtifact) out.cycle_nmse = CYCLE_NMSE_SOLO_ARTIFACT_REPLACEMENT_PCT;

  out.abs_irms_zscore_vs_baseline = clampf(fabsf(out.irms_a - baselineIrms) / (baselineStd + 1e-6f), 0.0f, 25.0f);

  const bool baselineStep = s_baseline.initialized &&
      (fabsf(out.irms_a - s_baseline.irmsMean) >= fmaxf(BASELINE_STEP_FREEZE_A, BASELINE_STEP_FREEZE_FRAC * fmaxf(s_baseline.irmsMean, out.irms_a)));
  const bool suspectedArcLike =
      (out.spectral_flux_midhf >= ARC_SIG_SPECTRAL_FLUX) ||
      (out.residual_crest_factor >= ARC_SIG_RESIDUAL_CF) ||
      (out.edge_spike_ratio >= ARC_SIG_EDGE_SPIKE_RATIO) ||
      (out.midband_residual_ratio >= ARC_SIG_MIDBAND_RATIO) ||
      (out.cycle_nmse >= ARC_SIG_CYCLE_NMSE) ||
      (out.peak_fluct_cv >= ARC_SIG_PEAK_FLUCT) ||
      (out.hf_energy_delta >= ARC_SIG_HF_ENERGY_DELTA) ||
      (out.abs_irms_zscore_vs_baseline >= ARC_SIG_IRMS_ZSCORE);

  if (baselineStep || suspectedArcLike) s_baseline.freezeUntilMs = nowMs + BASELINE_FREEZE_MS;

  const bool baselineFrozen = ((int32_t)(s_baseline.freezeUntilMs - nowMs) > 0);
  const bool stableForBaseline =
      out.current_valid &&
      (out.irms_a >= BASELINE_MIN_IRMS_A) &&
      !baselineFrozen &&
      (out.spectral_flux_midhf < 4.5f) &&
      (out.residual_crest_factor < BASELINE_STABLE_RESIDUAL_CF_DB) &&
      (out.edge_spike_ratio < BASELINE_STABLE_EDGE_SPIKE_DB) &&
      (out.midband_residual_ratio < BASELINE_STABLE_MIDBAND_RATIO_DB) &&
      (out.cycle_nmse < 5.5f) &&
      (out.peak_fluct_cv < 1.0f) &&
      (fabsf(out.hf_energy_delta) < BASELINE_STABLE_HF_DELTA_DB) &&
      (out.abs_irms_zscore_vs_baseline < 1.5f);

  if (out.irms_a <= BASELINE_RESET_IRMS_A) {
    if (s_baseline.idleSinceMs == 0) s_baseline.idleSinceMs = nowMs;
    if ((nowMs - s_baseline.idleSinceMs) >= BASELINE_RESET_IDLE_MS) s_baseline.reset();
  } else {
    s_baseline.idleSinceMs = 0;
  }

  if (stableForBaseline) {
    if (!s_baseline.initialized) {
      s_baseline.initialized = true;
      s_baseline.irmsMean = out.irms_a;
      s_baseline.irmsVar = powf(fmaxf(BASELINE_STD_FLOOR_A, BASELINE_STD_FLOOR_FRAC * out.irms_a), 2.0f);
      s_baseline.hfMean = hfBandEnergyRatio;
    } else {
      const float prevMean = s_baseline.irmsMean;
      s_baseline.irmsMean += BASELINE_IRMS_ALPHA * (out.irms_a - s_baseline.irmsMean);
      const float err = out.irms_a - prevMean;
      s_baseline.irmsVar = (1.0f - BASELINE_IRMS_ALPHA) * s_baseline.irmsVar + BASELINE_IRMS_ALPHA * err * err;
      s_baseline.hfMean += BASELINE_HF_ALPHA * (hfBandEnergyRatio - s_baseline.hfMean);
    }
  }
  s_baseline.lastIrms = out.irms_a;

  sanitizeArcDetectionResult_(out);
  out.feat_valid = true;
  return true;
}

static inline int arcModelBaseFeatureCount_() {
#if defined(ARC_MODEL_FEATURE_VERSION) && (ARC_MODEL_FEATURE_VERSION >= 6) && defined(ARC_MODEL_BASE_FEATURE_COUNT)
  return ARC_MODEL_BASE_FEATURE_COUNT;
#elif defined(ARC_MODEL_FEATURE_VERSION) && (ARC_MODEL_FEATURE_VERSION >= 5)
  const int baseCount = ARC_MODEL_INPUT_DIM - 7;
  if (baseCount == 10 || baseCount == 12 || baseCount == 14 || baseCount == 16) return baseCount;
#endif
  return 10;
}


struct ArcModelFeatureSnapshot {
  float spectral_flux_midhf;
  float residual_crest_factor;
  float edge_spike_ratio;
  float midband_residual_ratio;
  float cycle_nmse;
  float peak_fluct_cv;
  float thd_i;
  float hf_energy_delta;
  float zcv;
  float abs_irms_zscore_vs_baseline;
  float suspicious_run_energy;
  float delta_irms_abs;
  float delta_hf_energy;
  float delta_flux;
  float halfcycle_asymmetry;
  float v_sag_pct;
  int8_t ctxFamily;
  float ctxConfidence;
};

static inline float arcFeatureValueById_(const ArcModelFeatureSnapshot& x, int featureId) {
  switch (featureId) {
    case TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE:
      return tinymlClampFeatureValue(featureId, x.abs_irms_zscore_vs_baseline);
    case TINYML_FEATURE_DELTA_IRMS_ABS:
      return tinymlClampFeatureValue(featureId, x.delta_irms_abs);
    case TINYML_FEATURE_HALFCYCLE_ASYMMETRY:
      return tinymlClampFeatureValue(featureId, x.halfcycle_asymmetry);
    case TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY:
      return tinymlClampFeatureValue(featureId, x.suspicious_run_energy);
    case TINYML_FEATURE_DELTA_HF_ENERGY:
      return tinymlClampFeatureValue(featureId, x.delta_hf_energy);
    case TINYML_FEATURE_DELTA_FLUX:
      return tinymlClampFeatureValue(featureId, x.delta_flux);
    case TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO:
      return tinymlClampFeatureValue(featureId, x.midband_residual_ratio);
    case TINYML_FEATURE_ZCV:
      return tinymlClampFeatureValue(featureId, x.zcv);
    case TINYML_FEATURE_SPECTRAL_FLUX_MIDHF:
      return tinymlClampFeatureValue(featureId, x.spectral_flux_midhf);
    case TINYML_FEATURE_PEAK_FLUCT_CV:
      return tinymlClampFeatureValue(featureId, x.peak_fluct_cv);
    case TINYML_FEATURE_RESIDUAL_CREST_FACTOR:
      return tinymlClampFeatureValue(featureId, x.residual_crest_factor);
    case TINYML_FEATURE_THD_I:
      return tinymlClampFeatureValue(featureId, x.thd_i);
    case TINYML_FEATURE_HF_ENERGY_DELTA:
      return tinymlClampFeatureValue(featureId, x.hf_energy_delta);
    case TINYML_FEATURE_EDGE_SPIKE_RATIO:
      return tinymlClampFeatureValue(featureId, x.edge_spike_ratio);
    case TINYML_FEATURE_V_SAG_PCT:
      return tinymlClampFeatureValue(featureId, x.v_sag_pct);
    case TINYML_FEATURE_CYCLE_NMSE:
      return tinymlClampFeatureValue(featureId, x.cycle_nmse);
    case TINYML_FEATURE_CTX_FAMILY_RESISTIVE_LINEAR: return (x.ctxFamily == FAMILY_RESISTIVE_LINEAR) ? 1.0f : 0.0f;
    case TINYML_FEATURE_CTX_FAMILY_INDUCTIVE_MOTOR: return (x.ctxFamily == FAMILY_INDUCTIVE_MOTOR) ? 1.0f : 0.0f;
    case TINYML_FEATURE_CTX_FAMILY_RECTIFIER_SMPS: return (x.ctxFamily == FAMILY_RECTIFIER_SMPS) ? 1.0f : 0.0f;
    case TINYML_FEATURE_CTX_FAMILY_PHASE_ANGLE_CONTROLLED: return (x.ctxFamily == FAMILY_PHASE_ANGLE_CONTROLLED) ? 1.0f : 0.0f;
    case TINYML_FEATURE_CTX_FAMILY_BRUSH_UNIVERSAL_MOTOR: return (x.ctxFamily == FAMILY_BRUSH_UNIVERSAL_MOTOR) ? 1.0f : 0.0f;
    case TINYML_FEATURE_CTX_FAMILY_OTHER_MIXED: return (x.ctxFamily == FAMILY_OTHER_MIXED) ? 1.0f : 0.0f;
    case TINYML_FEATURE_CONTEXT_FAMILY_CONFIDENCE:
      return tinymlClampFeatureValue(featureId, x.ctxConfidence);
    default: return 0.0f;
  }
}

static inline float arcPositiveThresholdById_(int featureId) {
  switch (featureId) {
    case TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE: return ARC_SIG_IRMS_ZSCORE;
    case TINYML_FEATURE_DELTA_IRMS_ABS: return 0.12f;
    case TINYML_FEATURE_HALFCYCLE_ASYMMETRY: return 10.0f;
    case TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY: return 1.60f;
    case TINYML_FEATURE_DELTA_HF_ENERGY: return 0.70f;
    case TINYML_FEATURE_DELTA_FLUX: return 4.00f;
    case TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO: return ARC_SIG_MIDBAND_RATIO;
    case TINYML_FEATURE_ZCV: return ARC_SIG_ZCV;
    case TINYML_FEATURE_SPECTRAL_FLUX_MIDHF: return ARC_SIG_SPECTRAL_FLUX;
    case TINYML_FEATURE_PEAK_FLUCT_CV: return ARC_SIG_PEAK_FLUCT;
    case TINYML_FEATURE_RESIDUAL_CREST_FACTOR: return ARC_SIG_RESIDUAL_CF;
    case TINYML_FEATURE_THD_I: return ARC_SIG_THD_I;
    case TINYML_FEATURE_HF_ENERGY_DELTA: return ARC_SIG_HF_ENERGY_DELTA;
    case TINYML_FEATURE_EDGE_SPIKE_RATIO: return ARC_SIG_EDGE_SPIKE_RATIO;
    case TINYML_FEATURE_V_SAG_PCT: return 3.0f;
    case TINYML_FEATURE_CYCLE_NMSE: return ARC_SIG_CYCLE_NMSE;
    default: return INFINITY;
  }
}


static inline void fillArcModelInput_(double* dst,
                                     float spectral_flux_midhf,
                                     float residual_crest_factor,
                                     float edge_spike_ratio,
                                     float midband_residual_ratio,
                                     float cycle_nmse,
                                     float peak_fluct_cv,
                                     float thd_i,
                                     float hf_energy_delta,
                                     float zcv,
                                     float abs_irms_zscore_vs_baseline,
                                     float suspicious_run_energy,
                                     float delta_irms_abs,
                                     float delta_hf_energy,
                                     float delta_flux,
                                     float halfcycle_asymmetry,
                                     float v_sag_pct,
                                     int8_t ctxFamily,
                                     float ctxConfidence) {
  for (int i = 0; i < ARC_MODEL_INPUT_DIM; ++i) dst[i] = 0.0;
  const ArcModelFeatureSnapshot snapshot = {
    spectral_flux_midhf,
    residual_crest_factor,
    edge_spike_ratio,
    midband_residual_ratio,
    cycle_nmse,
    peak_fluct_cv,
    thd_i,
    hf_energy_delta,
    zcv,
    abs_irms_zscore_vs_baseline,
    suspicious_run_energy,
    delta_irms_abs,
    delta_hf_energy,
    delta_flux,
    halfcycle_asymmetry,
    v_sag_pct,
    ctxFamily,
    ctxConfidence,
  };

#if defined(ARC_MODEL_FEATURE_VERSION) && (ARC_MODEL_FEATURE_VERSION >= 6) && defined(ARC_MODEL_BASE_FEATURE_COUNT)
  for (int i = 0; i < ARC_MODEL_INPUT_DIM; ++i) {
    dst[i] = (double)arcFeatureValueById_(snapshot, arc_model_input_feature_ids[i]);
  }
  return;
#else
  const int baseCount = arcModelBaseFeatureCount_();

  if (baseCount == 10) {
    const float legacyBase[10] = {
      spectral_flux_midhf,
      residual_crest_factor,
      edge_spike_ratio,
      midband_residual_ratio,
      cycle_nmse,
      peak_fluct_cv,
      thd_i,
      hf_energy_delta,
      zcv,
      abs_irms_zscore_vs_baseline
    };
    for (int i = 0; i < 10; ++i) dst[i] = (double)legacyBase[i];
  } else {
    const float rankedBase[16] = {
      abs_irms_zscore_vs_baseline,
      delta_irms_abs,
      halfcycle_asymmetry,
      suspicious_run_energy,
      delta_hf_energy,
      delta_flux,
      midband_residual_ratio,
      zcv,
      spectral_flux_midhf,
      peak_fluct_cv,
      residual_crest_factor,
      thd_i,
      hf_energy_delta,
      edge_spike_ratio,
      v_sag_pct,
      cycle_nmse
    };
    const int usableBase = (baseCount < 16) ? baseCount : 16;
    for (int i = 0; i < usableBase; ++i) dst[i] = (double)rankedBase[i];
  }

  if (ctxFamily >= 0 && ctxFamily < FAMILY_COUNT) dst[baseCount + ctxFamily] = 1.0;
  dst[baseCount + FAMILY_COUNT] = (double)clampf(ctxConfidence, 0.0f, 1.0f);
#endif
}

static inline float contextAwareArcThresholdFor_(int8_t ctxFamily, float ctxConfidence) {
#if defined(ARC_CONTEXT_CONFIDENCE_MIN) && defined(ARC_THRESHOLD_UNKNOWN)
  if (ctxFamily >= 0 && ctxFamily < FAMILY_COUNT && ctxConfidence >= ARC_CONTEXT_CONFIDENCE_MIN) {
  #if defined(arc_family_thresholds)
    return arc_family_thresholds[ctxFamily];
  #else
    return arc_context_threshold_for_family((int)ctxFamily, ctxConfidence);
  #endif
  }
  return ARC_THRESHOLD_UNKNOWN;
#else
  return ARC_THRESHOLD;
#endif
}

static inline float contextAwareArcThreshold_() {
  return contextAwareArcThresholdFor_(s_ctxFamily, s_ctxConfidence);
}


static inline int unknownContextArcVotes_(float spectral_flux_midhf,
                                          float residual_crest_factor,
                                          float edge_spike_ratio,
                                          float midband_residual_ratio,
                                          float cycle_nmse,
                                          float peak_fluct_cv,
                                          float thd_i,
                                          float hf_energy_delta,
                                          float zcv,
                                          float abs_irms_zscore_vs_baseline,
                                          float suspicious_run_energy,
                                          float delta_irms_abs,
                                          float delta_hf_energy,
                                          float delta_flux,
                                          float halfcycle_asymmetry,
                                          float v_sag_pct) {
  const ArcModelFeatureSnapshot snapshot = {
    spectral_flux_midhf,
    residual_crest_factor,
    edge_spike_ratio,
    midband_residual_ratio,
    cycle_nmse,
    peak_fluct_cv,
    thd_i,
    hf_energy_delta,
    zcv,
    abs_irms_zscore_vs_baseline,
    suspicious_run_energy,
    delta_irms_abs,
    delta_hf_energy,
    delta_flux,
    halfcycle_asymmetry,
    v_sag_pct,
    CONTEXT_FAMILY_UNKNOWN,
    0.0f,
  };
#if defined(ARC_MODEL_FEATURE_VERSION) && (ARC_MODEL_FEATURE_VERSION >= 6) && defined(ARC_MODEL_BASE_FEATURE_COUNT)
  int votes = 0;
  for (int i = 0; i < ARC_MODEL_BASE_FEATURE_COUNT; ++i) {
    const int featureId = arc_model_base_feature_ids[i];
    const float threshold = arcPositiveThresholdById_(featureId);
    if (isfinite(threshold) && arcFeatureValueById_(snapshot, featureId) >= threshold) votes++;
  }
  return votes;
#else
  const int baseCount = arcModelBaseFeatureCount_();
  int votes = 0;

  if (baseCount == 10) {
    const float legacyBase[10] = {
      spectral_flux_midhf,
      residual_crest_factor,
      edge_spike_ratio,
      midband_residual_ratio,
      cycle_nmse,
      peak_fluct_cv,
      thd_i,
      hf_energy_delta,
      zcv,
      abs_irms_zscore_vs_baseline
    };
    const float legacyThresholds[10] = {
      ARC_SIG_SPECTRAL_FLUX,
      ARC_SIG_RESIDUAL_CF,
      ARC_SIG_EDGE_SPIKE_RATIO,
      ARC_SIG_MIDBAND_RATIO,
      ARC_SIG_CYCLE_NMSE,
      ARC_SIG_PEAK_FLUCT,
      ARC_SIG_THD_I,
      ARC_SIG_HF_ENERGY_DELTA,
      ARC_SIG_ZCV,
      ARC_SIG_IRMS_ZSCORE
    };
    for (int i = 0; i < 10; ++i) {
      if (legacyBase[i] >= legacyThresholds[i]) votes++;
    }
    return votes;
  }

  const float rankedBase[16] = {
    abs_irms_zscore_vs_baseline,
    delta_irms_abs,
    halfcycle_asymmetry,
    suspicious_run_energy,
    delta_hf_energy,
    delta_flux,
    midband_residual_ratio,
    zcv,
    spectral_flux_midhf,
    peak_fluct_cv,
    residual_crest_factor,
    thd_i,
    hf_energy_delta,
    edge_spike_ratio,
    v_sag_pct,
    cycle_nmse
  };
  const float rankedThresholds[16] = {
    ARC_SIG_IRMS_ZSCORE,
    0.12f,
    10.0f,
    1.60f,
    0.70f,
    4.00f,
    ARC_SIG_MIDBAND_RATIO,
    ARC_SIG_ZCV,
    ARC_SIG_SPECTRAL_FLUX,
    ARC_SIG_PEAK_FLUCT,
    ARC_SIG_RESIDUAL_CF,
    ARC_SIG_THD_I,
    ARC_SIG_HF_ENERGY_DELTA,
    ARC_SIG_EDGE_SPIKE_RATIO,
    3.0f,
    ARC_SIG_CYCLE_NMSE
  };
  const int usableBase = (baseCount < 16) ? baseCount : 16;
  for (int i = 0; i < usableBase; ++i) {
    if (rankedBase[i] >= rankedThresholds[i]) votes++;
  }
  return votes;
#endif
}

int ArcDetection::predictWithContext(float spectral_flux_midhf, float residual_crest_factor,
                                     float edge_spike_ratio, float midband_residual_ratio,
                                     float cycle_nmse, float peak_fluct_cv,
                                     float thd_i, float hf_energy_delta,
                                     float zcv, float abs_irms_zscore_vs_baseline,
                                     float suspicious_run_energy, float delta_irms_abs,
                                     float delta_hf_energy, float delta_flux,
                                     float halfcycle_asymmetry, float v_sag_pct,
                                     float v_rms, float i_rms, float temp_c,
                                     int8_t ctxFamily, float ctxConfidence) const {
#if defined(ARC_MODEL_FEATURE_VERSION) && (ARC_MODEL_FEATURE_VERSION >= 5)
  (void)v_rms; (void)i_rms; (void)temp_c;
  double input_features[ARC_MODEL_INPUT_DIM];
  fillArcModelInput_(input_features,
                     spectral_flux_midhf,
                     residual_crest_factor,
                     edge_spike_ratio,
                     midband_residual_ratio,
                     cycle_nmse,
                     peak_fluct_cv,
                     thd_i,
                     hf_energy_delta,
                     zcv,
                     abs_irms_zscore_vs_baseline,
                     suspicious_run_energy,
                     delta_irms_abs,
                     delta_hf_energy,
                     delta_flux,
                     halfcycle_asymmetry,
                     v_sag_pct,
                     ctxFamily,
                     ctxConfidence);
  double output_probs[2] = {0.0, 0.0};
  arc_rf_predict(input_features, output_probs);
  const float threshold = contextAwareArcThresholdFor_(ctxFamily, ctxConfidence);
  const bool unknown_ctx = !((ctxFamily >= 0) && (ctxFamily < FAMILY_COUNT) && (ctxConfidence >= ARC_CONTEXT_CONFIDENCE_MIN));
  if (unknown_ctx) {
    const int votes = unknownContextArcVotes_(spectral_flux_midhf,
                                              residual_crest_factor,
                                              edge_spike_ratio,
                                              midband_residual_ratio,
                                              cycle_nmse,
                                              peak_fluct_cv,
                                              thd_i,
                                              hf_energy_delta,
                                              zcv,
                                              abs_irms_zscore_vs_baseline,
                                              suspicious_run_energy,
                                              delta_irms_abs,
                                              delta_hf_energy,
                                              delta_flux,
                                              halfcycle_asymmetry,
                                              v_sag_pct);
    if (votes < ARC_UNKNOWN_MIN_FEATURE_VOTES) return 0;
  }
  return (output_probs[1] >= threshold) ? 1 : 0;
#else
  (void)v_rms; (void)i_rms; (void)temp_c;
  float score = 0.0f;
  if (spectral_flux_midhf >= ARC_SIG_SPECTRAL_FLUX) score += 1.2f;
  if (residual_crest_factor >= ARC_SIG_RESIDUAL_CF) score += 0.8f;
  if (edge_spike_ratio >= ARC_SIG_EDGE_SPIKE_RATIO) score += 0.9f;
  if (midband_residual_ratio >= ARC_SIG_MIDBAND_RATIO) score += 0.9f;
  if (peak_fluct_cv >= ARC_SIG_PEAK_FLUCT) score += 0.5f;
  if (thd_i >= ARC_SIG_THD_I) score += 0.5f;
  if (hf_energy_delta >= ARC_SIG_HF_ENERGY_DELTA) score += 0.7f;
  if (zcv >= ARC_SIG_ZCV) score += 0.5f;
  if (abs_irms_zscore_vs_baseline >= ARC_SIG_IRMS_ZSCORE) score += 1.0f;
  if (delta_irms_abs >= 0.12f) score += 0.8f;
  if (delta_hf_energy >= 0.70f) score += 0.8f;
  if (delta_flux >= 4.0f) score += 0.7f;
  if (halfcycle_asymmetry >= 10.0f) score += 0.9f;
  return (score >= 4.0f) ? 1 : 0;
#endif
}

int ArcDetection::predict(float spectral_flux_midhf, float residual_crest_factor,
                          float edge_spike_ratio, float midband_residual_ratio,
                          float cycle_nmse, float peak_fluct_cv,
                          float thd_i, float hf_energy_delta,
                          float zcv, float abs_irms_zscore_vs_baseline,
                          float suspicious_run_energy, float delta_irms_abs,
                          float delta_hf_energy, float delta_flux,
                          float halfcycle_asymmetry, float v_sag_pct,
                          float v_rms, float i_rms, float temp_c) const {
  return predictWithContext(spectral_flux_midhf,
                            residual_crest_factor,
                            edge_spike_ratio,
                            midband_residual_ratio,
                            cycle_nmse,
                            peak_fluct_cv,
                            thd_i,
                            hf_energy_delta,
                            zcv,
                            abs_irms_zscore_vs_baseline,
                            suspicious_run_energy,
                            delta_irms_abs,
                            delta_hf_energy,
                            delta_flux,
                            halfcycle_asymmetry,
                            v_sag_pct,
                            v_rms,
                            i_rms,
                            temp_c,
                            s_ctxFamily,
                            s_ctxConfidence);
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

#if defined(ARC_MODEL_FEATURE_VERSION) && (ARC_MODEL_FEATURE_VERSION >= 5)
  double input_features[ARC_MODEL_INPUT_DIM];
  fillArcModelInput_(input_features,
                     out.spectral_flux_midhf,
                     out.residual_crest_factor,
                     out.edge_spike_ratio,
                     out.midband_residual_ratio,
                     out.cycle_nmse,
                     out.peak_fluct_cv,
                     out.thd_i,
                     out.hf_energy_delta,
                     out.zcv,
                     out.abs_irms_zscore_vs_baseline,
                     out.suspicious_run_energy,
                     out.delta_irms_abs,
                     out.delta_hf_energy,
                     out.delta_flux,
                     out.halfcycle_asymmetry,
                     out.v_sag_pct,
                     s_ctxFamily,
                     s_ctxConfidence);
  double output_probs[2] = {0.0, 0.0};
  arc_rf_predict(input_features, output_probs);
  const float threshold = contextAwareArcThreshold_();
  const bool unknown_ctx = !((s_ctxFamily >= 0) && (s_ctxFamily < FAMILY_COUNT) && (s_ctxConfidence >= ARC_CONTEXT_CONFIDENCE_MIN));
  if (unknown_ctx) {
    const int votes = unknownContextArcVotes_(out.spectral_flux_midhf,
                                              out.residual_crest_factor,
                                              out.edge_spike_ratio,
                                              out.midband_residual_ratio,
                                              out.cycle_nmse,
                                              out.peak_fluct_cv,
                                              out.thd_i,
                                              out.hf_energy_delta,
                                              out.zcv,
                                              out.abs_irms_zscore_vs_baseline,
                                              out.suspicious_run_energy,
                                              out.delta_irms_abs,
                                              out.delta_hf_energy,
                                              out.delta_flux,
                                              out.halfcycle_asymmetry,
                                              out.v_sag_pct);
    if (votes < ARC_UNKNOWN_MIN_FEATURE_VOTES) {
      out.model_pred = 0;
      return 0;
    }
  }
  out.model_pred = (output_probs[1] >= threshold) ? 1 : 0;
  return out.model_pred;
#elif defined(ARC_MODEL_FEATURE_VERSION) && (ARC_MODEL_FEATURE_VERSION >= 4)
  double input_features[10] = {
    (double)out.spectral_flux_midhf,
    (double)out.residual_crest_factor,
    (double)out.edge_spike_ratio,
    (double)out.midband_residual_ratio,
    (double)out.cycle_nmse,
    (double)out.peak_fluct_cv,
    (double)out.thd_i,
    (double)out.hf_energy_delta,
    (double)out.zcv,
    (double)out.abs_irms_zscore_vs_baseline
  };
  double output_probs[2] = {0.0, 0.0};
  arc_rf_predict(input_features, output_probs);
  out.model_pred = (output_probs[1] >= ARC_THRESHOLD) ? 1 : 0;
  return out.model_pred;
#else
  out.model_pred = (uint8_t)predict(out.spectral_flux_midhf,
                                    out.residual_crest_factor,
                                    out.edge_spike_ratio,
                                    out.midband_residual_ratio,
                                    out.cycle_nmse,
                                    out.peak_fluct_cv,
                                    out.thd_i,
                                    out.hf_energy_delta,
                                    out.zcv,
                                    out.abs_irms_zscore_vs_baseline,
                                    out.suspicious_run_energy,
                                    out.delta_irms_abs,
                                    out.delta_hf_energy,
                                    out.delta_flux,
                                    out.halfcycle_asymmetry,
                                    out.v_sag_pct,
                                    v_rms, out.irms_a, temp_c);
  return out.model_pred;
#endif
}
