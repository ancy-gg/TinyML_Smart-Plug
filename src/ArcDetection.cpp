#include "ArcDetection.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>

#include "TinyMLTreeEnsemble.h"
#include "esp_dsp.h"
#include "dsps_fft2r.h"
#include "dsps_wind_hann.h"
#include "dsp_common.h"
#include "esp_err.h"

#define CONFIG_DSP_MAX_FFT_SIZE 2048

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

void ArcDetection::resetRuntime() {
  memset(s_prevFluxNorm, 0, sizeof(s_prevFluxNorm));
  s_havePrevFluxNorm = false;
  s_baseline.reset();
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
  if (fabsf(fs_hz - FS_TARGET_HZ) <= FS_CANONICALIZE_TOL_HZ) fs_hz = FS_TARGET_HZ;
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
    out.peak_fluct_cv = (float)(sqrt(vv) / (mu + 1e-9)) * FEATURE_PERCENT_SCALE;
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
    (void)baseline;
    (void)minCycle;
  }

  if (nmsePairs > 0) out.cycle_nmse = (nmseAcc / (float)nmsePairs) * FEATURE_PERCENT_SCALE;

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

  if (!haveFund && irms < FEATURE_REQUIRE_FUND_BELOW_A) {
    out.feat_valid = false;
    return true;
  }

  if (haveFund) {
    double harmP = 0.0;
    for (int h = 2; h <= 10; ++h) {
      const int khNom = h * k1;
      if (khNom >= half) break;
      harmP += bandPeakExcess(sig, half, khNom, 1, 4);
    }
    out.thd_i = (fundP > 1e-12f) ? clampf((float)(sqrt(harmP / (double)fundP) * 100.0), DB_THD_CLIP_MIN, DB_THD_CLIP_MAX) : 0.0f;
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
      (void)bins;
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
  const float hfBandEnergyRatio = (float)(pHF / (pHF + pUM + 1e-12));

  int kMb0 = 0, kMb1 = 0;
  float midbandResidualRms = 0.0f;
  if (makeBandBins(MIDBAND_LO_HZ, MIDBAND_HI_HZ, binHz, half, kMb0, kMb1)) {
    double pResidual = 0.0;
    for (int k = kMb0; k <= kMb1; ++k) {
      if (shouldSkipHarmonicBin(k, haveFund, k1, half)) continue;
      pResidual += (double)sig[k];
    }
    midbandResidualRms = (float)(sqrt(pResidual + 1e-12) / (double)n);
  }

  float residPeak = 0.0f;
  for (size_t i = 0; i < n; ++i) residPeak = fmaxf(residPeak, fabsf(resid[i]));
  const float residualCrestLinear = (residRms > 1e-6f)
      ? clampf(residPeak / (residRms + 1e-6f), 0.0f, 100.0f)
      : 0.0f;
  out.residual_crest_factor = clampf(
      ratio_to_db20(residualCrestLinear),
      -20.0f,
      40.0f);

  double fluxMagSum = 0.0;
  int kFlux0 = 0, kFlux1 = 0;
  if (makeBandBins(SPECTRAL_FLUX_LO_HZ, SPECTRAL_FLUX_HI_HZ, binHz, half, kFlux0, kFlux1)) {
    for (int k = kFlux0; k <= kFlux1; ++k) {
      if (shouldSkipHarmonicBin(k, haveFund, k1, half)) continue;
      fluxMagSum += sqrt((double)fmaxf(sig[k], 0.0f));
    }
    if (fluxMagSum > 1e-12) {
      double fluxAcc = 0.0;
      for (int k = kFlux0; k <= kFlux1; ++k) {
        float cur = 0.0f;
        if (!shouldSkipHarmonicBin(k, haveFund, k1, half)) {
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

  out.feat_valid = true;
  return true;
}

int ArcDetection::predict(float spectral_flux_midhf, float residual_crest_factor,
                          float edge_spike_ratio, float midband_residual_ratio,
                          float cycle_nmse, float peak_fluct_cv,
                          float thd_i, float hf_energy_delta,
                          float zcv, float abs_irms_zscore_vs_baseline,
                          float v_rms, float i_rms, float temp_c) const {
  (void)v_rms; (void)i_rms; (void)temp_c;
  // Heuristic fallback operates directly in the current mixed feature space
  // where spectral flux / NMSE / CV / THD are percentages, ratio terms are in dB,
  // hf_energy_delta is a dB power-ratio delta, and zcv is in milliseconds.
  float score = 0.0f;
  if (spectral_flux_midhf >= ARC_SIG_SPECTRAL_FLUX) score += 1.3f;
  if (residual_crest_factor >= ARC_SIG_RESIDUAL_CF) score += 0.9f;
  if (edge_spike_ratio >= ARC_SIG_EDGE_SPIKE_RATIO) score += 1.4f;
  if (midband_residual_ratio >= ARC_SIG_MIDBAND_RATIO) score += 1.1f;
  if (cycle_nmse >= ARC_SIG_CYCLE_NMSE) score += 1.0f;
  if (peak_fluct_cv >= ARC_SIG_PEAK_FLUCT) score += 0.8f;
  if (thd_i >= ARC_SIG_THD_I) score += 0.4f;
  if (hf_energy_delta >= ARC_SIG_HF_ENERGY_DELTA) score += 0.8f;
  if (zcv >= ARC_SIG_ZCV) score += 0.6f;
  if (abs_irms_zscore_vs_baseline >= ARC_SIG_IRMS_ZSCORE) score += 0.8f;
  return (score >= 3.4f) ? 1 : 0;
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

#if defined(ARC_MODEL_FEATURE_VERSION) && (ARC_MODEL_FEATURE_VERSION >= 4)
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
                                    v_rms, out.irms_a, temp_c);
  return out.model_pred;
#endif
}
