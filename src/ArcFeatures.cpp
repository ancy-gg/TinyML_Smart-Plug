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
  if (a > b) { float t=a; a=b; b=t; }
  if (b > c) { float t=b; b=c; c=t; }
  if (a > b) { float t=a; a=b; b=t; }
  return b;
}

bool ArcFeatures::compute(const uint16_t* raw, size_t n, float fs_hz,
                          const CurrentCalib& cal, float mainsHz,
                          ArcFeatOut& out) {
  if (!raw || n != N_SAMP) return false;

  out.fs_hz = fs_hz;

  static float x[N_SAMP];
  static float vReal[N_SAMP];
  static float vImag[N_SAMP];

  // 1) Convert + mean
  double mean = 0.0;
  int sat = 0;
  for (size_t i=0; i<n; i++) {
    const uint16_t c = raw[i];
    if (c < 32 || c > 65503) sat++;
    const float a = codeToCurrentA(c, cal);
    x[i] = a;
    mean += a;
  }
  mean /= (double)n;

  // Too many saturated / corrupted samples -> invalid frame
  if (sat > 32) return false;

  // 2) Center + RMS
  double acc = 0.0;
  for (size_t i=0; i<n; i++) {
    const float s = x[i] - (float)mean;
    vReal[i] = s;
    vImag[i] = 0.0f;
    acc += (double)s * (double)s;
  }
  const float irms_raw = (float)sqrt(acc / (double)n);

  // Median-of-3 (frame-to-frame)
  static bool init=false;
  static float r0=0, r1=0, r2=0;
  if (!init) { init=true; r0=r1=r2=irms_raw; }
  else { r0=r1; r1=r2; r2=irms_raw; }
  const float irms_med = median3(r0,r1,r2);

  // 2a) Idle floor that DOES NOT chase real loads
  static bool floorInit=false;
  static float idleFloor=0.0f;
  static uint16_t learnFrames=0;

  const float frameHz = (fs_hz > 1000.0f) ? (fs_hz / (float)n) : 30.0f;
  const uint16_t learnMinFrames = (uint16_t)fmaxf(8.0f, frameHz * 1.2f);

  static constexpr float FLOOR_LEARN_MAX_A = 0.03f; // only learn floor below this
  static constexpr float FLOOR_MAX_A       = 0.03f;
  static constexpr float FLOOR_INIT_MAX_A  = 0.02f;

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
    const float alphaUp = 0.01f;
    const float alphaDn = 0.12f;
    const float alpha = (irms_med > idleFloor) ? alphaUp : alphaDn;
    idleFloor = (1.0f - alpha)*idleFloor + alpha*irms_med;
    idleFloor = fminf(idleFloor, FLOOR_MAX_A);
  }

  // Linear subtraction keeps small steady loads (chargers) from being “eaten”
  const float irms_clean = fmaxf(0.0f, irms_med - idleFloor);
  out.irms_a = irms_clean;

  const bool lowSignal = (irms_clean < IDLE_IRMS_A);

  // 3) ZCV + cycle-to-cycle RMS variance
  int crossings[100];
  int cCount = 0;
  const float hys = fmaxf(ZC_HYS_MIN_A, ZC_HYS_FRAC * irms_med);

  int state = 0;
  if (vReal[0] >  hys) state =  1;
  if (vReal[0] < -hys) state = -1;

  for (size_t i=1; i<n && cCount<100; i++) {
    const float s = vReal[i];
    if (state <= 0 && s >  hys) { crossings[cCount++] = (int)i; state =  1; }
    else if (state >= 0 && s < -hys) { crossings[cCount++] = (int)i; state = -1; }
  }

  // ZCV
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

  // Cycle-to-cycle RMS variance (normalized)
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
        const double s = (double)vReal[i];
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

  // 4) FFT magnitude
  ArduinoFFT<float> FFT(vReal, vImag, n, fs_hz);
  FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  const float binHz = fs_hz / (float)n;

  // Fundamental near expected bin
  int kNom = (int)lround(mainsHz / binHz);
  if (kNom < 1) kNom = 1;
  if (kNom > (int)(n/2 - 2)) kNom = (int)(n/2 - 2);

  int k1 = kNom;
  float fund = 0.0f;
  for (int dk=-2; dk<=2; dk++) {
    int k = kNom + dk;
    if (k>=1 && k<(int)(n/2)) {
      if (vReal[k] > fund) { fund = vReal[k]; k1 = k; }
    }
  }

  // THD 2..10
  double harm2 = 0.0;
  for (int h=2; h<=10; h++) {
    int khNom = h*k1;
    if (khNom >= (int)(n/2)) break;
    float hm = 0.0f;
    for (int dk=-1; dk<=1; dk++) {
      int kh = khNom + dk;
      if (kh>=1 && kh<(int)(n/2)) hm = max(hm, vReal[kh]);
    }
    harm2 += (double)hm*(double)hm;
  }
  out.thd_pct = (fund > 1e-9f) ? (float)(sqrt(harm2)/(double)fund*100.0) : 0.0f;

  // Entropy + Spectral Flatness (up to ENTROPY_MAX_HZ)
  const int maxBin = (int)min((double)(n/2), floor((double)ENTROPY_MAX_HZ / (double)binHz));
  double psum=0.0;
  double logSum=0.0;
  int K=0;

  for (int b=1; b<maxBin; b++) {
    const double pw = (double)vReal[b]*(double)vReal[b];
    psum += pw;
    logSum += log(pw + SF_EPS);
    K++;
    vImag[b] = (float)pw;
  }

  if (psum < 1e-18 || K <= 2) {
    out.entropy = 0.0f;
    out.sf = 0.0f;
  } else {
    double H=0.0;
    for (int b=1; b<maxBin; b++) {
      const double p = (double)vImag[b]/psum;
      if (p > 1e-18) H += -p*log(p);
    }
    H /= log((double)(maxBin - 1));
    out.entropy = clampf((float)H, 0.0f, 1.0f);

    const double geo = exp(logSum/(double)K);
    const double ari = psum/(double)K;
    out.sf = clampf((float)(geo/(ari + SF_EPS)), 0.0f, 1.0f);
  }

  // HF ratio (bounded 0..1) + variance
  const int lf1 = min((int)(n/2 - 1), (int)floor(LF_BAND_HI_HZ / binHz));
  const int hf0 = min((int)(n/2 - 1), (int)ceil(HF_BAND_LO_HZ / binHz));
  const int hf1 = min((int)(n/2 - 1), (int)floor(HF_BAND_HI_HZ / binHz));

  double pLF=0.0, pHF=0.0;
  for (int b=1;b<=lf1;b++) pLF += (double)vReal[b]*(double)vReal[b];
  for (int b=hf0;b<=hf1;b++) pHF += (double)vReal[b]*(double)vReal[b];

  const double pTot = pLF + pHF;

  if (out.irms_a < 0.02f || pTot < 1e-9) out.hf_ratio = 0.0f;
  else out.hf_ratio = (float)(pHF / (pTot + 1e-12)); // 0..1

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