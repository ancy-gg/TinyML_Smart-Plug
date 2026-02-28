#include "ArcFeatures.h"
#include <math.h>

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
  const float v_aux = (float(code) * ADS_VREF_V) / 65535.0f; // 0..Vref
  const float v_sensor = v_aux / cal.dividerRatio;          // undo divider
  return (v_sensor - cal.offsetV) / cal.voltsPerAmp;
}

static inline float median3(float a, float b, float c) {
  if (a > b) { float t=a; a=b; b=t; }
  if (b > c) { float t=b; b=c; c=t; }
  if (a > b) { float t=a; a=b; b=t; }
  return b;
}

bool ArcFeatures::compute(const uint16_t* raw, size_t n, float fs_hz,
                          const CurrentCalib& cal, float mainsHz,
                          ArcFeatOut& out) {
  if (!raw || n != N_SAMP) return false;
  if (fs_hz <= 1000.0f) return false;
  if (!dsp_is_power_of_two((int)n)) return false;
  if ((int)n > CONFIG_DSP_MAX_FFT_SIZE) return false;

  out.fs_hz = fs_hz;

  static float sig[N_SAMP];        // time-domain (then reused for power spectrum)
  static float fft_cf[2 * N_SAMP]; // complex interleaved
  static float win[N_SAMP];

  static bool dspReady = false;
  static bool winReady = false;

  if (!dspReady) {
    // Init FFT tables for max size (not n). :contentReference[oaicite:3]{index=3}
    if (dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE) != ESP_OK) return false;
    dspReady = true;
  }
  if (!winReady) {
    dsps_wind_hann_f32(win, (int)n);
    winReady = true;
  }

  // ---- Convert + basic integrity checks ----
  double mean = 0.0;
  int sat = 0;
  int changes = 0;

  uint16_t prev = raw[0];
  for (size_t i = 0; i < n; i++) {
    const uint16_t c = raw[i];
    if (c < 32 || c > 65503) sat++;
    if (i && c != prev) changes++;
    prev = c;

    const float a = codeToCurrentA(c, cal);
    sig[i] = a;
    mean += a;
  }
  mean /= (double)n;

  // If you're railed or basically flatlined -> ADC frame is bad
  if (sat > 32) return false;
  if (changes < 8) return false;  // catches “stale / not converting” frames (your CSV shows these)

  // ---- Center + RMS ----
  double acc = 0.0;
  for (size_t i = 0; i < n; i++) {
    const float s = sig[i] - (float)mean;
    sig[i] = s;
    acc += (double)s * (double)s;
  }
  const float irms_raw = sqrtf((float)(acc / (double)n));

  // Median-of-3 smoothing
  static bool init=false;
  static float r0=0, r1=0, r2=0;
  if (!init) { init=true; r0=r1=r2=irms_raw; }
  else { r0=r1; r1=r2; r2=irms_raw; }
  const float irms_med = median3(r0,r1,r2);

  // ---- Idle floor learning (yours was capped at 0.03A, but you said idle is ~0.12A) ----
  static bool floorInit=false;
  static float idleFloor=0.0f;
  static uint16_t learnFrames=0;

  const float frameHz = fs_hz / (float)n;
  const uint16_t learnMinFrames = (uint16_t)fmaxf(10.0f, frameHz * 1.5f);

  static constexpr float FLOOR_LEARN_MAX_A = 0.20f; // allow learning up to ~0.12A idle
  static constexpr float FLOOR_MAX_A      = 0.18f;
  static constexpr float FLOOR_INIT_MAX_A = 0.18f;

  if (!floorInit) {
    floorInit=true;
    idleFloor = fminf(irms_med, FLOOR_INIT_MAX_A);
    learnFrames=0;
  }

  if (irms_med <= FLOOR_LEARN_MAX_A) {
    if (learnFrames < 65535) learnFrames++;
  } else {
    learnFrames = 0;
  }

  if (learnFrames >= learnMinFrames) {
    const float alphaUp = 0.01f;  // rise slow
    const float alphaDn = 0.12f;  // fall fast
    const float alpha = (irms_med > idleFloor) ? alphaUp : alphaDn;
    idleFloor = (1.0f - alpha)*idleFloor + alpha*irms_med;
    idleFloor = fminf(idleFloor, FLOOR_MAX_A);
  }

  const float irms_clean = fmaxf(0.0f, irms_med - idleFloor);
  out.irms_a = irms_clean;

  const bool lowSignal = (irms_clean < IDLE_IRMS_A);

  // ---- ZCV + cycle variance (unchanged logic) ----
  int crossings[100];
  int cCount = 0;
  const float hys = fmaxf(ZC_HYS_MIN_A, ZC_HYS_FRAC * irms_med);

  int state = 0;
  if (sig[0] >  hys) state =  1;
  if (sig[0] < -hys) state = -1;

  for (size_t i=1; i<n && cCount<100; i++) {
    const float s = sig[i];
    if (state <= 0 && s >  hys) { crossings[cCount++] = (int)i; state =  1; }
    else if (state >= 0 && s < -hys) { crossings[cCount++] = (int)i; state = -1; }
  }

  if (cCount >= 6) {
    double sum=0.0, sum2=0.0;
    int m=0;
    for (int k=1;k<cCount;k++) {
      const int dt = crossings[k]-crossings[k-1];
      const double dtMs = (double(dt)/double(fs_hz))*1000.0;
      sum += dtMs;
      sum2 += dtMs*dtMs;
      m++;
    }
    const double mu = sum/(double)m;
    const double var = (sum2/(double)m) - (mu*mu);
    out.zcv_ms = (float)sqrt(var > 0 ? var : 0);
  } else {
    out.zcv_ms = 0.0f;
  }

  out.cyc_var = 0.0f;
  if (cCount >= 8) {
    float cycRms[12];
    int cycN=0;

    for (int k=0; k+2<cCount && cycN<12; k+=2) {
      const int a = crossings[k];
      const int b = crossings[k+2];
      if (a < 0 || b <= a || b > (int)n) continue;

      double accC=0.0;
      const int len = b - a;
      for (int i=a;i<b;i++) {
        const double s = (double)sig[i];
        accC += s*s;
      }
      const float rms = (len>0) ? (float)sqrt(accC/(double)len) : 0.0f;
      cycRms[cycN++] = rms;
    }

    if (cycN >= 3) {
      double mu=0.0;
      for (int i=0;i<cycN;i++) mu += cycRms[i];
      mu /= (double)cycN;

      double var=0.0;
      for (int i=0;i<cycN;i++) {
        const double d = (double)cycRms[i]-mu;
        var += d*d;
      }
      var /= (double)cycN;

      out.cyc_var = (float)(var / (mu*mu + 1e-12));
    }
  }

  if (lowSignal) {
    out.thd_pct = 0.0f;
    out.entropy = 0.0f;
    out.hf_ratio = 0.0f;
    out.hf_var = 0.0f;
    out.sf = 0.0f;
    return true;
  }

  // ---- FFT (real signal as complex with Im=0). Do NOT call dsps_cplx2reC_fc32 here. :contentReference[oaicite:4]{index=4}
  for (size_t i=0; i<n; i++) {
    fft_cf[2*i + 0] = sig[i] * win[i];
    fft_cf[2*i + 1] = 0.0f;
  }

  if (dsps_fft2r_fc32(fft_cf, (int)n) != ESP_OK) return false;
  if (dsps_bit_rev_fc32(fft_cf, (int)n) != ESP_OK) return false;

  // Power spectrum into sig[0..n/2-1]
  const int half = (int)(n/2);
  double pSum = 0.0;
  for (int k=0; k<half; k++) {
    const float re = fft_cf[2*k + 0];
    const float im = fft_cf[2*k + 1];
    const float P  = re*re + im*im;
    sig[k] = P;
    if (k) pSum += (double)P;
  }

  const float binHz = fs_hz / (float)n;

  // Fundamental near expected bin
  int kNom = (int)lround(mainsHz / binHz);
  if (kNom < 1) kNom = 1;
  if (kNom > (half - 2)) kNom = (half - 2);

  int k1 = kNom;
  float fundP = 0.0f;
  for (int dk=-2; dk<=2; dk++) {
    int k = kNom + dk;
    if (k>=1 && k<half) {
      if (sig[k] > fundP) { fundP = sig[k]; k1 = k; }
    }
  }

  // Frame guard: reject “random noise” frames (your CSV’s 1.58A / 283% THD ones)
  const double noiseAvg = pSum / (double)(half - 1);
  const double snr = (noiseAvg > 0) ? (double)fundP / noiseAvg : 0.0;
  if (fundP < FUND_MAG_MIN || snr < FUND_SNR_MIN) return false;

  // THD 2..10 using power
  double harmP = 0.0;
  for (int h=2; h<=10; h++) {
    int khNom = h*k1;
    if (khNom >= half) break;
    float hmP = 0.0f;
    for (int dk=-1; dk<=1; dk++) {
      int kh = khNom + dk;
      if (kh>=1 && kh<half) hmP = fmaxf(hmP, sig[kh]);
    }
    harmP += (double)hmP;
  }
  out.thd_pct = (fundP > 1e-18f) ? (float)(sqrt(harmP / (double)fundP) * 100.0) : 0.0f;

  // Entropy + Spectral Flatness up to ENTROPY_MAX_HZ
  const int maxBin = (int)fminf((float)half, floorf(ENTROPY_MAX_HZ / binHz));
  double psum2=0.0;
  double logSum=0.0;
  int K=0;

  for (int b=1; b<maxBin; b++) {
    const double pw = (double)sig[b];
    psum2 += pw;
    logSum += log(pw + SF_EPS);
    K++;
  }

  if (psum2 < 1e-18 || K <= 2) {
    out.entropy = 0.0f;
    out.sf = 0.0f;
  } else {
    double H=0.0;
    for (int b=1; b<maxBin; b++) {
      const double pw = (double)sig[b];
      const double p = pw/psum2;
      if (p > 1e-18) H += -p*log(p);
    }
    H /= log((double)(maxBin - 1));
    out.entropy = clampf((float)H, 0.0f, 1.0f);

    const double geo = exp(logSum/(double)K);
    const double ari = psum2/(double)K;
    out.sf = clampf((float)(geo/(ari + SF_EPS)), 0.0f, 1.0f);
  }

  // HF ratio + variance
  const int lf1 = (int)fminf((float)(half - 1), floorf(LF_BAND_HI_HZ / binHz));
  const int hf0 = (int)fminf((float)(half - 1), ceilf (HF_BAND_LO_HZ / binHz));
  const int hf1 = (int)fminf((float)(half - 1), floorf(HF_BAND_HI_HZ / binHz));

  double pLF=0.0, pHF=0.0;
  for (int b=1;b<=lf1;b++) pLF += (double)sig[b];
  for (int b=hf0;b<=hf1;b++) pHF += (double)sig[b];

  const double pTot = pLF + pHF;
  out.hf_ratio = (pTot > 1e-12) ? (float)(pHF / (pTot + 1e-12)) : 0.0f;

  static constexpr int HF_WIN = 12;
  static float hfHist[HF_WIN];
  static int hfIdx=0;
  static bool hfInit=false;

  if (!hfInit) { for (int i=0;i<HF_WIN;i++) hfHist[i]=out.hf_ratio; hfInit=true; }

  hfHist[hfIdx] = out.hf_ratio;
  hfIdx = (hfIdx + 1) % HF_WIN;

  double mu=0.0;
  for (int i=0;i<HF_WIN;i++) mu += hfHist[i];
  mu /= (double)HF_WIN;

  double vv=0.0;
  for (int i=0;i<HF_WIN;i++) {
    const double d = (double)hfHist[i]-mu;
    vv += d*d;
  }
  vv /= (double)HF_WIN;
  out.hf_var = (float)vv;

  return true;
}