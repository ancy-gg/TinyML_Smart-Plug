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

  // Keep the waveform path physically linear.
  // The cubic coefficients were fitted from RMS-vs-reference data, so applying
  // them sample-by-sample distorts low-current waveforms and can amplify idle
  // noise. Do only the sensor-domain volts->amps mapping here, then apply the
  // cubic once to the final RMS estimate later.
  return ((v_sensor - cal.offsetV) / cal.voltsPerAmp) * cal.ampsScale;
}

static inline float computeEntropyFromBands(const float* e, int n) {
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

bool ArcFeatures::compute(const uint16_t* raw, size_t n, float fs_hz,
                          const CurrentCalib& cal, float mainsHz,
                          ArcFeatOut& out) {
  out = ArcFeatOut{};
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
  for (size_t i = 1; i + 1 < n; ++i) {
    sigMed[i] = median3f(sig[i - 1], sig[i], sig[i + 1]);
  }
  sigMed[n - 1] = sig[n - 1];

  for (size_t i = 0; i < n; ++i) mean += sigMed[i];
  mean /= (double)n;

  for (size_t i = 0; i < n; ++i) sig[i] = sigMed[i] - (float)mean;

  const bool useSoftAaf = CURRENT_SOFT_AAF_ENABLE;
  if (useSoftAaf) {
    BiquadLPF softAaf;
    if (makeLowpassBiquad(fs_hz, CURRENT_SOFT_AAF_CUTOFF_HZ, CURRENT_SOFT_AAF_Q, softAaf)) {
      for (size_t i = 0; i < n; ++i) {
        sigFilt[i] = softAaf.step(sig[i]);
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

  // Mains-coherent RMS meter:
  // estimate current only from the 60 Hz fundamental and low-order harmonics,
  // which rejects broadband ADS residue that was being turned into false load current.
  double harmRmsSq = 0.0;
  int harmUsed = 0;
  for (int h = 1; h <= 15; ++h) {
    const float fh = mainsHz * (float)h;
    if (fh > 1500.0f || fh >= (0.45f * fs_hz)) break;

    double csum = 0.0;
    double ssum = 0.0;
    const float w = 2.0f * 3.14159265358979f * fh / fs_hz;
    for (size_t i = 0; i < n; ++i) {
      const float ang = w * (float)i;
      const float x = sigFilt[i];
      csum += (double)x * cosf(ang);
      ssum += (double)x * sinf(ang);
    }

    const float a = (2.0f / (float)n) * (float)csum;
    const float b = (2.0f / (float)n) * (float)ssum;
    const float peak = sqrtf(a * a + b * b);
    const float rmsH = peak * 0.70710678f;
    if (rmsH > 0.0025f) {
      harmRmsSq += (double)rmsH * (double)rmsH;
      harmUsed++;
    }
  }

  const float irmsCoherent = sqrtf((float)harmRmsSq);
  const float coherence = (irmsWide > 1e-6f) ? (irmsCoherent / irmsWide) : 0.0f;
  const float residFrac = (irmsWide > 1e-6f) ? (residRms / irmsWide) : 1.0f;

  const bool trustWideband =
      (coherence >= TRUE_RMS_MIN_COHERENCE) ||
      ((irmsCoherent >= TRUE_RMS_MIN_COHERENT_A) &&
       (coherence >= TRUE_RMS_RELAXED_COHERENCE) &&
       (residFrac <= TRUE_RMS_MAX_RESID_FRAC));

  // Use true RMS from the band-limited waveform when it is still sufficiently
  // mains-coherent. Otherwise fall back to the mains-coherent estimate so
  // broadband pickup is not turned into fake current.
  float irms = trustWideband ? irmsWide : irmsCoherent;

  if (irmsWide < CURRENT_IDLE_SUPPRESS_A && codeSpan < LOW_CURRENT_CODE_SPAN) {
    irms = 0.0f;
  } else if (!trustWideband && irmsCoherent < IRMS_GATE_OFF_A) {
    irms = 0.0f;
  } else if (trustWideband && irms < IRMS_GATE_OFF_A) {
    irms = 0.0f;
  }

  // Apply the RMS calibration in the RMS domain, not sample-by-sample.
  if (irms > 0.0f) {
    irms = eval_cubic_horner(irms, cal.cubic3, cal.cubic2, cal.cubic1, cal.cubic0);
    if (irms < 0.0f) irms = 0.0f;
  }

  // Remove the measured backend floor after calibration.
  if (CURRENT_RMS_FLOOR_SUB_A > 0.0f) {
    irms -= CURRENT_RMS_FLOOR_SUB_A;
    if (irms < 0.0f) irms = 0.0f;
  }

  out.irms_a = irms;
  out.current_valid = ((harmUsed > 0) || trustWideband) && (irms > 0.0f || codeSpan >= LOW_CURRENT_CODE_SPAN);

  if (irms < FEATURE_MIN_IRMS_A) {
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
  int peakN = 0;
  float pulsePerCycleAcc = 0.0f;
  int pulseCycles = 0;
  float nmseAcc = 0.0f;
  int nmsePairs = 0;

  static constexpr int RSZ = 96;
  float prevCycle[RSZ];
  bool havePrevCycle = false;

  const bool pulseEligible = (irms >= PULSE_ANALYSIS_MIN_IRMS_A) &&
                             (residRms >= PULSE_ANALYSIS_MIN_RESID_A);
  float pulseThr = fmaxf(PULSE_THRESH_MIN_A, PULSE_THRESH_RMS_MUL * residRms);
  pulseThr = fmaxf(pulseThr, 0.10f * irms);

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

      const bool over = pulseEligible && (fabsf(resid[i]) >= pulseThr);
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
    if (sig[k] > fundP) {
      fundP = sig[k];
      k1 = k;
    }
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
          if (abs(k - kh) <= 2) {
            skip = true;
            break;
          }
        }
      }
      if (!skip) pResidual += (double)sig[k];
    }
    out.midband_residual_rms = (float)(sqrt(pResidual + 1e-12) / (double)n);
  } else {
    out.midband_residual_rms = 0.0f;
  }

  static constexpr int WP_N = 512;
  static float wpA[WP_N];
  static float wpB[WP_N];

  int step = (int)(n / WP_N);
  if (step < 1) step = 1;
  for (int i = 0; i < WP_N; ++i) {
    const int idx = clampi(i * step, 0, (int)n - 1);
    wpA[i] = sigClean[idx];
  }

  float* in = wpA;
  float* outBuf = wpB;
  for (int level = 0; level < 3; ++level) {
    const int nodeCount = 1 << level;
    const int nodeLen = WP_N / nodeCount;
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