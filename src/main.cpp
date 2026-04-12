#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <math.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <string.h>

#include "MainConfiguration.h"
#include "WifiHandler.h"
#include "FirebaseNetwork.h"
#include "Notification.h"
#include "UpdateManager.h"
#include "CurrentSensor.h"
#include "VoltageSensor.h"
#include "TempSensor.h"
#include "ProtectionManager.h"
#include "ArcDetection.h"
#include "TinyMLContextModel.h"

Notification       notification(0x3C);
WifiHandler        wifiMgr;
FirebaseNetwork    network;
UpdateManager      updater;
VoltageSensor      voltSensor(PIN_VOLT_ADC);
TempSensor         tempSensor(PIN_TEMP_ADC);
ProtectionManager  protection;
CurrentSensor      curSensor;
ArcDetection       arcDetect;
CurrentCalib       curCalib;

struct RawFeatureFrame {
  uint16_t sample_count = 0;
  uint16_t fft_size = 0;
  uint16_t hop_samples = 0;
  uint32_t queue_drop_count = 0;
  uint32_t frame_start_uptime_ms = 0;
  uint32_t frame_end_uptime_ms = 0;
  float frame_dt_ms = 0.0f;
  float adc_fs_hz = 0.0f;
  float fs_err_hz = 0.0f;
  uint8_t sampling_quality_bad = 0;
  uint16_t samples[N_SAMP] = {0};
};

static uint16_t* s_raw = nullptr;
static uint16_t* s_hop = nullptr;
static QueueHandle_t qRawFeat = nullptr;
static QueueHandle_t qFeat = nullptr;
static TaskHandle_t gCore0SenseTask = nullptr;
static TaskHandle_t gCore1FeatTask = nullptr;
static SemaphoreHandle_t gArcRuntimeMutex = nullptr;
static bool gSafeMode = false;
static volatile bool gPauseByOta = false;
static volatile bool gStopSenseTask = false;
static volatile bool gSenseTaskRunning = false;
static volatile bool gFeatTaskRunning = false;
static volatile uint32_t gFeaturePipelineResetEpoch = 1;
static volatile uint32_t gRelayArtifactBlankUntilMs = 0;
static bool gManualRelayAssumedOn = false;
static bool gManualRelayConfirmPending = false;
static uint32_t gManualRelayCandidateSinceMs = 0;
static uint32_t gManualRelayReleaseSinceMs = 0;
static uint32_t gManualRelayConfirmedActiveMs = 0;
static uint32_t gManualRelayConfirmStartedMs = 0;
static uint32_t gManualRelayCooldownUntilMs = 0;
static uint32_t gManualRelayWebHoldoffUntilMs = 0;

static void Core0Task(void* pv);
static void Core1FeatureTask(void* pv);



struct RuntimeContextTracker {
  bool ready = false;
  int8_t family = FAMILY_UNKNOWN;
  float confidence = 0.0f;
  int8_t provisionalFamily = CONTEXT_FAMILY_UNKNOWN;
  float provisionalConfidence = 0.0f;
  uint16_t stableFrames = 0;
  uint32_t activeSinceMs = 0;
  uint32_t zeroSinceMs = 0;
  uint32_t stableAccumMs = 0;
  float sumAbsIrmsZ = 0.0f;
  float sumDeltaIrmsAbs = 0.0f;
  float sumHalfcycleAsymmetry = 0.0f;
  float sumSuspiciousRunEnergy = 0.0f;
  float sumDeltaHfEnergy = 0.0f;
  float sumDeltaFlux = 0.0f;
  float sumVSagPct = 0.0f;
  float sumMidband = 0.0f;
  float sumZcv = 0.0f;
  float sumSpectralFlux = 0.0f;
  float sumPeak = 0.0f;
  float sumResidualCf = 0.0f;
  float sumThd = 0.0f;
  float sumHf = 0.0f;
  float sumEdge = 0.0f;
  float sumIrms = 0.0f;
  float sumVrms = 0.0f;
  float sumNmse = 0.0f;
  uint32_t noMainsSinceMs = 0;
};

static RuntimeContextTracker gContext;

static void resetContextTracker_() {
  memset(&gContext, 0, sizeof(gContext));
  gContext.family = FAMILY_UNKNOWN;
  gContext.provisionalFamily = CONTEXT_FAMILY_UNKNOWN;
  gContext.confidence = 0.0f;
  gContext.provisionalConfidence = 0.0f;
  arcDetect.setContext(CONTEXT_FAMILY_UNKNOWN, 0.0f);
}

static inline void clearContextAccumulation_() {
  const uint32_t activeSinceMs = gContext.activeSinceMs;
  const uint32_t zeroSinceMs = gContext.zeroSinceMs;
  const uint32_t noMainsSinceMs = gContext.noMainsSinceMs;
  memset(&gContext, 0, sizeof(gContext));
  gContext.family = FAMILY_UNKNOWN;
  gContext.provisionalFamily = CONTEXT_FAMILY_UNKNOWN;
  gContext.activeSinceMs = activeSinceMs;
  gContext.zeroSinceMs = zeroSinceMs;
  gContext.noMainsSinceMs = noMainsSinceMs;
  arcDetect.setContext(CONTEXT_FAMILY_UNKNOWN, 0.0f);
}

static inline bool contextCurrentActive_(float vProtect, float irmsLogic) {
  return (vProtect >= MAINS_PRESENT_ON_V) && (irmsLogic >= CONTEXT_MIN_IRMS_A);
}

static inline bool contextFeatureFrameUsable_(const FeatureFrame& f, float vProtect, float irmsLogic, bool arcBlank) {
  if (arcBlank) return false;
  if (vProtect < MAINS_PRESENT_ON_V) return false;
  if ((f.current_valid == 0) || (f.feat_valid == 0)) return false;
  if (irmsLogic < CONTEXT_MIN_IRMS_A) return false;
  return true;
}

static inline uint32_t contextFrameMs_(const FeatureFrame& f) {
  float dtMs = f.frame_dt_ms;
  if (!isfinite(dtMs) || dtMs < 20.0f || dtMs > 500.0f) dtMs = FEATURE_FRAME_PERIOD_MS;
  if (dtMs < 20.0f) dtMs = 20.0f;
  if (dtMs > 500.0f) dtMs = 500.0f;
  return (uint32_t)(dtMs + 0.5f);
}

static inline float frameComputedFeatureById_(const FeatureFrame& f, int featureId) {
  switch (featureId) {
    case TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE: return f.abs_irms_zscore_vs_baseline;
    case TINYML_FEATURE_DELTA_IRMS_ABS: return f.delta_irms_abs;
    case TINYML_FEATURE_HALFCYCLE_ASYMMETRY: return f.halfcycle_asymmetry;
    case TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY: return f.suspicious_run_energy;
    case TINYML_FEATURE_DELTA_HF_ENERGY: return f.delta_hf_energy;
    case TINYML_FEATURE_DELTA_FLUX: return f.delta_flux;
    case TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO: return f.midband_residual_ratio;
    case TINYML_FEATURE_ZCV: return f.zcv;
    case TINYML_FEATURE_SPECTRAL_FLUX_MIDHF: return f.spectral_flux_midhf;
    case TINYML_FEATURE_PEAK_FLUCT_CV: return f.peak_fluct_cv;
    case TINYML_FEATURE_RESIDUAL_CREST_FACTOR: return f.residual_crest_factor;
    case TINYML_FEATURE_THD_I: return f.thd_i;
    case TINYML_FEATURE_HF_ENERGY_DELTA: return f.hf_energy_delta;
    case TINYML_FEATURE_EDGE_SPIKE_RATIO: return f.edge_spike_ratio;
    case TINYML_FEATURE_V_SAG_PCT: return f.v_sag_pct;
    case TINYML_FEATURE_CYCLE_NMSE: return f.cycle_nmse;
    default: return 0.0f;
  }
}

static inline void setFrameComputedFeatureById_(FeatureFrame& f, int featureId, float value) {
  switch (featureId) {
    case TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE: f.abs_irms_zscore_vs_baseline = value; break;
    case TINYML_FEATURE_DELTA_IRMS_ABS: f.delta_irms_abs = value; break;
    case TINYML_FEATURE_HALFCYCLE_ASYMMETRY: f.halfcycle_asymmetry = value; break;
    case TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY: f.suspicious_run_energy = value; break;
    case TINYML_FEATURE_DELTA_HF_ENERGY: f.delta_hf_energy = value; break;
    case TINYML_FEATURE_DELTA_FLUX: f.delta_flux = value; break;
    case TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO: f.midband_residual_ratio = value; break;
    case TINYML_FEATURE_ZCV: f.zcv = value; break;
    case TINYML_FEATURE_SPECTRAL_FLUX_MIDHF: f.spectral_flux_midhf = value; break;
    case TINYML_FEATURE_PEAK_FLUCT_CV: f.peak_fluct_cv = value; break;
    case TINYML_FEATURE_RESIDUAL_CREST_FACTOR: f.residual_crest_factor = value; break;
    case TINYML_FEATURE_THD_I: f.thd_i = value; break;
    case TINYML_FEATURE_HF_ENERGY_DELTA: f.hf_energy_delta = value; break;
    case TINYML_FEATURE_EDGE_SPIKE_RATIO: f.edge_spike_ratio = value; break;
    case TINYML_FEATURE_V_SAG_PCT: f.v_sag_pct = value; break;
    case TINYML_FEATURE_CYCLE_NMSE: f.cycle_nmse = value; break;
    default: break;
  }
}

static inline float arcResultComputedFeatureById_(const ArcDetectionResult& out, int featureId) {
  switch (featureId) {
    case TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE: return out.abs_irms_zscore_vs_baseline;
    case TINYML_FEATURE_DELTA_IRMS_ABS: return out.delta_irms_abs;
    case TINYML_FEATURE_HALFCYCLE_ASYMMETRY: return out.halfcycle_asymmetry;
    case TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY: return out.suspicious_run_energy;
    case TINYML_FEATURE_DELTA_HF_ENERGY: return out.delta_hf_energy;
    case TINYML_FEATURE_DELTA_FLUX: return out.delta_flux;
    case TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO: return out.midband_residual_ratio;
    case TINYML_FEATURE_ZCV: return out.zcv;
    case TINYML_FEATURE_SPECTRAL_FLUX_MIDHF: return out.spectral_flux_midhf;
    case TINYML_FEATURE_PEAK_FLUCT_CV: return out.peak_fluct_cv;
    case TINYML_FEATURE_RESIDUAL_CREST_FACTOR: return out.residual_crest_factor;
    case TINYML_FEATURE_THD_I: return out.thd_i;
    case TINYML_FEATURE_HF_ENERGY_DELTA: return out.hf_energy_delta;
    case TINYML_FEATURE_EDGE_SPIKE_RATIO: return out.edge_spike_ratio;
    case TINYML_FEATURE_V_SAG_PCT: return out.v_sag_pct;
    case TINYML_FEATURE_CYCLE_NMSE: return out.cycle_nmse;
    default: return 0.0f;
  }
}

static inline void copyComputedFeaturesToFrame_(FeatureFrame& f, const ArcDetectionResult& out) {
  for (uint8_t i = 0; i < TINYML_COMPUTED_FEATURE_COUNT; ++i) {
    const int featureId = TINYML_RUNTIME_EXPORT_FEATURE_IDS[i];
    setFrameComputedFeatureById_(f, featureId,
                                 tinymlClampFeatureValue(featureId, arcResultComputedFeatureById_(out, featureId)));
  }
}

static inline void zeroComputedFeatures_(FeatureFrame& f) {
  for (uint8_t i = 0; i < TINYML_COMPUTED_FEATURE_COUNT; ++i) {
    const int featureId = TINYML_RUNTIME_EXPORT_FEATURE_IDS[i];
    setFrameComputedFeatureById_(f, featureId, 0.0f);
  }
}

static inline void sanitizeFeatureFrame_(FeatureFrame& f) {
  for (uint8_t i = 0; i < TINYML_COMPUTED_FEATURE_COUNT; ++i) {
    const int featureId = TINYML_RUNTIME_EXPORT_FEATURE_IDS[i];
    setFrameComputedFeatureById_(f, featureId,
                                 tinymlClampFeatureValue(featureId, frameComputedFeatureById_(f, featureId)));
  }
  if (!isfinite(f.vrms) || f.vrms < 0.0f) f.vrms = 0.0f;
  else if (f.vrms > 400.0f) f.vrms = 400.0f;
  if (!isfinite(f.irms) || f.irms < 0.0f) f.irms = 0.0f;
  else if (f.irms > 40.0f) f.irms = 40.0f;
  if (!isfinite(f.temp_c)) f.temp_c = 0.0f;
  if (!isfinite(f.adc_fs_hz) || f.adc_fs_hz < 0.0f) f.adc_fs_hz = 0.0f;
  if (!isfinite(f.fs_err_hz) || f.fs_err_hz < 0.0f) f.fs_err_hz = fabsf(FS_INTENDED_HZ);
  if (f.fft_size == 0U) f.fft_size = ARC_RUNTIME_FRAME_SAMPLES;
  if (f.hop_samples == 0U) f.hop_samples = ARC_RUNTIME_HOP_SAMPLES;
  f.context_family_confidence =
      tinymlClampFeatureValue(TINYML_FEATURE_CONTEXT_FAMILY_CONFIDENCE, f.context_family_confidence);
  f.context_family_confidence_provisional =
      tinymlClampFeatureValue(TINYML_FEATURE_CONTEXT_FAMILY_CONFIDENCE, f.context_family_confidence_provisional);
}

static inline float contextFeatureAverageById_(int featureId, float denom) {
  float value = 0.0f;
  switch (featureId) {
    case TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE: value = gContext.sumAbsIrmsZ / denom; break;
    case TINYML_FEATURE_DELTA_IRMS_ABS: value = gContext.sumDeltaIrmsAbs / denom; break;
    case TINYML_FEATURE_HALFCYCLE_ASYMMETRY: value = gContext.sumHalfcycleAsymmetry / denom; break;
    case TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY: value = gContext.sumSuspiciousRunEnergy / denom; break;
    case TINYML_FEATURE_DELTA_HF_ENERGY: value = gContext.sumDeltaHfEnergy / denom; break;
    case TINYML_FEATURE_DELTA_FLUX: value = gContext.sumDeltaFlux / denom; break;
    case TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO: value = gContext.sumMidband / denom; break;
    case TINYML_FEATURE_ZCV: value = gContext.sumZcv / denom; break;
    case TINYML_FEATURE_SPECTRAL_FLUX_MIDHF: value = gContext.sumSpectralFlux / denom; break;
    case TINYML_FEATURE_PEAK_FLUCT_CV: value = gContext.sumPeak / denom; break;
    case TINYML_FEATURE_RESIDUAL_CREST_FACTOR: value = gContext.sumResidualCf / denom; break;
    case TINYML_FEATURE_THD_I: value = gContext.sumThd / denom; break;
    case TINYML_FEATURE_HF_ENERGY_DELTA: value = gContext.sumHf / denom; break;
    case TINYML_FEATURE_EDGE_SPIKE_RATIO: value = gContext.sumEdge / denom; break;
    case TINYML_FEATURE_V_SAG_PCT: value = gContext.sumVSagPct / denom; break;
    case TINYML_FEATURE_CYCLE_NMSE: value = gContext.sumNmse / denom; break;
    default: value = 0.0f; break;
  }
  return tinymlClampFeatureValue(featureId, value);
}

static inline void fillContextModelInput_(double* dst) {
  for (int i = 0; i < CONTEXT_MODEL_INPUT_DIM; ++i) dst[i] = 0.0;
  const float denom = fmaxf(1.0f, (float)gContext.stableFrames);

#if defined(CONTEXT_MODEL_METADATA_VERSION)
  for (int i = 0; i < CONTEXT_MODEL_INPUT_DIM; ++i) {
    dst[i] = (double)contextFeatureAverageById_(context_model_input_feature_ids[i], denom);
  }
  return;
#else
  if (CONTEXT_MODEL_INPUT_DIM == 10) {
    const float legacyFeatures[10] = {
      gContext.sumResidualCf / denom,
      gContext.sumEdge / denom,
      gContext.sumMidband / denom,
      gContext.sumThd / denom,
      gContext.sumHf / denom,
      gContext.sumZcv / denom,
      gContext.sumIrms / denom,
      gContext.sumVrms / denom,
      gContext.sumNmse / denom,
      gContext.sumPeak / denom,
    };
    for (int i = 0; i < 10; ++i) dst[i] = (double)legacyFeatures[i];
    return;
  }

  const float rankedFeatures[14] = {
    gContext.sumAbsIrmsZ / denom,
    gContext.sumDeltaIrmsAbs / denom,
    gContext.sumHalfcycleAsymmetry / denom,
    gContext.sumSuspiciousRunEnergy / denom,
    gContext.sumDeltaHfEnergy / denom,
    gContext.sumDeltaFlux / denom,
    gContext.sumMidband / denom,
    gContext.sumZcv / denom,
    gContext.sumSpectralFlux / denom,
    gContext.sumPeak / denom,
    gContext.sumResidualCf / denom,
    gContext.sumThd / denom,
    gContext.sumHf / denom,
    gContext.sumEdge / denom,
  };
  for (int i = 0; i < 14; ++i) dst[i] = (double)rankedFeatures[i];
#endif
}



static inline void evaluateContextPrediction_() {
  if (gContext.stableFrames == 0) {
    gContext.provisionalFamily = CONTEXT_FAMILY_UNKNOWN;
    gContext.provisionalConfidence = 0.0f;
    return;
  }

  double input[CONTEXT_MODEL_INPUT_DIM] = {0};
  fillContextModelInput_(input);
  double probs[CONTEXT_MODEL_FAMILY_COUNT] = {0};

  context_family_predict(input, probs);
  double conf = 0.0;
  int fam = context_family_best(probs, &conf);
  const float cf = (conf >= 0.0) ? (float)conf : 0.0f;
  gContext.provisionalConfidence = cf;
  if (cf < CONTEXT_MIN_CONFIDENCE) fam = CONTEXT_FAMILY_UNKNOWN;
  gContext.provisionalFamily =
      (fam >= 0 && fam < FAMILY_COUNT) ? (int8_t)fam : (int8_t)CONTEXT_FAMILY_UNKNOWN;
}

static void updateContextTracker_(const FeatureFrame& f, float vProtect, float irmsLogic, bool arcBlank) {
  const uint32_t now = millis();
  if (vProtect < MAINS_PRESENT_ON_V) {
    if (gContext.noMainsSinceMs == 0) gContext.noMainsSinceMs = now;
    if ((now - gContext.noMainsSinceMs) >= CONTEXT_RESET_NO_MAINS_MS) resetContextTracker_();
    return;
  }
  gContext.noMainsSinceMs = 0;

  const bool currentActive = contextCurrentActive_(vProtect, irmsLogic);

  if (!currentActive) {
    if (gContext.ready) {
      if (gContext.zeroSinceMs == 0) gContext.zeroSinceMs = now;
      if ((now - gContext.zeroSinceMs) >= CONTEXT_UNLATCH_ZERO_MS) resetContextTracker_();
      else arcDetect.setContext(gContext.family, gContext.confidence);
    } else {
      if (gContext.activeSinceMs == 0U) {
        gContext.zeroSinceMs = 0;
        clearContextAccumulation_();
      } else {
        if (gContext.zeroSinceMs == 0U) gContext.zeroSinceMs = now;
        if ((now - gContext.zeroSinceMs) >= CONTEXT_ACQUIRE_DROP_GRACE_MS) {
          gContext.activeSinceMs = 0;
          gContext.zeroSinceMs = 0;
          clearContextAccumulation_();
        }
      }
    }
    return;
  }

  gContext.zeroSinceMs = 0;
  if (gContext.activeSinceMs == 0) {
    clearContextAccumulation_();
    gContext.activeSinceMs = now;
  }

  if (gContext.ready) {
    arcDetect.setContext(gContext.family, gContext.confidence);
    return;
  }

  if (contextFeatureFrameUsable_(f, vProtect, irmsLogic, arcBlank)) {
    gContext.stableFrames++;
    gContext.stableAccumMs += contextFrameMs_(f);
    gContext.sumAbsIrmsZ += f.abs_irms_zscore_vs_baseline;
    gContext.sumDeltaIrmsAbs += f.delta_irms_abs;
    gContext.sumHalfcycleAsymmetry += f.halfcycle_asymmetry;
    gContext.sumSuspiciousRunEnergy += f.suspicious_run_energy;
    gContext.sumDeltaHfEnergy += f.delta_hf_energy;
    gContext.sumDeltaFlux += f.delta_flux;
    gContext.sumVSagPct += f.v_sag_pct;
    gContext.sumMidband += f.midband_residual_ratio;
    gContext.sumZcv += f.zcv;
    gContext.sumSpectralFlux += f.spectral_flux_midhf;
    gContext.sumPeak += f.peak_fluct_cv;
    gContext.sumResidualCf += f.residual_crest_factor;
    gContext.sumThd += f.thd_i;
    gContext.sumHf += f.hf_energy_delta;
    gContext.sumEdge += f.edge_spike_ratio;
    gContext.sumIrms += irmsLogic;
    gContext.sumVrms += f.vrms;
    gContext.sumNmse += f.cycle_nmse;
    evaluateContextPrediction_();
  }

  const uint32_t activeWindowMs = (gContext.activeSinceMs > 0U) ? (now - gContext.activeSinceMs) : 0U;
  const bool provisionalReady =
      (activeWindowMs >= CONTEXT_PROVISIONAL_MIN_MS) &&
      (gContext.provisionalFamily != CONTEXT_FAMILY_UNKNOWN) &&
      (gContext.provisionalConfidence >= CONTEXT_MIN_CONFIDENCE);

  if ((activeWindowMs >= CONTEXT_ACQUIRE_WINDOW_MS) && provisionalReady) {
    gContext.ready = true;
    gContext.family = gContext.provisionalFamily;
    gContext.confidence = gContext.provisionalConfidence;
    arcDetect.setContext(gContext.family, gContext.confidence);
  } else if (provisionalReady) {
    arcDetect.setContext(gContext.provisionalFamily, gContext.provisionalConfidence);
  } else {
    arcDetect.setContext(CONTEXT_FAMILY_UNKNOWN, 0.0f);
  }
}

static inline void applyContextToFrame_(FeatureFrame& f) {
  const uint32_t now = millis();
  const uint32_t activeWindowMs = (gContext.activeSinceMs > 0U) ? (now - gContext.activeSinceMs) : 0U;
  const bool provisionalReady =
      !gContext.ready &&
      (activeWindowMs >= CONTEXT_PROVISIONAL_MIN_MS) &&
      (gContext.provisionalFamily != CONTEXT_FAMILY_UNKNOWN) &&
      (gContext.provisionalConfidence >= CONTEXT_MIN_CONFIDENCE);

  const int8_t runtimeFamily =
      gContext.ready ? gContext.family : (provisionalReady ? gContext.provisionalFamily : (int8_t)CONTEXT_FAMILY_UNKNOWN);
  const float runtimeConfidence =
      gContext.ready ? gContext.confidence : (provisionalReady ? gContext.provisionalConfidence : 0.0f);

  f.context_family_code_runtime = runtimeFamily;
  f.context_family_confidence = runtimeConfidence;
  f.context_family_code_provisional = gContext.provisionalFamily;
  f.context_family_confidence_provisional = gContext.provisionalConfidence;
  f.context_acquiring = (!gContext.ready && gContext.activeSinceMs > 0U) ? 1U : 0U;
  f.context_latched = gContext.ready ? 1U : 0U;
}

static inline void requestFeaturePipelineReset_() {
  gFeaturePipelineResetEpoch++;
}

static void resetArcRuntime_() {
  if (gArcRuntimeMutex && xSemaphoreTake(gArcRuntimeMutex, pdMS_TO_TICKS(40)) == pdTRUE) {
    arcDetect.resetRuntime();
    xSemaphoreGive(gArcRuntimeMutex);
  } else {
    arcDetect.resetRuntime();
  }
}

static bool allocSampleBuffer_() {
  if (!s_raw) {
    s_raw = static_cast<uint16_t*>(heap_caps_malloc(sizeof(uint16_t) * N_SAMP, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!s_raw) s_raw = static_cast<uint16_t*>(heap_caps_malloc(sizeof(uint16_t) * N_SAMP, MALLOC_CAP_8BIT));
  }
  if (!s_hop) {
    s_hop = static_cast<uint16_t*>(heap_caps_malloc(sizeof(uint16_t) * ARC_RUNTIME_HOP_SAMPLES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!s_hop) s_hop = static_cast<uint16_t*>(heap_caps_malloc(sizeof(uint16_t) * ARC_RUNTIME_HOP_SAMPLES, MALLOC_CAP_8BIT));
  }
  return (s_raw != nullptr) && (s_hop != nullptr);
}

static bool requestSenseTaskStop_(uint32_t waitMs = 3500UL) {
  if (!gCore0SenseTask && !gSenseTaskRunning && !gCore1FeatTask && !gFeatTaskRunning) {
    gStopSenseTask = false;
    return true;
  }

  gStopSenseTask = true;
  const uint32_t t0 = millis();
  while ((gCore0SenseTask || gSenseTaskRunning || gCore1FeatTask || gFeatTaskRunning) &&
         ((millis() - t0) < waitMs)) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  const bool stopped = (!gCore0SenseTask && !gSenseTaskRunning && !gCore1FeatTask && !gFeatTaskRunning);
  gStopSenseTask = false;
  return stopped;
}

static bool stopSensePipeline_(bool freeBuffer = true) {
  const bool stopped = requestSenseTaskStop_();
  if (!stopped) return false;
  if (qRawFeat) {
    vQueueDelete(qRawFeat);
    qRawFeat = nullptr;
  }
  if (qFeat) {
    vQueueDelete(qFeat);
    qFeat = nullptr;
  }
  if (freeBuffer && s_raw) {
    heap_caps_free(s_raw);
    s_raw = nullptr;
  }
  if (freeBuffer && s_hop) {
    heap_caps_free(s_hop);
    s_hop = nullptr;
  }
  return true;
}

static bool startSensePipeline_() {
  if (gCore0SenseTask || gCore1FeatTask) return true;
  if (!allocSampleBuffer_()) return false;
  if (!curSensor.begin()) return false;
  if (!gArcRuntimeMutex) gArcRuntimeMutex = xSemaphoreCreateMutex();
  if (!gArcRuntimeMutex) return false;
  if (!qRawFeat) qRawFeat = xQueueCreate(FEATURE_RAW_FRAME_QUEUE_LEN, sizeof(RawFeatureFrame));
  if (!qRawFeat) return false;
  if (!qFeat) qFeat = xQueueCreate(FEATURE_FRAME_QUEUE_LEN, sizeof(FeatureFrame));
  if (!qFeat) {
    vQueueDelete(qRawFeat);
    qRawFeat = nullptr;
    return false;
  }
  const BaseType_t capOk = xTaskCreatePinnedToCore(Core0Task, "Core0Sense", 14336, nullptr, 4, &gCore0SenseTask, 0);
  if (capOk != pdPASS) {
    gCore0SenseTask = nullptr;
    vQueueDelete(qRawFeat);
    qRawFeat = nullptr;
    vQueueDelete(qFeat);
    qFeat = nullptr;
    return false;
  }
  const BaseType_t featOk = xTaskCreatePinnedToCore(Core1FeatureTask, "Core1Feat", 16384, nullptr, 2, &gCore1FeatTask, 1);
  if (featOk != pdPASS) {
    gStopSenseTask = true;
    requestSenseTaskStop_(2500UL);
    gStopSenseTask = false;
    gCore1FeatTask = nullptr;
    if (qRawFeat) {
      vQueueDelete(qRawFeat);
      qRawFeat = nullptr;
    }
    if (qFeat) {
      vQueueDelete(qFeat);
      qFeat = nullptr;
    }
    return false;
  }
  requestFeaturePipelineReset_();
  return true;
}

static bool enterOtaExclusiveMode_() {
  gPauseByOta = true;
  network.setLogEnabled(false);
  network.setMlUploadSuspended(true);
  network.stopAllClients();
  delay(180);
  const bool stopped = stopSensePipeline_(true);
  network.stopAllClients();
  delay(120);
  return stopped;
}

static bool exitOtaExclusiveMode_() {
  gPauseByOta = false;
  network.setMlUploadSuspended(false);
  network.stopAllClients();
  delay(120);
  if (gSafeMode) return true;
  return startSensePipeline_();
}

static inline void armRelayArtifactBlank_(uint32_t extraMs = RELAY_ARTIFACT_BLANK_MS) {
  gRelayArtifactBlankUntilMs = millis() + extraMs;
  requestFeaturePipelineReset_();
  resetArcRuntime_();
}

static inline void clearManualRelayAssume_() {
  gManualRelayAssumedOn = false;
  gManualRelayConfirmPending = false;
  gManualRelayCandidateSinceMs = 0;
  gManualRelayReleaseSinceMs = 0;
  gManualRelayConfirmedActiveMs = 0;
  gManualRelayConfirmStartedMs = 0;
}

static bool restartSensePipelineSoft_() {
  if (!stopSensePipeline_(false)) return false;
  resetArcRuntime_();
  delay(40);
  return startSensePipeline_();
}

static bool arcInputStable(bool currentValid, float irms) {
  static uint32_t stableSince = 0;
  static float refIrms = 0.0f;
  const uint32_t now = millis();
  if (!currentValid || irms < 0.03f) {
    stableSince = 0; refIrms = irms; return false;
  }
  if (stableSince == 0) { stableSince = now; refIrms = irms; return false; }
  const float tol = fmaxf(0.12f, 0.40f * fmaxf(refIrms, 0.10f));
  if (fabsf(irms - refIrms) > tol) { stableSince = now; refIrms = irms; return false; }
  refIrms += 0.12f * (irms - refIrms);
  return (now - stableSince) >= 420UL;
}


static bool arcTurnOnBlanking(bool relayOn, bool currentValid, float vProtect, float irms) {
  static uint32_t lowSinceMs = 0;
  static uint32_t blankUntilMs = 0;

  const uint32_t now = millis();
  const bool mainsOn = (vProtect >= MAINS_PRESENT_ON_V);
  const bool lowNow = (!currentValid) || (irms <= ARC_TURNON_LOW_A);
  const bool activeNow = currentValid && relayOn && mainsOn && (irms >= ARC_TURNON_ACTIVE_A);

  if (!relayOn || !mainsOn) {
    lowSinceMs = 0;
    blankUntilMs = 0;
    return false;
  }

  if (lowNow) {
    if (lowSinceMs == 0) lowSinceMs = now;
    return ((int32_t)(blankUntilMs - now) > 0);
  }

  const bool hadLowState = (lowSinceMs != 0) && ((now - lowSinceMs) >= ARC_TURNON_LOW_MS);
  if (hadLowState && activeNow) {
    blankUntilMs = now + ARC_TURNON_BLANK_MS;
    lowSinceMs = 0;
  } else if (!activeNow) {
    lowSinceMs = 0;
  }

  return ((int32_t)(blankUntilMs - now) > 0);
}

static bool arcTransientBlanking(bool relayOn,
                                 bool currentValid,
                                 float vProtect,
                                 float irms) {
  static float prevIrms = 0.0f;
  static uint32_t blankUntilMs = 0;

  const uint32_t now = millis();
  const bool mainsOn = (vProtect >= MAINS_PRESENT_ON_V);

  if (!relayOn || !mainsOn || !currentValid) {
    prevIrms = irms;
    blankUntilMs = 0;
    return false;
  }

  const float stepAbs = fabsf(irms - prevIrms);
  const float stepNeed = fmaxf(ARC_TRANSIENT_STEP_A,
                               ARC_TRANSIENT_STEP_FRAC * fmaxf(prevIrms, irms));
  prevIrms = irms;

  if (stepAbs >= stepNeed) {
    blankUntilMs = now + ARC_TRANSIENT_BLANK_MS;
  }

  return ((int32_t)(blankUntilMs - now) > 0);
}

static bool arcEventSignature(const FeatureFrame& f, float irmsLogic) {
  if ((f.feat_valid == 0) || (irmsLogic < ARC_SOFT_MIN_IRMS_A)) return false;

  int hits = 0;
  if (f.abs_irms_zscore_vs_baseline >= ARC_SIG_IRMS_ZSCORE) hits += 3;
  if (f.delta_irms_abs >= 0.12f) hits += 2;
  if (f.halfcycle_asymmetry >= 10.0f) hits += 2;
  if (f.delta_hf_energy >= 0.70f) hits += 2;
  if (f.delta_flux >= 4.0f) hits += 2;
  if (f.midband_residual_ratio >= ARC_SIG_MIDBAND_RATIO) hits += 1;
  if (f.zcv >= ARC_SIG_ZCV) hits += 1;
  if (f.spectral_flux_midhf >= ARC_SIG_SPECTRAL_FLUX) hits += 1;
  if (f.peak_fluct_cv >= ARC_SIG_PEAK_FLUCT) hits += 1;
  if (f.residual_crest_factor >= ARC_SIG_RESIDUAL_CF) hits += 1;
  if (f.thd_i >= ARC_SIG_THD_I) hits += 1;
  if (f.hf_energy_delta >= ARC_SIG_HF_ENERGY_DELTA) hits += 1;
  if (f.edge_spike_ratio >= ARC_SIG_EDGE_SPIKE_RATIO) hits += 1;

  return hits >= 5;
}

static bool softArcBurstEvent(const FeatureFrame& f,
                              float irmsLogic,
                              bool hadSteadyLoad,
                              float vProtect) {
  if (!hadSteadyLoad) return false;
  if ((f.feat_valid == 0) || (f.current_valid == 0)) return false;
  if (irmsLogic < ARC_SOFT_MIN_IRMS_A) return false;
  if (vProtect < VOLT_NORMAL_MIN_V || vProtect > VOLT_NORMAL_MAX_V) return false;

  int hits = 0;
  if (f.abs_irms_zscore_vs_baseline >= ARC_SIG_IRMS_ZSCORE) hits += 3;
  if (f.delta_irms_abs >= 0.12f) hits += 2;
  if (f.halfcycle_asymmetry >= 10.0f) hits += 2;
  if (f.delta_hf_energy >= 0.70f) hits += 2;
  if (f.delta_flux >= 4.0f) hits += 2;
  if (f.midband_residual_ratio >= ARC_SIG_MIDBAND_RATIO) hits += 1;
  if (f.zcv >= ARC_SIG_ZCV) hits += 1;
  if (f.spectral_flux_midhf >= ARC_SIG_SPECTRAL_FLUX) hits += 1;
  if (f.peak_fluct_cv >= ARC_SIG_PEAK_FLUCT) hits += 1;
  if (f.residual_crest_factor >= ARC_SIG_RESIDUAL_CF) hits += 1;
  if (f.thd_i >= ARC_SIG_THD_I) hits += 1;
  if (f.hf_energy_delta >= ARC_SIG_HF_ENERGY_DELTA) hits += 1;
  if (f.edge_spike_ratio >= ARC_SIG_EDGE_SPIKE_RATIO) hits += 1;

  const bool primaryBurst =
      (f.abs_irms_zscore_vs_baseline >= ARC_SIG_IRMS_ZSCORE) ||
      (f.delta_irms_abs >= 0.12f) ||
      (f.delta_hf_energy >= 0.70f && f.delta_flux >= 4.0f) ||
      (f.halfcycle_asymmetry >= 10.0f);

  return primaryBurst && (hits >= 6);
}

static bool debouncedMainsPresentForState(float vProtect) {
  static bool init = false, stable = false;
  static int8_t pending = -1;
  static uint32_t pendingSince = 0;
  const uint32_t now = millis();
  const bool rawOn = (vProtect >= MAINS_PRESENT_ON_V);
  const bool rawOff = (vProtect <= MAINS_PRESENT_OFF_V);
  if (!init) { stable = rawOn; init = true; }
  int8_t target = -1;
  if (rawOn) target = 1; else if (rawOff) target = 0;
  if (target >= 0 && target != (stable ? 1 : 0)) {
    if (pending != target) { pending = target; pendingSince = now; }
    else if ((now - pendingSince) >= MAINS_EDGE_DEBOUNCE_MS) { stable = (target != 0); pending = -1; pendingSince = 0; }
  } else if (target == (stable ? 1 : 0)) {
    pending = -1; pendingSince = 0;
  }
  return stable;
}

static bool classifyUnpluggedSocket(float vRaw, float vProtect, float irmsA, bool currentValid, FaultState st, uint32_t* sinceMs) {
  (void)irmsA;
  (void)currentValid;
  (void)st;
  if (!sinceMs) return false;
  const uint32_t now = millis();
  const bool mainsPresent = (vRaw >= VOLTAGE_SNAP_RESTORE_V) || (vProtect >= VOLTAGE_SNAP_RESTORE_V);
  const bool mainsGone = (vRaw <= VOLTAGE_SNAP_ZERO_V) && (vProtect <= VOLTAGE_SNAP_ZERO_V);
  if (mainsPresent) {
    *sinceMs = 0;
    return false;
  }
  if (mainsGone) {
    if (*sinceMs == 0) *sinceMs = now;
  } else {
    *sinceMs = 0;
  }
  return (*sinceMs != 0) && ((now - *sinceMs) >= UNPLUGGED_STATE_DELAY_MS);
}


static bool stabilizeFeatureValidity(FeatureFrame& f, float vProtect, float irmsLogic, bool clearState) {
  static FeatureFrame lastValidFeat = {};
  static bool haveLastValidFeat = false;
  static uint32_t lastValidFeatMs = 0;
  static uint32_t zeroIrmsSinceMs = 0;

  const uint32_t now = millis();
  if (clearState || vProtect <= MAINS_PRESENT_OFF_V) {
    haveLastValidFeat = false;
    lastValidFeatMs = 0;
    zeroIrmsSinceMs = 0;
    return false;
  }

  const bool mainsOn = (vProtect >= MAINS_PRESENT_ON_V);
  const bool nearZeroIrms = (fabsf(irmsLogic) <= CURRENT_ANALYSIS_IDLE_A);
  if (mainsOn && nearZeroIrms) {
    if (zeroIrmsSinceMs == 0) zeroIrmsSinceMs = now;
  } else {
    zeroIrmsSinceMs = 0;
  }

  const bool zeroTooLong = (zeroIrmsSinceMs != 0) && ((now - zeroIrmsSinceMs) >= 5000UL);

  if ((f.current_valid != 0) && (f.feat_valid != 0)) {
    lastValidFeat = f;
    haveLastValidFeat = true;
    lastValidFeatMs = now;
    return false;
  }

  const bool bridgeAllowed =
      mainsOn &&
      !zeroTooLong &&
      haveLastValidFeat &&
      ((now - lastValidFeatMs) <= 600UL) &&
      ((f.current_valid != 0) || (irmsLogic > 0.03f));

  if (!bridgeAllowed) return false;

  f.spectral_flux_midhf = lastValidFeat.spectral_flux_midhf;
  f.residual_crest_factor = lastValidFeat.residual_crest_factor;
  f.edge_spike_ratio = lastValidFeat.edge_spike_ratio;
  f.midband_residual_ratio = lastValidFeat.midband_residual_ratio;
  f.cycle_nmse = lastValidFeat.cycle_nmse;
  f.peak_fluct_cv = lastValidFeat.peak_fluct_cv;
  f.thd_i = lastValidFeat.thd_i;
  f.hf_energy_delta = lastValidFeat.hf_energy_delta;
  f.zcv = lastValidFeat.zcv;
  f.abs_irms_zscore_vs_baseline = lastValidFeat.abs_irms_zscore_vs_baseline;
  f.adc_fs_hz = lastValidFeat.adc_fs_hz;
  f.feat_valid = 1;
  return true;
}

static float cleanDisplayCurrent(float irmsRaw, bool currentValid, bool featValid, float vRaw, float vProtect) {
  static bool learnStarted = false, floorLocked = false, displayGateOn = false;
  static uint32_t learnStartMs = 0, lastUpdateMs = 0;
  static double floorSqAcc = 0.0;
  static uint32_t floorCount = 0;
  static float learnedFloorA = 0.0f, dispIrms = 0.0f;
  static float shownIrms = 0.0f;
  static uint32_t shownAtMs = 0;
  const uint32_t now = millis();
  const bool mainsOn = (vProtect >= MAINS_PRESENT_ON_V);

  if (vRaw <= MAINS_PRESENT_OFF_V) { dispIrms = 0.0f; shownIrms = 0.0f; displayGateOn = false; lastUpdateMs = now; shownAtMs = now; return 0.0f; }
  if (!learnStarted && mainsOn) { learnStarted = true; learnStartMs = now; }

  const bool canLearnIdle = CURRENT_IDLE_LEARN_ENABLE && learnStarted && !floorLocked && mainsOn && currentValid && !featValid &&
                            irmsRaw > 0.005f && irmsRaw < CURRENT_IDLE_LEARN_MAX_A && ((now - learnStartMs) <= CURRENT_IDLE_LEARN_WINDOW_MS);
  if (canLearnIdle) {
    floorSqAcc += (double)irmsRaw * (double)irmsRaw; floorCount++;
    learnedFloorA = sqrtf((float)(floorSqAcc / (double)floorCount));
  }
  if (CURRENT_IDLE_LEARN_ENABLE && learnStarted && !floorLocked && ((now - learnStartMs) > CURRENT_IDLE_LEARN_WINDOW_MS)) {
    floorLocked = (floorCount >= CURRENT_IDLE_LEARN_MIN_FRAMES);
  }

  float irms = irmsRaw;
  if (floorLocked && learnedFloorA > 0.005f) {
    const float floorUsed = fmaxf(0.0f, learnedFloorA - CURRENT_IDLE_FLOOR_MARGIN_A);
    irms = sqrtf(fmaxf(0.0f, (irms * irms) - (floorUsed * floorUsed)));
  }
  if (!currentValid) irms = 0.0f;

  const uint32_t dtMs = (lastUpdateMs == 0) ? 0 : (now - lastUpdateMs);
  lastUpdateMs = now;
  const float dtS = (dtMs > 0) ? (dtMs / 1000.0f) : 0.0f;
  const float tau = (irms <= CURRENT_DISPLAY_SMOOTH_SPLIT_A) ? CURRENT_DISPLAY_SMOOTH_TAU_LOW_S : CURRENT_DISPLAY_SMOOTH_TAU_HIGH_S;
  const float alpha = (dtS <= 0.0f) ? 1.0f : fminf(1.0f, dtS / fmaxf(0.02f, tau));
  dispIrms += alpha * (irms - dispIrms);
  if (displayGateOn) { if (dispIrms < CURRENT_DISPLAY_GATE_OFF_A) displayGateOn = false; }
  else { if (dispIrms > CURRENT_DISPLAY_GATE_ON_A) displayGateOn = true; }
  if (!displayGateOn) dispIrms = 0.0f;
  if ((now - shownAtMs) >= 250UL) {
    shownIrms = dispIrms;
    shownAtMs = now;
  }
  return shownIrms;
}

static float cleanLogicCurrent(float irmsRaw, bool currentValid, float vRaw, float vProtect) {
  static uint32_t mainsGoneSinceMs = 0;
  const uint32_t now = millis();
  const bool rawGone = (vRaw <= MAINS_PRESENT_OFF_V);
  const bool protGone = (vProtect <= MAINS_PRESENT_OFF_V);
  if (rawGone || protGone) {
    if (mainsGoneSinceMs == 0) mainsGoneSinceMs = now;
    if ((now - mainsGoneSinceMs) >= 450UL) return 0.0f;
  } else mainsGoneSinceMs = 0;
  if (!currentValid) return 0.0f;
  return irmsRaw;
}

static void handleCueEvents(float vRaw, float vProtect, float irms, bool mainsPresent, bool paused, FaultState st) {
  static uint32_t lowSinceMs = 0, highSinceMs = 0, lastVoltWarnMs = 0;
  static float loadBaseA = 0.0f, prevIrms = 0.0f;
  static uint32_t stableSinceMs = 0, lastLoadCueMs = 0, mainsStableSinceMs = 0;
  const uint32_t now = millis();
  if (paused) return;
  const bool stateNormal = (st == STATE_NORMAL);
  const bool lowWarn  = (vProtect >= VOLT_UV_CANDIDATE_RAW_MIN_V && vProtect < VOLT_NORMAL_MIN_V);
  const bool highWarn = (vProtect >= VOLT_OV_DELAY_V);
  if (lowWarn) { if (lowSinceMs == 0) lowSinceMs = now; } else lowSinceMs = 0;
  if (highWarn) { if (highSinceMs == 0) highSinceMs = now; } else highSinceMs = 0;
  if (lowSinceMs && (now - lowSinceMs >= 1500U) && (now - lastVoltWarnMs >= 8000U)) { lastVoltWarnMs = now; notification.notify(SND_VOLT_LOW); }
  if (highSinceMs && (now - highSinceMs >= 1000U) && (now - lastVoltWarnMs >= 8000U)) { lastVoltWarnMs = now; notification.notify(SND_VOLT_HIGH); }
  if (!mainsPresent || vProtect < VOLT_UV_DELAY_V || !stateNormal) { loadBaseA = 0.0f; prevIrms = irms; stableSinceMs = 0; mainsStableSinceMs = 0; return; }
  if (mainsStableSinceMs == 0) mainsStableSinceMs = now;
  const float dI = fabsf(irms - prevIrms); prevIrms = irms;
  if (dI < 0.08f) { if (stableSinceMs == 0) stableSinceMs = now; } else stableSinceMs = 0;
  if (stableSinceMs && (now - stableSinceMs >= DEVICE_PLUG_STABLE_MS)) loadBaseA += 0.02f * (irms - loadBaseA);
  const bool cueAllowed = ((now - mainsStableSinceMs) >= DEVICE_PLUG_CUE_INHIBIT_MS);
  const bool firstLoad = (loadBaseA < 0.15f && irms >= 0.35f);
  const bool addedLoad = (loadBaseA >= 0.15f && (irms - loadBaseA) >= 0.45f && dI < 0.15f);
  if (cueAllowed && (firstLoad || addedLoad) && (now - lastLoadCueMs >= DEVICE_PLUG_CUE_COOLDOWN_MS)) {
    notification.notify(SND_DEVICE_PLUG); lastLoadCueMs = now; loadBaseA = irms; stableSinceMs = 0;
  }
}

static bool pushFeatureFrame_(FeatureFrame& frame) {
  if (!qFeat) return false;
  UBaseType_t queued = uxQueueMessagesWaiting(qFeat);
  uint32_t dropped = 0;
  while (queued >= FEATURE_FRAME_QUEUE_LEN) {
    FeatureFrame dump;
    if (xQueueReceive(qFeat, &dump, 0) != pdTRUE) break;
    dropped++;
    queued = uxQueueMessagesWaiting(qFeat);
  }
  frame.queue_drop_count += dropped;
  const BaseType_t ok = xQueueSendToBack(qFeat, &frame, 0);
  return ok == pdPASS;
}

static void drainFeatureQueue_() {
  if (!qFeat) return;
  FeatureFrame dump = {};
  while (xQueueReceive(qFeat, &dump, 0) == pdTRUE) {}
}

static bool pushRawFeatureFrame_(RawFeatureFrame& frame) {
  if (!qRawFeat) return false;
  UBaseType_t queued = uxQueueMessagesWaiting(qRawFeat);
  uint32_t dropped = 0;
  while (queued >= FEATURE_RAW_FRAME_QUEUE_LEN) {
    RawFeatureFrame dump = {};
    if (xQueueReceive(qRawFeat, &dump, 0) != pdTRUE) break;
    dropped++;
    queued = uxQueueMessagesWaiting(qRawFeat);
  }
  frame.queue_drop_count += dropped;
  const BaseType_t ok = xQueueSendToBack(qRawFeat, &frame, 0);
  return ok == pdPASS;
}

static void drainRawFeatureQueue_() {
  if (!qRawFeat) return;
  RawFeatureFrame dump = {};
  while (xQueueReceive(qRawFeat, &dump, 0) == pdTRUE) {}
}

static void sleepUntilUs_(int64_t targetUs) {
  while (true) {
    const int64_t nowUs = esp_timer_get_time();
    const int64_t remainUs = targetUs - nowUs;
    if (remainUs <= 0) break;

    if (remainUs >= 12000) {
      TickType_t ticks = pdMS_TO_TICKS((uint32_t)(remainUs / 1000));
      if (ticks < 1) ticks = 1;
      vTaskDelay(ticks);
    } else if (remainUs >= 150) {
      delayMicroseconds((uint32_t)remainUs);
    } else {
      break;
    }
  }
}

static void onOtaEvent(OtaEvent ev, int progress) {
  static int s_lastDbProgress = -100;
  if (ev == OtaEvent::START) {
    s_lastDbProgress = 0;
    notification.setOta(true, 0);
    notification.notify(SND_OTA_START);
    (void)enterOtaExclusiveMode_();
  } else if (ev == OtaEvent::PROGRESS) {
    const int pct = constrain(progress, 0, 100);
    notification.setOta(true, (uint8_t)pct);
    if (pct == 0 || pct == 100 || (pct - s_lastDbProgress) >= 20) {
      s_lastDbProgress = pct;
    }
  } else if (ev == OtaEvent::SUCCESS) {
    s_lastDbProgress = 100;
    notification.setOta(true, 100);
    notification.notify(SND_OTA_OK);
  } else if (ev == OtaEvent::FAIL) {
    s_lastDbProgress = -100;
    notification.setOta(false, 0);
    notification.notify(SND_OTA_FAIL);
    if (!exitOtaExclusiveMode_()) {
      notification.showStatus("OTA FAILED", "Sense restart err");
      delay(700);
    }
    // FAIL debug/log is handled after exclusive OTA mode exits.
  }
}

static void Core0Task(void* pv) {
  (void)pv;
  gSenseTaskRunning = true;
  uint32_t timingGraceStartMs = 0;
  int64_t prevEmitStartUs = 0;
  uint32_t seenResetEpoch = gFeaturePipelineResetEpoch;
  uint32_t hopSeq = 0;
  size_t windowCount = 0;
  uint8_t emitHopCounter = 0;
  uint8_t emitSinceIdleSlice = 0;
  int64_t hopStartUsBuf[ARC_RUNTIME_FRAME_HOP_COUNT] = {0};
  int64_t hopEndUsBuf[ARC_RUNTIME_FRAME_HOP_COUNT] = {0};
  uint32_t hopStartMsBuf[ARC_RUNTIME_FRAME_HOP_COUNT] = {0};
  uint32_t hopEndMsBuf[ARC_RUNTIME_FRAME_HOP_COUNT] = {0};
  float hopFsBuf[ARC_RUNTIME_FRAME_HOP_COUNT] = {0.0f};
  bool hopWarnBuf[ARC_RUNTIME_FRAME_HOP_COUNT] = {false};

  while (millis() < SENSOR_BOOT_SETTLE_MS) {
    if (gStopSenseTask) {
      gSenseTaskRunning = false;
      gCore0SenseTask = nullptr;
      vTaskDelete(nullptr);
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }

  timingGraceStartMs = millis();

  while (true) {
    if (gStopSenseTask) break;
    if (gPauseByOta || gSafeMode) {
      if (gStopSenseTask) break;
      vTaskDelay(20 / portTICK_PERIOD_MS);
      prevEmitStartUs = 0;
      windowCount = 0;
      emitHopCounter = 0;
      hopSeq = 0;
      timingGraceStartMs = millis();
      continue;
    }

    if (seenResetEpoch != gFeaturePipelineResetEpoch) {
      seenResetEpoch = gFeaturePipelineResetEpoch;
      windowCount = 0;
      emitHopCounter = 0;
      hopSeq = 0;
      prevEmitStartUs = 0;
      memset(hopStartUsBuf, 0, sizeof(hopStartUsBuf));
      memset(hopEndUsBuf, 0, sizeof(hopEndUsBuf));
      memset(hopStartMsBuf, 0, sizeof(hopStartMsBuf));
      memset(hopEndMsBuf, 0, sizeof(hopEndMsBuf));
      memset(hopFsBuf, 0, sizeof(hopFsBuf));
      memset(hopWarnBuf, 0, sizeof(hopWarnBuf));
      drainRawFeatureQueue_();
      drainFeatureQueue_();
      timingGraceStartMs = millis();
      emitSinceIdleSlice = 0;
    }

    const int64_t hopStartUs = esp_timer_get_time();
    const uint32_t hopStartMs = millis();
    float fs_hz = 0.0f;
    size_t got = curSensor.capture(s_hop, ARC_RUNTIME_HOP_SAMPLES, &fs_hz);
    uint32_t hopEndMs = millis();
    int64_t hopEndUs = esp_timer_get_time();

    if (gPauseByOta || gSafeMode) {
      if (gStopSenseTask) break;
      vTaskDelay(20 / portTICK_PERIOD_MS);
      prevEmitStartUs = 0;
      windowCount = 0;
      emitHopCounter = 0;
      hopSeq = 0;
      timingGraceStartMs = millis();
      continue;
    }

    bool sampleRateHardBad = (!isfinite(fs_hz)) || (fs_hz < MCP3204_FS_HARD_LOW_HZ) || (fs_hz > MCP3204_FS_HARD_HIGH_HZ);
    if (got != ARC_RUNTIME_HOP_SAMPLES || sampleRateHardBad) {
      vTaskDelay(1);
      fs_hz = 0.0f;
      got = curSensor.capture(s_hop, ARC_RUNTIME_HOP_SAMPLES, &fs_hz);
      hopEndMs = millis();
      hopEndUs = esp_timer_get_time();
      sampleRateHardBad = (!isfinite(fs_hz)) || (fs_hz < MCP3204_FS_HARD_LOW_HZ) || (fs_hz > MCP3204_FS_HARD_HIGH_HZ);
    }

    if (got != ARC_RUNTIME_HOP_SAMPLES || sampleRateHardBad) {
      continue;
    }

    if (windowCount < ARC_RUNTIME_FRAME_SAMPLES) {
      memcpy(s_raw + windowCount, s_hop, sizeof(uint16_t) * ARC_RUNTIME_HOP_SAMPLES);
      windowCount += ARC_RUNTIME_HOP_SAMPLES;
      if (windowCount > ARC_RUNTIME_FRAME_SAMPLES) windowCount = ARC_RUNTIME_FRAME_SAMPLES;
    } else {
      if (ARC_RUNTIME_FRAME_SAMPLES > ARC_RUNTIME_HOP_SAMPLES) {
        memmove(s_raw, s_raw + ARC_RUNTIME_HOP_SAMPLES, sizeof(uint16_t) * (ARC_RUNTIME_FRAME_SAMPLES - ARC_RUNTIME_HOP_SAMPLES));
      }
      memcpy(s_raw + (ARC_RUNTIME_FRAME_SAMPLES - ARC_RUNTIME_HOP_SAMPLES), s_hop, sizeof(uint16_t) * ARC_RUNTIME_HOP_SAMPLES);
    }

    const uint8_t hopSlot = (uint8_t)(hopSeq % ARC_RUNTIME_FRAME_HOP_COUNT);
    hopStartUsBuf[hopSlot] = hopStartUs;
    hopEndUsBuf[hopSlot] = hopEndUs;
    hopStartMsBuf[hopSlot] = hopStartMs;
    hopEndMsBuf[hopSlot] = hopEndMs;
    hopFsBuf[hopSlot] = fs_hz;
    hopWarnBuf[hopSlot] =
        (!isfinite(fs_hz)) ||
        (fs_hz < MCP3204_FS_WARN_LOW_HZ) ||
        (fs_hz > MCP3204_FS_WARN_HIGH_HZ) ||
        (fabsf(fs_hz - FS_INTENDED_HZ) > FS_ERR_BAD_HZ);
    hopSeq++;

    if (windowCount < ARC_RUNTIME_FRAME_SAMPLES) continue;

    emitHopCounter++;
    if (emitHopCounter < ARC_RUNTIME_EMIT_EVERY_HOPS) continue;
    emitHopCounter = 0;

    RawFeatureFrame rawFrame = {};
    rawFrame.sample_count = ARC_RUNTIME_FRAME_SAMPLES;
    rawFrame.fft_size = ARC_RUNTIME_FRAME_SAMPLES;
    rawFrame.hop_samples = ARC_RUNTIME_HOP_SAMPLES;
    memcpy(rawFrame.samples, s_raw, sizeof(uint16_t) * ARC_RUNTIME_FRAME_SAMPLES);

    const uint8_t oldestSlot = (uint8_t)((hopSeq - ARC_RUNTIME_FRAME_HOP_COUNT) % ARC_RUNTIME_FRAME_HOP_COUNT);
    const uint8_t newestSlot = (uint8_t)((hopSeq - 1U) % ARC_RUNTIME_FRAME_HOP_COUNT);
    rawFrame.frame_start_uptime_ms = hopStartMsBuf[oldestSlot];
    rawFrame.frame_end_uptime_ms = hopEndMsBuf[newestSlot];
    if (prevEmitStartUs > 0 && hopStartUsBuf[oldestSlot] > prevEmitStartUs) {
      rawFrame.frame_dt_ms = float(hopStartUsBuf[oldestSlot] - prevEmitStartUs) * 0.001f;
    }
    prevEmitStartUs = hopStartUsBuf[oldestSlot];

    float fsSum = 0.0f;
    uint8_t fsCount = 0;
    bool sampleRateWarnBad = false;
    for (uint8_t i = 0; i < ARC_RUNTIME_FRAME_HOP_COUNT; ++i) {
      const uint8_t slot = (uint8_t)((hopSeq - ARC_RUNTIME_FRAME_HOP_COUNT + i) % ARC_RUNTIME_FRAME_HOP_COUNT);
      if (hopFsBuf[slot] > 0.0f) {
        fsSum += hopFsBuf[slot];
        fsCount++;
      }
      sampleRateWarnBad = sampleRateWarnBad || hopWarnBuf[slot];
    }
    rawFrame.adc_fs_hz = (fsCount > 0) ? (fsSum / float(fsCount)) : fs_hz;
    rawFrame.fs_err_hz = (rawFrame.adc_fs_hz > 0.0f) ? fabsf(rawFrame.adc_fs_hz - FS_INTENDED_HZ) : fabsf(FS_INTENDED_HZ);

    const bool timingGraceActive = ((hopStartMs - timingGraceStartMs) < FEATURE_TIMING_GRACE_MS);
    const float frameJitterMs = (rawFrame.frame_dt_ms > 0.0f)
        ? fabsf(rawFrame.frame_dt_ms - FEATURE_FRAME_PERIOD_MS)
        : 0.0f;
    const bool frameTimingBad =
        !timingGraceActive &&
        (rawFrame.frame_dt_ms > 0.0f) &&
        ((rawFrame.frame_dt_ms < FRAME_DT_BAD_EARLY_MS) ||
         (rawFrame.frame_dt_ms > FRAME_DT_BAD_LATE_MS) ||
         (frameJitterMs > FRAME_DT_JITTER_BAD_MS));

    rawFrame.sampling_quality_bad = (sampleRateWarnBad || frameTimingBad) ? 1U : 0U;
    (void)pushRawFeatureFrame_(rawFrame);
    emitSinceIdleSlice++;
    if (FEATURE_CAPTURE_IDLE_SLICE_MS > 0U &&
        emitSinceIdleSlice >= FEATURE_CAPTURE_IDLE_SLICE_EVERY_EMITS) {
      emitSinceIdleSlice = 0;
      TickType_t idleTicks = pdMS_TO_TICKS(FEATURE_CAPTURE_IDLE_SLICE_MS);
      if (idleTicks < 1) idleTicks = 1;
      vTaskDelay(idleTicks);
    }
  }

  gSenseTaskRunning = false;
  gCore0SenseTask = nullptr;
  vTaskDelete(nullptr);
}

static void Core1FeatureTask(void* pv) {
  (void)pv;
  gFeatTaskRunning = true;
  ArcDetectionResult out = {};
  uint32_t seenResetEpoch = gFeaturePipelineResetEpoch;

  while (true) {
    if (gStopSenseTask) break;
    if (gPauseByOta || gSafeMode) {
      if (gStopSenseTask) break;
      vTaskDelay(20 / portTICK_PERIOD_MS);
      continue;
    }

    if (seenResetEpoch != gFeaturePipelineResetEpoch) {
      seenResetEpoch = gFeaturePipelineResetEpoch;
      resetArcRuntime_();
      continue;
    }

    RawFeatureFrame rawFrame = {};
    if (!qRawFeat || xQueueReceive(qRawFeat, &rawFrame, pdMS_TO_TICKS(20)) != pdTRUE) {
      continue;
    }

    FeatureFrame f = {};
    f.uptime_ms = rawFrame.frame_end_uptime_ms;
    f.frame_start_uptime_ms = rawFrame.frame_start_uptime_ms;
    f.frame_end_uptime_ms = rawFrame.frame_end_uptime_ms;
    f.epoch_ms = network.epochForUptimeMs(rawFrame.frame_end_uptime_ms);
    f.frame_dt_ms = rawFrame.frame_dt_ms;
    f.adc_fs_hz = rawFrame.adc_fs_hz;
    f.fs_err_hz = rawFrame.fs_err_hz;
    f.fft_size = rawFrame.fft_size;
    f.hop_samples = rawFrame.hop_samples;
    f.queue_drop_count = rawFrame.queue_drop_count;

    const int64_t tComp0 = esp_timer_get_time();
    bool ok = false;
    if (gArcRuntimeMutex && xSemaphoreTake(gArcRuntimeMutex, pdMS_TO_TICKS(40)) == pdTRUE) {
      ok = arcDetect.compute(rawFrame.samples, rawFrame.sample_count, rawFrame.adc_fs_hz, curCalib, MAINS_F0_HZ, out);
      xSemaphoreGive(gArcRuntimeMutex);
    } else {
      ok = arcDetect.compute(rawFrame.samples, rawFrame.sample_count, rawFrame.adc_fs_hz, curCalib, MAINS_F0_HZ, out);
    }
    const int64_t tComp1 = esp_timer_get_time();
    f.compute_time_ms = (tComp1 > tComp0) ? float(tComp1 - tComp0) * 0.001f : 0.0f;
    f.feature_compute_end_uptime_ms = millis();

    if (ok && out.current_valid) {
      f.adc_fs_hz = out.fs_hz;
      f.fs_err_hz = fabsf(f.adc_fs_hz - FS_INTENDED_HZ);
      f.irms = out.irms_a;
      f.current_valid = 1;
      f.feat_valid = out.feat_valid ? 1U : 0U;
      copyComputedFeaturesToFrame_(f, out);
    } else {
      f.current_valid = 0;
      f.feat_valid = 0;
      f.irms = 0.0f;
      zeroComputedFeatures_(f);
    }

    const bool computeTimingBad = (f.compute_time_ms > FRAME_COMPUTE_BAD_MS);
    f.sampling_quality_bad = rawFrame.sampling_quality_bad || computeTimingBad || !ok;
    (void)pushFeatureFrame_(f);
  }

  gFeatTaskRunning = false;
  gCore1FeatTask = nullptr;
  vTaskDelete(nullptr);
}



static void pollMlControl(FirebaseNetwork& network, Notification& notifyUi) {
  static uint32_t lastPoll = 0;
  static bool lastEnabled = false;
  if (millis() - lastPoll < ML_CONTROL_POLL_MS) return;
  lastPoll = millis();

  bool enabled = false;
  bool trustedNormal = false;
  int dur = ML_LOG_DURATION_S;
  int labelOv = ML_UNKNOWN_LABEL;
  int trialNumber = 1;
  String sid = "";
  String load = "unknown";
  String deviceFamily = "mixed_unknown";
  String deviceName = "unknown_device";
  String divisionTag = "steady";
  String notes = "";
  String requestToken = "";
  (void)network.fetchMlControl(enabled, dur, labelOv, sid, load,
                               deviceFamily, deviceName, trialNumber,
                               divisionTag, notes, trustedNormal, requestToken);

  if (dur < ML_LOG_MIN_DURATION_S) dur = ML_LOG_MIN_DURATION_S;
  if (dur > ML_LOG_MAX_DURATION_S) dur = ML_LOG_MAX_DURATION_S;
  if (trialNumber < 1) trialNumber = 1;
  if (enabled && sid.length() < 3) enabled = false;

  network.setLogDurationSeconds((uint16_t)dur);
  network.setLogSession(sid, load, labelOv,
                        deviceFamily, deviceName, trialNumber,
                        divisionTag, notes, trustedNormal);
  network.setLogEnabled(enabled);
  if (enabled && !lastEnabled) {
    notification.notify(SND_LOGGER_ON);
    notifyUi.triggerCollecting(1000);
    if (requestToken.length()) (void)network.publishControlAck("ml_log_enable", requestToken);
  }
  lastEnabled = enabled;
}

void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);
  notification.begin(PIN_BUZZER_PWM);
  notification.startBootSequence(3600);
  protection.begin(PIN_LATCH_ON, PIN_LATCH_OFF);
  notification.notify(SND_BOOT);

  voltSensor.begin();
  voltSensor.setWindowMs(650);
  voltSensor.setClampHysteresis(12.0f, 24.0f);
  voltSensor.setLongAverage(4.6f, 12.0f);
  voltSensor.setCubicCalib(VOLTAGE_CAL_C3, VOLTAGE_CAL_C2, VOLTAGE_CAL_C1, VOLTAGE_CAL_C0);

  curCalib.cubic3 = CURRENT_CAL_C3; curCalib.cubic2 = CURRENT_CAL_C2; curCalib.cubic1 = CURRENT_CAL_C1; curCalib.cubic0 = CURRENT_CAL_C0;
  curSensor.setCalib(curCalib);
  tempSensor.begin(); tempSensor.setLongAverage(8.0f, 1.0f);

  network.begin(FIREBASE_API_KEY, FIREBASE_DB_URL);
  network.setFirmwareVersion(FW_VERSION);
  network.setNormalIntervalMs(CLOUD_LIVE_NORMAL_INTERVAL_MS);
  network.setFaultIntervalMs(CLOUD_LIVE_FAULT_INTERVAL_MS);

  updater.begin(FW_VERSION, &network, 45000, 3);
  updater.setEventCallback(onOtaEvent);
  updater.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  updater.setCheckInterval(0UL);
  gSafeMode = updater.safeMode();

  if (!gSafeMode) {
    if (startSensePipeline_()) {
      notification.showStatus("Sensors", "MCP3204 ready");
    } else {
      notification.showStatus("WARN", allocSampleBuffer_() ? "No MCP3204" : "No sample RAM");
      delay(700);
    }
  } else {
    notification.showStatus("SAFE MODE", "OTA only");
  }

  wifiMgr.begin([](WiFiManager* wm) { (void)wm; });
  network.setLogEnabled(false);
  network.setLogDurationSeconds(ML_LOG_DURATION_S);
  resetContextTracker_();
}

void loop() {
  network.updateClock();
  wifiMgr.update();

  const bool portalActive = wifiMgr.inConfigPortal();
  const bool paused = gPauseByOta;

  static String s_lastOtaErr = String("__BOOT__");
  const String otaErr = updater.lastError();
  if (!paused && otaErr != s_lastOtaErr) {
    s_lastOtaErr = otaErr;
    if (otaErr.length()) {
      (void)network.publishOtaDebug("FAIL", otaErr, -1);
    } else {
      (void)network.publishOtaDebug("IDLE", "", -1);
    }
  } else if (paused) {
    s_lastOtaErr = otaErr;
  }
  const bool bootSettling = (millis() < SENSOR_BOOT_SETTLE_MS);
  const bool protectionInhibit = (millis() < (SENSOR_BOOT_SETTLE_MS + PROTECTION_INHIBIT_MS));

  static WifiHandler::Phase lastWiFiPhase = WifiHandler::PHASE_BOOT_CONNECT;
  static bool lastWiFiConnected = false;
  static uint32_t wifiBannerUntilMs = 0;
  static uint32_t wifiCloudReadyAtMs = 0;
  static bool wifiTimedOutUi = false;
  const auto wifiPhase = wifiMgr.phase();
  const bool wifiConnected = wifiMgr.isConnected();
  if (wifiPhase != lastWiFiPhase) {
    if (wifiPhase == WifiHandler::PHASE_CONNECTING) { wifiBannerUntilMs = millis() + 2500UL; wifiTimedOutUi = false; }
    else if (wifiPhase == WifiHandler::PHASE_TIMEOUT) { wifiBannerUntilMs = millis() + 2500UL; wifiTimedOutUi = true; }
    else if (wifiPhase == WifiHandler::PHASE_CONNECTED) { wifiBannerUntilMs = 0; wifiTimedOutUi = false; notification.triggerConnected(1500UL); }
    else if (wifiPhase == WifiHandler::PHASE_AP_WAIT_CLIENT) { wifiBannerUntilMs = 0; wifiTimedOutUi = false; notification.notify(SND_WIFI_PORTAL); }
    lastWiFiPhase = wifiPhase;
  }
  if (wifiConnected && !lastWiFiConnected) {
    notification.notify(SND_WIFI_OK);
    wifiCloudReadyAtMs = millis() + CLOUD_WIFI_WARMUP_MS;
  } else if (!wifiConnected) {
    wifiCloudReadyAtMs = 0;
  }
  lastWiFiConnected = wifiConnected;
  const bool wifiCloudWarm = wifiConnected &&
      (wifiCloudReadyAtMs == 0U || (int32_t)(millis() - wifiCloudReadyAtMs) >= 0);

  static FeatureFrame lastF = {};
  static bool hasLast = false;
  static uint32_t lastFeatRxMs = 0;
  bool freshFeatThisLoop = false;
  FeatureFrame f;
  if (qFeat) {
    bool gotAny = false;
    FeatureFrame newest = {};
    while (xQueueReceive(qFeat, &newest, 0) == pdTRUE) {
      f = newest;
      gotAny = true;
    }
    if (gotAny) {
      lastF = f;
      hasLast = true;
      lastFeatRxMs = millis();
      freshFeatThisLoop = true;
    } else if (hasLast) {
      f = lastF;
      if ((millis() - lastFeatRxMs) > FEAT_STALE_MS) {
        f.irms = 0.0f; f.current_valid = 0; f.feat_valid = 0; f.model_pred = 0; f.adc_fs_hz = 0.0f;
        f.fs_err_hz = fabsf(FS_INTENDED_HZ); f.sampling_quality_bad = 1;
        zeroComputedFeatures_(f);
        f.invalid_loaded_flag = 0; f.invalid_off_flag = 1;
        f.relay_blank_active = 0; f.turnon_blank_active = 0; f.transient_blank_active = 0;
        lastF = f;
      }
    } else {
      memset(&f, 0, sizeof(f));
      f.uptime_ms = millis();
      f.fft_size = ARC_RUNTIME_FRAME_SAMPLES;
      f.hop_samples = ARC_RUNTIME_HOP_SAMPLES;
    }
  } else if (hasLast) {
    f = lastF;
    if ((millis() - lastFeatRxMs) > FEAT_STALE_MS) {
      f.irms = 0.0f; f.current_valid = 0; f.feat_valid = 0; f.model_pred = 0; f.adc_fs_hz = 0.0f;
      f.fs_err_hz = fabsf(FS_INTENDED_HZ); f.sampling_quality_bad = 1;
      zeroComputedFeatures_(f);
      f.invalid_loaded_flag = 0; f.invalid_off_flag = 1;
      f.relay_blank_active = 0; f.turnon_blank_active = 0; f.transient_blank_active = 0;
      lastF = f;
    }
  } else {
    memset(&f, 0, sizeof(f));
    f.uptime_ms = millis();
    f.fft_size = ARC_RUNTIME_FRAME_SAMPLES;
    f.hop_samples = ARC_RUNTIME_HOP_SAMPLES;
  }

  static float vRms = 0.0f, vFast = 0.0f, vRaw = 0.0f, tC = 0.0f;
  static uint32_t tTemp = 0;
  float newV = voltSensor.update();
  if (newV >= 0.0f) { vRms = newV; vFast = voltSensor.protectVrms(); vRaw = voltSensor.rawVrms(); }
  if (millis() - tTemp >= 500) { tTemp = millis(); float newT = tempSensor.readTempC(); if (newT > -50.0f && newT < 150.0f) tC = newT; }

  static bool lastMainsPresentForFeat = false;
  const bool mainsPresentForFeat = (vFast >= MAINS_PRESENT_ON_V);
  if (mainsPresentForFeat != lastMainsPresentForFeat) {
    hasLast = false;
    lastFeatRxMs = 0;
    memset(&lastF, 0, sizeof(lastF));
    requestFeaturePipelineReset_();
    resetArcRuntime_();
    resetContextTracker_();
    if (mainsPresentForFeat) armRelayArtifactBlank_(1200UL);
  }
  lastMainsPresentForFeat = mainsPresentForFeat;

  f.vrms = vRms; f.temp_c = tC;
  const float irmsRawMeasured = f.irms;
  float irmsRawForLogic = cleanLogicCurrent(irmsRawMeasured, f.current_valid != 0, vRaw, vFast);

  if (vFast <= MAINS_PRESENT_OFF_V) {
    clearManualRelayAssume_();
  }

  const uint32_t manualNowMs = millis();
  const bool manualAssistHoldoffActive = ((int32_t)(gManualRelayWebHoldoffUntilMs - manualNowMs) > 0);
  const bool manualAssistCooldownActive = ((int32_t)(gManualRelayCooldownUntilMs - manualNowMs) > 0);
  const bool manualRelayRearmSignal =
      (!protection.relayLatchedOn()) &&
      !manualAssistHoldoffActive &&
      !manualAssistCooldownActive &&
      (vFast >= MAINS_PRESENT_ON_V) &&
      (f.current_valid != 0) &&
      (irmsRawForLogic >= MANUAL_RELAY_REARM_MIN_A);

  if (!gManualRelayConfirmPending) {
    if (manualRelayRearmSignal) {
      if (gManualRelayCandidateSinceMs == 0) gManualRelayCandidateSinceMs = manualNowMs;
      if ((manualNowMs - gManualRelayCandidateSinceMs) >= MANUAL_RELAY_REARM_DEBOUNCE_MS) {
        const bool wasLatched = protection.relayLatchedOn();
        protection.pulseRelayOn();
        const bool pulseSent = protection.relayPulseActive() || (protection.relayLatchedOn() != wasLatched);
        if (pulseSent) {
          String localRelayToken("relay_on_local_");
          localRelayToken += String((uint32_t)manualNowMs);
          gManualRelayAssumedOn = true;
          gManualRelayConfirmPending = true;
          gManualRelayCandidateSinceMs = 0;
          gManualRelayReleaseSinceMs = 0;
          gManualRelayConfirmedActiveMs = 0;
          gManualRelayConfirmStartedMs = manualNowMs;
          gManualRelayCooldownUntilMs = 0;
          armRelayArtifactBlank_(MANUAL_RELAY_REARM_BLANK_MS);
          (void)network.publishRelayPulseEvent("relay_on", localRelayToken, "physical_confirm");
          notification.notify(SND_RESET_ACK);
        }
      }
    } else {
      gManualRelayCandidateSinceMs = 0;
    }
  } else {
    const bool assistCurrentActive =
        (vFast >= MAINS_PRESENT_ON_V) &&
        (f.current_valid != 0) &&
        (irmsRawForLogic >= MANUAL_RELAY_REARM_MIN_A);
    if (assistCurrentActive) {
      if (gManualRelayConfirmStartedMs == 0) gManualRelayConfirmStartedMs = manualNowMs;
      gManualRelayReleaseSinceMs = 0;
      const uint32_t activeBase = (gManualRelayCandidateSinceMs != 0) ? gManualRelayCandidateSinceMs : manualNowMs;
      const uint32_t elapsed = (manualNowMs >= activeBase) ? (manualNowMs - activeBase) : 0;
      gManualRelayCandidateSinceMs = manualNowMs;
      gManualRelayConfirmedActiveMs += elapsed;
      if (gManualRelayConfirmedActiveMs >= MANUAL_RELAY_REARM_CONFIRM_MS) {
        gManualRelayConfirmPending = false;
        gManualRelayAssumedOn = false;
        gManualRelayCandidateSinceMs = 0;
        gManualRelayReleaseSinceMs = 0;
        gManualRelayConfirmedActiveMs = MANUAL_RELAY_REARM_CONFIRM_MS;
      }
    } else {
      gManualRelayCandidateSinceMs = 0;
      if (gManualRelayReleaseSinceMs == 0) gManualRelayReleaseSinceMs = manualNowMs;
      else if ((manualNowMs - gManualRelayReleaseSinceMs) >= MANUAL_RELAY_REARM_RELEASE_MS) {
        const bool wasLatched = protection.relayLatchedOn();
        protection.pulseRelayOff();
        const bool pulseSent = protection.relayPulseActive() || (protection.relayLatchedOn() != wasLatched);
        if (pulseSent) {
          String localRelayToken("relay_off_local_");
          localRelayToken += String((uint32_t)manualNowMs);
          (void)network.publishRelayPulseEvent("relay_off", localRelayToken, "physical_rollback");
          notification.notify(SND_RESET_ACK);
          armRelayArtifactBlank_();
        }
        clearManualRelayAssume_();
        gManualRelayCooldownUntilMs = manualNowMs + MANUAL_RELAY_REARM_COOLDOWN_MS;
      }
    }
  }

  const bool effectiveRelayLatchedOn = protection.relayLatchedOn() || gManualRelayAssumedOn;
  const bool featureBridgeUsed = stabilizeFeatureValidity(f, vFast, irmsRawForLogic, !effectiveRelayLatchedOn);
  (void)featureBridgeUsed;

  const bool relayArtifactBlankActive =
      (!effectiveRelayLatchedOn) &&
      ((int32_t)(gRelayArtifactBlankUntilMs - millis()) > 0);
  if (relayArtifactBlankActive && irmsRawForLogic <= RELAY_ARTIFACT_FORCE_ZERO_A) {
    irmsRawForLogic = 0.0f;
    f.irms = 0.0f;
    f.current_valid = 0;
    f.feat_valid = 0;
    f.model_pred = 0;
    zeroComputedFeatures_(f);
  }

  static uint32_t lowIrmsSinceMs = 0;
  const bool mainsOnForIdle = (vFast >= MAINS_PRESENT_ON_V);
  const bool lowIrmsNow = (fabsf(irmsRawForLogic) <= CURRENT_ANALYSIS_IDLE_A);
  if (mainsOnForIdle && lowIrmsNow) {
    if (lowIrmsSinceMs == 0) lowIrmsSinceMs = millis();
  } else {
    lowIrmsSinceMs = 0;
  }
  const bool longIdleLowCurrent = (lowIrmsSinceMs != 0) && ((millis() - lowIrmsSinceMs) >= CURRENT_IDLE_SUPPRESS_HOLD_MS);

  float irmsDisplayIn = irmsRawForLogic;
  if (notification.shouldSuppressCurrentArtifacts() && longIdleLowCurrent && irmsDisplayIn < BUZZER_ARTIFACT_MAX_A) {
    irmsDisplayIn = 0.0f;
  }
  f.irms = cleanDisplayCurrent(irmsDisplayIn, f.current_valid != 0, f.feat_valid != 0, vRaw, vFast);

  const float apparentPowerVa = (vRms > 0.10f && f.irms > 0.001f) ? (vRms * f.irms) : 0.0f;
  const bool voltageNormal = (vFast >= VOLT_NORMAL_MIN_V && vFast <= VOLT_NORMAL_MAX_V);
  const bool arcTurnOnBlankActive =
      arcTurnOnBlanking(effectiveRelayLatchedOn,
                        f.current_valid != 0,
                        vFast,
                        irmsRawForLogic);

  const bool arcTransientBlankActive =
      arcTransientBlanking(effectiveRelayLatchedOn,
                           f.current_valid != 0,
                           vFast,
                           irmsRawForLogic);

  const bool arcBlankActive =
      arcTurnOnBlankActive || arcTransientBlankActive;
  const bool mlEventLike = arcEventSignature(f, irmsRawForLogic);

  static float loadRefA = 0.0f;
  static float prevIrmsLogic = 0.0f;
  static float prevVFast = 0.0f;
  static float prevFlux = 0.0f;
  static float prevHfDelta = 0.0f;
  static float healthyVoltageBaseline = 0.0f;
  static uint32_t invalidBurstSinceMs = 0;
  static uint32_t collapseSinceMs = 0;
  static uint32_t voltDipSinceMs = 0;
  static uint32_t faultClearSuppressUntilMs = 0;
  static uint16_t suspiciousRunLen = 0;
  static uint16_t invalidLoadedRunLen = 0;
  static float suspiciousRunEnergy = 0.0f;
  static int arcLeakyScore = 0;
  static bool prevSuspiciousFrame = false;
  static uint32_t restrikeTimes[6] = {0,0,0,0,0,0};
  static uint8_t restrikeHead = 0;

  if (effectiveRelayLatchedOn && vFast >= MAINS_PRESENT_ON_V && irmsRawForLogic >= 0.50f) {
    loadRefA += 0.10f * (irmsRawForLogic - loadRefA);
  } else if (!effectiveRelayLatchedOn || vFast <= MAINS_PRESENT_OFF_V) {
    loadRefA = 0.0f;
  }

  const bool hadSteadyLoad = (loadRefA >= 0.80f);
  const bool energizedArcWindow = (vFast >= 170.0f);
  const bool invalidWhileLoaded = energizedArcWindow && hadSteadyLoad && (f.feat_valid == 0);
  const bool invalidOff = (!energizedArcWindow) && (f.feat_valid == 0) && (f.current_valid == 0);
  const bool irmsCollapse =
      energizedArcWindow &&
      hadSteadyLoad &&
      (prevIrmsLogic >= 0.80f) &&
      (irmsRawForLogic <= 0.05f);
  const bool fastVoltDip =
      hadSteadyLoad &&
      (prevVFast >= VOLT_NORMAL_MIN_V) &&
      ((prevVFast - vFast) >= 20.0f);

  if (voltageNormal && effectiveRelayLatchedOn && !arcBlankActive) {
    if (healthyVoltageBaseline <= 1.0f) healthyVoltageBaseline = vFast;
    else healthyVoltageBaseline += 0.03f * (vFast - healthyVoltageBaseline);
  } else if (vFast <= MAINS_PRESENT_OFF_V) {
    healthyVoltageBaseline = 0.0f;
  }

  f.delta_irms_abs = fabsf(irmsRawForLogic - prevIrmsLogic);
  f.delta_hf_energy = fabsf(f.hf_energy_delta - prevHfDelta);
  f.delta_flux = fabsf(f.spectral_flux_midhf - prevFlux);
  f.v_sag_pct = (healthyVoltageBaseline > 10.0f) ? fmaxf(0.0f, ((healthyVoltageBaseline - vFast) / healthyVoltageBaseline) * 100.0f) : 0.0f;
  // Hold the temporal energy feature at the exact value consumed this frame so
  // runtime contexting, arc inference, logging, and retraining see the same input.
  f.suspicious_run_energy =
      tinymlClampFeatureValue(TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY, suspiciousRunEnergy);
  f.invalid_loaded_flag = invalidWhileLoaded ? 1 : 0;
  f.invalid_off_flag = invalidOff ? 1 : 0;
  f.relay_blank_active = relayArtifactBlankActive ? 1 : 0;
  f.turnon_blank_active = arcTurnOnBlankActive ? 1 : 0;
  f.transient_blank_active = arcTransientBlankActive ? 1 : 0;
  sanitizeFeatureFrame_(f);

  if (invalidWhileLoaded) {
    if (invalidBurstSinceMs == 0) invalidBurstSinceMs = millis();
    if (invalidLoadedRunLen < 65535U) invalidLoadedRunLen++;
  } else {
    invalidBurstSinceMs = 0;
    invalidLoadedRunLen = 0;
  }

  if (irmsCollapse) {
    if (collapseSinceMs == 0) collapseSinceMs = millis();
  } else {
    collapseSinceMs = 0;
  }

  if (fastVoltDip) {
    if (voltDipSinceMs == 0) voltDipSinceMs = millis();
  } else {
    voltDipSinceMs = 0;
  }

  const bool fallbackArcEvent =
      effectiveRelayLatchedOn &&
      (((invalidBurstSinceMs && (millis() - invalidBurstSinceMs) >= 80UL)) ||
       ((collapseSinceMs && (millis() - collapseSinceMs) >= 40UL)) ||
       ((voltDipSinceMs && (millis() - voltDipSinceMs) >= 40UL)));

  const bool softFallbackArcEvent =
      effectiveRelayLatchedOn &&
      !arcBlankActive &&
      arcInputStable(f.current_valid != 0, irmsRawForLogic) &&
      softArcBurstEvent(f, irmsRawForLogic, hadSteadyLoad, vFast);

  const bool haveUsableFeatures = (f.feat_valid != 0);
  const bool mlArcEligible =
      (!gSafeMode &&
       !paused &&
       !bootSettling &&
       !protectionInhibit &&
       !arcBlankActive &&
       voltageNormal &&
       haveUsableFeatures &&
       arcInputStable(f.current_valid != 0, irmsRawForLogic));

  static uint32_t relayActionAtMs = 0;
  static uint32_t relayNetQuietUntilMs = 0;
  static uint32_t controlPollMuteUntilMs = 0;
  static bool pendingProtectionTripOff = false;
  static bool pendingProtectionAutoOn = false;
  bool relayNetQuietActive = ((int32_t)(relayNetQuietUntilMs - millis()) > 0);
  const bool controlPollMuted = ((int32_t)(controlPollMuteUntilMs - millis()) > 0);

  network.pollControls(!paused && !portalActive && wifiCloudWarm && !relayNetQuietActive && !controlPollMuted, portalActive);
  if (paused || gSafeMode) network.setLogEnabled(false);
  else if (!relayNetQuietActive) pollMlControl(network, notification);
  const bool collectionModeActive = network.manualEnabled();

  const bool faultClearRequested = (!gSafeMode) ? network.consumeFaultClearRequest() : false;
  const bool revertFirmwareRequested = network.consumeRevertFirmwareRequest();
  const bool otaCheckRequested = network.consumeOtaCheckRequest();
  if (otaCheckRequested && !portalActive && wifiCloudWarm) {
    (void)network.publishOtaDebug("CHECKING", "Manual OTA check requested", -1);
    updater.requestCheckNow();
  }
  updater.loop();
  if (faultClearRequested) {
    clearManualRelayAssume_();
    protection.resetLatch();
    armRelayArtifactBlank_();
    faultClearSuppressUntilMs = millis() + FAULT_NET_QUIET_MS;
    loadRefA = 0.0f;
    invalidBurstSinceMs = 0;
    collapseSinceMs = 0;
    voltDipSinceMs = 0;
    suspiciousRunLen = 0;
    invalidLoadedRunLen = 0;
    suspiciousRunEnergy = 0.0f;
    arcLeakyScore = 0;
    prevSuspiciousFrame = false;
    memset(restrikeTimes, 0, sizeof(restrikeTimes));
    restrikeHead = 0;
    healthyVoltageBaseline = 0.0f;
    prevFlux = f.spectral_flux_midhf;
    prevHfDelta = f.hf_energy_delta;
    prevIrmsLogic = irmsRawForLogic;
    prevVFast = vFast;
    requestFeaturePipelineReset_();
    resetArcRuntime_();
    resetContextTracker_();
    (void)stabilizeFeatureValidity(f, vFast, irmsRawForLogic, true);
    notification.notify(SND_RESET_ACK);
    notification.clearFaultAlert();
  }
  if (revertFirmwareRequested) {
    network.logStatusEvent("FIRMWARE REVERT REQUESTED", 0.0f, 0.0f, 0.0f, 0.0f);
    if (!updater.rollbackToPrevious()) {
      network.logStatusEvent("FIRMWARE REVERT FAILED", 0.0f, 0.0f, 0.0f, 0.0f);
    }
  }

  const bool faultClearSuppressActive = ((int32_t)(faultClearSuppressUntilMs - millis()) > 0);

  updateContextTracker_(f, vFast, irmsRawForLogic, arcBlankActive || relayArtifactBlankActive);
  applyContextToFrame_(f);
  sanitizeFeatureFrame_(f);

  int rawPred = 0;
  if (!faultClearSuppressActive && mlArcEligible) {
    rawPred = arcDetect.predictWithContext(f.spectral_flux_midhf,
                                           f.residual_crest_factor,
                                           f.edge_spike_ratio,
                                           f.midband_residual_ratio,
                                           f.cycle_nmse,
                                           f.peak_fluct_cv,
                                           f.thd_i,
                                           f.hf_energy_delta,
                                           f.zcv,
                                           f.abs_irms_zscore_vs_baseline,
                                           f.suspicious_run_energy,
                                           f.delta_irms_abs,
                                           f.delta_hf_energy,
                                           f.delta_flux,
                                           f.halfcycle_asymmetry,
                                           f.v_sag_pct,
                                           f.vrms,
                                           irmsRawForLogic,
                                           f.temp_c,
                                           f.context_family_code_runtime,
                                           f.context_family_confidence);
    if (rawPred == 1 && !mlEventLike) rawPred = 0;
  }

  const bool temporalKick =
      !arcBlankActive &&
      effectiveRelayLatchedOn &&
      hadSteadyLoad &&
      ((f.invalid_loaded_flag != 0 && invalidLoadedRunLen >= 2U) ||
       (f.delta_flux >= 4.0f && f.delta_hf_energy >= 0.70f) ||
       (f.v_sag_pct >= 3.0f && f.delta_flux >= 3.0f) ||
       (f.halfcycle_asymmetry >= 10.0f && f.delta_irms_abs >= 0.12f));

  bool suspiciousFrame =
      ((rawPred == 1) || fallbackArcEvent || softFallbackArcEvent || temporalKick);
  if (arcBlankActive || relayArtifactBlankActive || invalidOff) suspiciousFrame = false;

  if (suspiciousFrame) {
    if (suspiciousRunLen < 65535U) suspiciousRunLen++;
    suspiciousRunEnergy = fminf(12.0f, suspiciousRunEnergy * 0.82f + 1.10f + (temporalKick ? 0.25f : 0.0f));
    arcLeakyScore = (arcLeakyScore + ARC_LEAKY_SCORE_STEP > ARC_LEAKY_SCORE_MAX) ? ARC_LEAKY_SCORE_MAX : (arcLeakyScore + ARC_LEAKY_SCORE_STEP);
  } else {
    if (suspiciousRunLen > 0) suspiciousRunLen--;
    suspiciousRunEnergy *= invalidOff ? 0.45f : 0.72f;
    arcLeakyScore = (arcLeakyScore > ARC_LEAKY_SCORE_DECAY) ? (arcLeakyScore - ARC_LEAKY_SCORE_DECAY) : 0;
  }

  if (suspiciousFrame && !prevSuspiciousFrame) {
    restrikeTimes[restrikeHead % 6U] = millis();
    restrikeHead = (uint8_t)((restrikeHead + 1U) % 6U);
  }
  prevSuspiciousFrame = suspiciousFrame;

  uint8_t restrikeCountShort = 0;
  for (uint8_t i = 0; i < 6U; ++i) {
    if (restrikeTimes[i] != 0 && (millis() - restrikeTimes[i]) <= 1200UL) restrikeCountShort++;
  }

  int pred = 0;
  if (!faultClearSuppressActive && !arcBlankActive) {
    const bool leakyArmed = (arcLeakyScore >= ARC_LEAKY_SCORE_FIRE);
    // Keep logging suspicious_run_energy for future dataset analysis, but do not
    // let its direct magnitude force a live arc decision path.
    const bool sustainedSuspicion = (suspiciousRunLen >= 2U) || leakyArmed;
    const bool hardContextBurst = fallbackArcEvent || (softFallbackArcEvent && leakyArmed);
    const bool temporalBurst =
        temporalKick &&
        ((arcLeakyScore >= ARC_LEAKY_SCORE_STRONG) || (invalidLoadedRunLen >= 3U) || (restrikeCountShort >= 2U));
    if (hardContextBurst || temporalBurst || (rawPred == 1 && sustainedSuspicion)) pred = 1;
  }

  f.suspicious_run_len = suspiciousRunLen;
  f.invalid_loaded_run_len = invalidLoadedRunLen;
  f.restrike_count_short = restrikeCountShort;
  f.model_pred = (uint8_t)pred;

  FaultState st = STATE_NORMAL;
  if (!bootSettling && !faultClearSuppressActive) {
    const bool arcProtectionEligible =
        !collectionModeActive &&
        !arcBlankActive &&
        (mlArcEligible || fallbackArcEvent || softFallbackArcEvent || temporalKick);
    st = protection.update(vFast,
                           vRaw,
                           tC,
                           irmsRawForLogic,
                           collectionModeActive ? 0 : f.model_pred,
                           arcProtectionEligible);
  }
  if (gSafeMode) st = STATE_NORMAL;

  prevIrmsLogic = irmsRawForLogic;
  prevVFast = vFast;
  prevFlux = f.spectral_flux_midhf;
  prevHfDelta = f.hf_energy_delta;

  static uint32_t noPowerSinceMsCtl = 0;
  const bool unpluggedLiveCtl = classifyUnpluggedSocket(vRaw, vFast, irmsRawForLogic, f.current_valid != 0, st, &noPowerSinceMsCtl);
  const bool controlsLocked = gSafeMode || paused || bootSettling || protectionInhibit || unpluggedLiveCtl || protection.webControlLocked() || protection.voltageLockoutActive();
  const bool portalRequested = (!gSafeMode) ? network.consumePortalRequest() : false;
  String relayOnToken = "";
  String relayOffToken = "";
  const bool relayOnRequested = (!gSafeMode) ? network.consumeRelayOnRequest(&relayOnToken) : false;
  const bool relayOffRequested = (!gSafeMode) ? network.consumeRelayOffRequest(&relayOffToken) : false;
  if (!gSafeMode) {
    if (portalRequested && !controlsLocked) wifiMgr.requestPortal(true);
    if (!controlsLocked) {
      if (relayOffRequested) {
        clearManualRelayAssume_();
        gManualRelayCooldownUntilMs = millis() + MANUAL_RELAY_REARM_COOLDOWN_MS;
        gManualRelayWebHoldoffUntilMs = millis() + MANUAL_RELAY_WEB_HOLDOFF_MS;
        const bool wasLatched = protection.relayLatchedOn();
        protection.pulseRelayOff();
        const bool pulseSent = protection.relayPulseActive() || (protection.relayLatchedOn() != wasLatched);
        if (pulseSent) (void)network.publishRelayPulseEvent("relay_off", relayOffToken, "web");
        else (void)network.publishControlAck("relay_off", relayOffToken);
        armRelayArtifactBlank_();
        notification.notify(SND_RESET_ACK);
      } else if (relayOnRequested) {
        clearManualRelayAssume_();
        gManualRelayWebHoldoffUntilMs = millis() + MANUAL_RELAY_WEB_HOLDOFF_MS;
        const bool wasLatched = protection.relayLatchedOn();
        protection.pulseRelayOn();
        const bool pulseSent = protection.relayPulseActive() || (protection.relayLatchedOn() != wasLatched);
        if (pulseSent) (void)network.publishRelayPulseEvent("relay_on", relayOnToken, "web");
        else (void)network.publishControlAck("relay_on", relayOnToken);
        armRelayArtifactBlank_(1200UL);
        notification.notify(SND_RESET_ACK);
      }
    }
  }

  const bool tripOffEdge = protection.consumeTripOffEdge();
  const bool autoOnEdge  = protection.consumeAutoOnEdge();

#if PROTECTION
  const bool protectionEnabled = true;
#else
  const bool protectionEnabled = false;
#endif

  if (protectionEnabled && tripOffEdge) {
    pendingProtectionTripOff = true;
    pendingProtectionAutoOn = false;
    relayActionAtMs = millis();
    if ((int32_t)(relayNetQuietUntilMs - (relayActionAtMs + FAULT_NET_QUIET_MS)) < 0) {
      relayNetQuietUntilMs = relayActionAtMs + FAULT_NET_QUIET_MS;
    }
  }
  if (protectionEnabled && autoOnEdge && !controlsLocked && !pendingProtectionTripOff) {
    pendingProtectionAutoOn = true;
    relayActionAtMs = millis();
    if ((int32_t)(relayNetQuietUntilMs - (relayActionAtMs + 400UL)) < 0) relayNetQuietUntilMs = relayActionAtMs + 400UL;
  }

  if (pendingProtectionTripOff && (int32_t)(millis() - relayActionAtMs) >= 0) {
    clearManualRelayAssume_();
    gManualRelayCooldownUntilMs = millis() + MANUAL_RELAY_REARM_COOLDOWN_MS;
    protection.pulseRelayOff();
    armRelayArtifactBlank_();
    pendingProtectionTripOff = false;
  }
  if (pendingProtectionAutoOn && (int32_t)(millis() - relayActionAtMs) >= 0) {
    clearManualRelayAssume_();
    gManualRelayWebHoldoffUntilMs = millis() + MANUAL_RELAY_WEB_HOLDOFF_MS;
    protection.pulseRelayOn();
    armRelayArtifactBlank_(1200UL);
    pendingProtectionAutoOn = false;
  }

  relayNetQuietActive = ((int32_t)(relayNetQuietUntilMs - millis()) > 0);

  if (st != STATE_NORMAL || tripOffEdge || relayNetQuietActive) {
    const uint32_t muteUntil = millis() + FAULT_NET_QUIET_MS;
    if ((int32_t)(controlPollMuteUntilMs - muteUntil) < 0) controlPollMuteUntilMs = muteUntil;
  }

  if (tripOffEdge && !paused && !gSafeMode) {
    FeatureFrame fTrip = f;
    fTrip.irms = irmsRawForLogic;
    fTrip.log_enqueue_uptime_ms = millis();
    fTrip.timing_skew_ms = (fTrip.log_enqueue_uptime_ms >= fTrip.frame_start_uptime_ms)
        ? float(fTrip.log_enqueue_uptime_ms - fTrip.frame_start_uptime_ms)
        : 0.0f;
    network.ingestLog(fTrip, st, collectionModeActive ? 0 : protection.arcCounter());
  }

  protection.apply(st, vRms, vFast, irmsRawForLogic, tC);
  notification.updateBuzzer(st, vFast, irmsRawForLogic, tC);
  const bool mainsPresentStable = debouncedMainsPresentForState(vFast);
#if PROTECTION
  handleCueEvents(vRaw, vFast, irmsRawForLogic, mainsPresentStable, paused || gSafeMode || protectionInhibit, st);
#endif

  static uint32_t relayArtifactSinceMs = 0;
  static uint32_t lastRelayArtifactHealMs = 0;
  const bool suspiciousRelayOffArtifact =
      !effectiveRelayLatchedOn &&
      (vFast >= MAINS_PRESENT_ON_V) &&
      (irmsRawMeasured > CURRENT_DISPLAY_ON_A) &&
      (irmsRawMeasured <= RELAY_ARTIFACT_FORCE_ZERO_A);
  if (suspiciousRelayOffArtifact) {
    if (relayArtifactSinceMs == 0) relayArtifactSinceMs = millis();
  } else {
    relayArtifactSinceMs = 0;
  }
  if (!paused && !gSafeMode && !portalActive && relayArtifactSinceMs != 0 &&
      (millis() - relayArtifactSinceMs) >= RELAY_ARTIFACT_SELF_HEAL_MS &&
      (millis() - lastRelayArtifactHealMs) >= 5000UL) {
    lastRelayArtifactHealMs = millis();
    armRelayArtifactBlank_();
    (void)restartSensePipelineSoft_();
    relayArtifactSinceMs = 0;
  }

  network.setMlUploadSuspended(relayNetQuietActive || paused || gSafeMode || (st != STATE_NORMAL));
  if (!paused && !gSafeMode) {
    static uint32_t lastLoggedFeatUptimeMs = 0;
    static uint32_t lastSeenFeatUptimeMs = 0;
    static float logCadenceCarryMs = (float)CSV_LOG_MIN_INTERVAL_MS;
    if (freshFeatThisLoop && f.uptime_ms != 0U && f.uptime_ms != lastSeenFeatUptimeMs) {
      float featGapMs = (lastSeenFeatUptimeMs != 0U && f.uptime_ms > lastSeenFeatUptimeMs)
          ? float(f.uptime_ms - lastSeenFeatUptimeMs)
          : f.frame_dt_ms;
      if (!isfinite(featGapMs) || featGapMs < 1.0f || featGapMs > 500.0f) featGapMs = FEATURE_FRAME_PERIOD_MS;
      lastSeenFeatUptimeMs = f.uptime_ms;
      logCadenceCarryMs += featGapMs;
      if (logCadenceCarryMs > (float)CSV_LOG_MIN_INTERVAL_MS * 3.0f) {
        logCadenceCarryMs = (float)CSV_LOG_MIN_INTERVAL_MS;
      }

      const bool logCadenceReady =
          (f.uptime_ms != lastLoggedFeatUptimeMs) &&
          (lastLoggedFeatUptimeMs == 0U || logCadenceCarryMs >= (float)CSV_LOG_MIN_INTERVAL_MS);
      if (logCadenceReady) {
        lastLoggedFeatUptimeMs = f.uptime_ms;
        logCadenceCarryMs -= (float)CSV_LOG_MIN_INTERVAL_MS;
        if (logCadenceCarryMs < 0.0f) logCadenceCarryMs = 0.0f;
        const uint32_t logEnqueueMs = millis();
        FeatureFrame fLog = f;
        fLog.irms = irmsRawForLogic;
        fLog.log_enqueue_uptime_ms = logEnqueueMs;
        fLog.timing_skew_ms = (fLog.log_enqueue_uptime_ms >= fLog.frame_start_uptime_ms)
            ? float(fLog.log_enqueue_uptime_ms - fLog.frame_start_uptime_ms)
            : 0.0f;
        network.ingestLog(fLog, st, collectionModeActive ? 0 : protection.arcCounter());
      }
    }
  }

  static FaultState displayFaultState = STATE_NORMAL;
  static uint32_t displayFaultUntil = 0;
  if (faultClearRequested) { displayFaultState = STATE_NORMAL; displayFaultUntil = 0; }
  if (!faultClearSuppressActive && st != STATE_NORMAL) { displayFaultState = st; displayFaultUntil = millis() + FAULT_ALERT_MIN_MS; }
  else if (displayFaultState != STATE_NORMAL && (int32_t)(millis() - displayFaultUntil) >= 0) { displayFaultState = STATE_NORMAL; displayFaultUntil = 0; }
  const bool displayFaultActive = (displayFaultState != STATE_NORMAL) && ((int32_t)(displayFaultUntil - millis()) > 0);

  static uint32_t noPowerSinceMsOled = 0;
  const bool unpluggedLiveOled = classifyUnpluggedSocket(vRaw, vFast, irmsRawForLogic, f.current_valid != 0, st, &noPowerSinceMsOled);
  OledOverlay ov = OledOverlay::NONE;
  if (!gPauseByOta && !bootSettling) {
    if (displayFaultActive) {
      if (displayFaultState == STATE_HEATING) ov = OledOverlay::FAULT_HEAT;
      else if (displayFaultState == STATE_ARCING) ov = OledOverlay::FAULT_ARC;
      else if (displayFaultState == STATE_OVERVOLTAGE) ov = OledOverlay::FAULT_OVERVOLT;
      else if (displayFaultState == STATE_UNDERVOLTAGE) ov = OledOverlay::FAULT_UNDERVOLT;
      else if (displayFaultState == STATE_OVERLOAD || displayFaultState == STATE_SUSTAINED_OVERLOAD) ov = OledOverlay::FAULT_OVERLOAD;
    } else if (unpluggedLiveOled) ov = OledOverlay::UNPLUGGED;
  }

  notification.setOverlay(ov); notification.setState(displayFaultActive ? displayFaultState : st); notification.setMeasurements(vRms, f.irms, apparentPowerVa, tC);
  const bool showWifiWait = (!wifiConnected && (wifiPhase == WifiHandler::PHASE_CONNECTING || wifiPhase == WifiHandler::PHASE_AP_WAIT_CLIENT || wifiPhase == WifiHandler::PHASE_PORTAL_ACTIVE)) &&
                            ((wifiBannerUntilMs == 0) || ((int32_t)(wifiBannerUntilMs - millis()) > 0) || portalActive || wifiPhase == WifiHandler::PHASE_AP_WAIT_CLIENT);
  notification.setWiFi(wifiConnected, wifiMgr.rssi(), wifiMgr.isBlockingPhase(), portalActive, wifiTimedOutUi, wifiPhase == WifiHandler::PHASE_AP_WAIT_CLIENT);
  if (!gPauseByOta && !showWifiWait) notification.setOta(false, 0);
  static uint32_t lastUiRenderMs = 0;
  static OledOverlay lastUiOv = OledOverlay::NONE;
  static FaultState lastUiState = STATE_NORMAL;
  const FaultState uiState = displayFaultActive ? displayFaultState : st;
  const bool uiUrgent = (ov != lastUiOv) || (uiState != lastUiState);
  if (uiUrgent || ((millis() - lastUiRenderMs) >= OLED_RENDER_INTERVAL_MS)) {
    notification.render();
    lastUiRenderMs = millis();
    lastUiOv = ov;
    lastUiState = uiState;
  }

  static FaultState lastImmediateFaultState = STATE_NORMAL;
  if (!gSafeMode && !paused && !bootSettling) {
    if (st != STATE_NORMAL && (st != lastImmediateFaultState || tripOffEdge)) {
      FeatureFrame fHist = f; fHist.irms = irmsRawForLogic; (void)network.logFeatureEvent(String(stateToCstr(st)), fHist, apparentPowerVa, (tripOffEdge && protectionEnabled));
    }
    lastImmediateFaultState = st;
  }

  static uint32_t lastLive = 0;
  if (!paused && !relayNetQuietActive && (millis() - lastLive > 1000)) {
    lastLive = millis();
    static uint32_t noPowerSinceMs = 0;
    const bool unpluggedLive = classifyUnpluggedSocket(vRaw, vFast, irmsRawForLogic, f.current_valid != 0, st, &noPowerSinceMs);
    String stateStr;
    if (portalActive) stateStr = "CONFIG_PORTAL";
    else if (bootSettling || protectionInhibit) stateStr = "STARTUP_STABILIZING";
    else if (gSafeMode) stateStr = "SAFE_MODE";
    else if (unpluggedLive && st != STATE_ARCING && st != STATE_HEATING) stateStr = "UNPLUGGED";
    else stateStr = String(stateToCstr(st));
    network.requestLiveUpdate(vRms, f.irms, apparentPowerVa, tC,
                              f.abs_irms_zscore_vs_baseline, f.delta_irms_abs,
                              f.halfcycle_asymmetry, f.suspicious_run_energy,
                              f.delta_hf_energy, f.delta_flux, f.v_sag_pct,
                              f.midband_residual_ratio, f.zcv,
                              f.spectral_flux_midhf, f.peak_fluct_cv,
                              f.residual_crest_factor, f.thd_i,
                              f.hf_energy_delta, f.edge_spike_ratio,
                              f.model_pred,
                              f.context_family_code_runtime,
                              f.context_family_confidence,
                              f.context_family_code_provisional,
                              f.context_family_confidence_provisional,
                              f.context_acquiring != 0,
                              f.context_latched != 0,
                              stateStr,
                              protection.faultLatched(), controlsLocked, effectiveRelayLatchedOn, protection.relayPulseActive());
  }

  static uint32_t wifiDropSinceMs = 0;
  static uint32_t lastWifiRecoverKickMs = 0;
  if (!portalActive && !wifiConnected) {
    if (wifiDropSinceMs == 0) wifiDropSinceMs = millis();
    const bool wifiDropPersistent = (millis() - wifiDropSinceMs) >= 8000UL;
    if (wifiDropPersistent && (millis() - lastWifiRecoverKickMs) >= 10000UL) {
      network.stopAllClients();
      WiFi.disconnect(false, false);
      delay(20);
      WiFi.reconnect();
      lastWifiRecoverKickMs = millis();
    }
  } else {
    wifiDropSinceMs = 0;
  }

  if (!paused && wifiCloudWarm && ((int32_t)(relayNetQuietUntilMs - millis()) <= 0)) network.loop();
  delay(1);
}
