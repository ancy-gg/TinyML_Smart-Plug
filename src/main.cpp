#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <math.h>

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

static uint16_t s_raw[N_SAMP];
static QueueHandle_t qFeat = nullptr;
static TaskHandle_t gCore0SenseTask = nullptr;
static bool gSafeMode = false;
static volatile bool gPauseByOta = false;

static bool arcInputStable(bool currentValid, bool featValid, float irms) {
  static uint32_t stableSince = 0;
  static float refIrms = 0.0f;
  const uint32_t now = millis();
  if (!currentValid || !featValid || irms < ARC_MIN_IRMS_A) {
    stableSince = 0; refIrms = irms; return false;
  }
  if (stableSince == 0) { stableSince = now; refIrms = irms; return false; }
  const float tol = fmaxf(0.08f, 0.25f * fmaxf(refIrms, 0.10f));
  if (fabsf(irms - refIrms) > tol) { stableSince = now; refIrms = irms; return false; }
  refIrms += 0.08f * (irms - refIrms);
  return (now - stableSince) >= 1200UL;
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
  if (!sinceMs) return false;
  const bool rawGone  = (vRaw <= MAINS_PRESENT_OFF_V);
  const bool protGone = (vProtect <= MAINS_PRESENT_OFF_V);
  const bool zeroLike = rawGone || protGone;
  const bool noLoad = (!currentValid) || (irmsA <= LOAD_OFF_DETECT_A);
  const bool faultKeep = (st == STATE_ARCING) || (st == STATE_HEATING);
  if (zeroLike && noLoad && !faultKeep) {
    if (*sinceMs == 0) *sinceMs = millis();
  } else if (vRaw >= MAINS_PRESENT_ON_V || vProtect >= MAINS_PRESENT_ON_V || !noLoad || faultKeep) {
    *sinceMs = 0;
  }
  return (*sinceMs != 0) && ((millis() - *sinceMs) >= UNPLUGGED_STATE_DELAY_MS);
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

static void onOtaEvent(OtaEvent ev, int progress) {
  if (ev == OtaEvent::START) {
    gPauseByOta = true; notification.setOta(true, 0); notification.notify(SND_OTA_START);
  } else if (ev == OtaEvent::PROGRESS) {
    notification.setOta(true, (uint8_t)constrain(progress, 0, 100));
  } else if (ev == OtaEvent::SUCCESS) {
    notification.setOta(true, 100); notification.notify(SND_OTA_OK); network.logStatusEvent("FIRMWARE UPDATED", 0.0f, 0.0f, 0.0f, 0.0f);
  } else if (ev == OtaEvent::FAIL) {
    gPauseByOta = false; notification.setOta(false, 0); notification.notify(SND_OTA_FAIL);
  }
}

static void Core0Task(void* pv) {
  (void)pv;
  ArcDetectionResult out;
  FeatureFrame lastGood = {};
  bool hasGood = false;

  while (millis() < SENSOR_BOOT_SETTLE_MS) vTaskDelay(20 / portTICK_PERIOD_MS);

  while (true) {
    if (gPauseByOta || gSafeMode) {
      vTaskDelay(20 / portTICK_PERIOD_MS);
      continue;
    }

    FeatureFrame f = {};
    f.uptime_ms = millis();
    float fs_hz = FS_TARGET_HZ;
    size_t got = curSensor.capture(s_raw, N_SAMP, &fs_hz);

    if (gPauseByOta || gSafeMode) {
      vTaskDelay(20 / portTICK_PERIOD_MS);
      continue;
    }

    if (got != N_SAMP || fs_hz < CURRENT_FRAME_MIN_FS_HZ) {
      vTaskDelay(1);
      got = curSensor.capture(s_raw, N_SAMP, &fs_hz);
    }

    if (gPauseByOta || gSafeMode) {
      vTaskDelay(20 / portTICK_PERIOD_MS);
      continue;
    }

    bool ok = false;
    if (got == N_SAMP && fs_hz >= CURRENT_FRAME_MIN_FS_HZ) ok = arcDetect.compute(s_raw, N_SAMP, fs_hz, curCalib, MAINS_F0_HZ, out);
    if (ok && out.current_valid) {
      f.adc_fs_hz = out.fs_hz; f.irms = out.irms_a; f.current_valid = 1; f.feat_valid = out.feat_valid ? 1 : 0;
      if (out.feat_valid) {
        f.cycle_nmse = out.cycle_nmse; f.zcv = out.zcv; f.zc_dwell_ratio = out.zc_dwell_ratio;
        f.pulse_count_per_cycle = out.pulse_count_per_cycle; f.peak_fluct_cv = out.peak_fluct_cv;
        f.midband_residual_rms = out.midband_residual_rms; f.hf_band_energy_ratio = out.hf_band_energy_ratio;
        f.wpe_entropy = out.wpe_entropy; f.spec_entropy = out.spec_entropy; f.thd_i = out.thd_i;
      }
      lastGood = f; hasGood = true; if (qFeat) xQueueOverwrite(qFeat, &f);
    } else {
      FeatureFrame invalid = {};
      invalid.uptime_ms = millis();
      hasGood = false;
      if (qFeat) xQueueOverwrite(qFeat, &invalid);
    }
    vTaskDelay(1);
  }
}


static void pollMlControl(FirebaseNetwork& network, Notification& notifyUi) {
  static uint32_t lastPoll = 0;
  static bool lastEnabled = false;
  if (millis() - lastPoll < ML_CONTROL_POLL_MS) return;
  lastPoll = millis();

  bool enabled = false;
  int dur = ML_LOG_DURATION_S;
  int labelOv = ML_UNKNOWN_LABEL;
  String sid = "";
  String load = "unknown";
  (void)network.fetchMlControl(enabled, dur, labelOv, sid, load);

  if (dur < ML_LOG_MIN_DURATION_S) dur = ML_LOG_MIN_DURATION_S;
  if (dur > ML_LOG_MAX_DURATION_S) dur = ML_LOG_MAX_DURATION_S;
  if (enabled && sid.length() < 3) {
    network.setLogEnabled(false);
    lastEnabled = false;
    return;
  }

  network.setLogDurationSeconds((uint16_t)dur);
  network.setLogSession(sid, load, labelOv);
  network.setLogEnabled(enabled);
  if (enabled && !lastEnabled) { notification.notify(SND_LOGGER_ON); notifyUi.triggerCollecting(1000); }
  lastEnabled = enabled;
}

static void maybeStartAutoArcCapture(FirebaseNetwork& network, Notification& notifyUi, bool paused, bool safeMode, bool modelPositive) {
  static bool lastPositive = false;
  static uint32_t lastAutoStartMs = 0;
  const bool rising = (modelPositive && !lastPositive);
  lastPositive = modelPositive;
  if (!paused && !safeMode && !network.manualEnabled() && rising) {
    const uint32_t now = millis();
    if ((now - lastAutoStartMs) >= AUTO_ARC_CAPTURE_COOLDOWN_MS) {
      if (network.startAutoCapture("model_arc", AUTO_ARC_CAPTURE_DURATION_S)) {
        lastAutoStartMs = now; notification.notify(SND_LOGGER_ON); notifyUi.triggerCollecting(1000);
      }
    }
  }
}

void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);
  notification.begin(PIN_BUZZER_PWM);
  notification.startBootSequence(3600);
  protection.begin(PIN_LATCH_ON, PIN_LATCH_OFF);
  notification.notify(SND_BOOT);

  voltSensor.begin();
  voltSensor.setWindowMs(500);
  voltSensor.setClampHysteresis(12.0f, 24.0f);
  voltSensor.setLongAverage(2.6f, 18.0f);
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
  updater.setCheckInterval(10000UL);
  gSafeMode = updater.safeMode();

  if (!gSafeMode) {
    if (curSensor.begin()) {
      qFeat = xQueueCreate(1, sizeof(FeatureFrame));
      xTaskCreatePinnedToCore(Core0Task, "Core0Sense", 16384, nullptr, 3, &gCore0SenseTask, 0);
      notification.showStatus("Sensors", "MCP3204 ready");
    } else {
      notification.showStatus("WARN", "No MCP3204"); delay(700);
    }
  } else {
    notification.showStatus("SAFE MODE", "OTA only");
  }

  wifiMgr.begin([](WiFiManager* wm) { (void)wm; });
  network.setLogEnabled(false);
  network.setLogDurationSeconds(ML_LOG_DURATION_S);
}

void loop() {
  network.updateClock();
  wifiMgr.update();
  updater.loop();

  static String s_lastOtaErr = "";
  const String otaErr = updater.lastError();
  if (otaErr.length() && otaErr != s_lastOtaErr) {
    s_lastOtaErr = otaErr;
    network.logStatusEvent(otaErr, 0.0f, 0.0f, 0.0f, 0.0f);
  }

  const bool portalActive = wifiMgr.inConfigPortal();
  const bool paused = gPauseByOta;
  const bool bootSettling = (millis() < SENSOR_BOOT_SETTLE_MS);
  const bool protectionInhibit = (millis() < (SENSOR_BOOT_SETTLE_MS + PROTECTION_INHIBIT_MS));

  static WifiHandler::Phase lastWiFiPhase = WifiHandler::PHASE_BOOT_CONNECT;
  static bool lastWiFiConnected = false;
  static uint32_t wifiBannerUntilMs = 0;
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
  if (wifiConnected && !lastWiFiConnected) notification.notify(SND_WIFI_OK);
  lastWiFiConnected = wifiConnected;

  static FeatureFrame lastF = {};
  static bool hasLast = false;
  static uint32_t lastFeatRxMs = 0;
  FeatureFrame f;
  if (qFeat && xQueueReceive(qFeat, &f, 0) == pdTRUE) {
    lastF = f; hasLast = true; lastFeatRxMs = millis();
  } else if (hasLast) {
    f = lastF;
    if ((millis() - lastFeatRxMs) > FEAT_STALE_MS) {
      f.irms = 0.0f; f.current_valid = 0; f.feat_valid = 0; f.model_pred = 0; f.adc_fs_hz = 0.0f;
      f.cycle_nmse = f.zcv = f.zc_dwell_ratio = f.pulse_count_per_cycle = f.peak_fluct_cv = 0.0f;
      f.midband_residual_rms = f.hf_band_energy_ratio = f.wpe_entropy = f.spec_entropy = f.thd_i = 0.0f;
      lastF = f;
    }
  } else { memset(&f, 0, sizeof(f)); f.uptime_ms = millis(); }

  static float vRms = 0.0f, vFast = 0.0f, vRaw = 0.0f, tC = 0.0f;
  static uint32_t tTemp = 0;
  float newV = voltSensor.update();
  if (newV >= 0.0f) { vRms = newV; vFast = voltSensor.protectVrms(); vRaw = voltSensor.rawVrms(); }
  if (millis() - tTemp >= 500) { tTemp = millis(); float newT = tempSensor.readTempC(); if (newT > -50.0f && newT < 150.0f) tC = newT; }

  static bool lastMainsPresentForFeat = false;
  const bool mainsPresentForFeat = (vFast >= MAINS_PRESENT_ON_V);
  if (mainsPresentForFeat && !lastMainsPresentForFeat) {
    hasLast = false;
    lastFeatRxMs = 0;
    memset(&lastF, 0, sizeof(lastF));
  }
  lastMainsPresentForFeat = mainsPresentForFeat;

  f.vrms = vRms; f.temp_c = tC;
  const float irmsRawMeasured = f.irms;
  float irmsRawForLogic = cleanLogicCurrent(irmsRawMeasured, f.current_valid != 0, vRaw, vFast);
  if (notification.shouldSuppressCurrentArtifacts() && irmsRawForLogic < BUZZER_ARTIFACT_MAX_A) {
    irmsRawForLogic = 0.0f;
    f.current_valid = 0;
    f.feat_valid = 0;
  }
  f.irms = cleanDisplayCurrent(irmsRawForLogic, f.current_valid != 0, f.feat_valid != 0, vRaw, vFast);
  if (irmsRawForLogic < FEATURE_MIN_IRMS_A) {
    f.feat_valid = 0; f.cycle_nmse = f.zcv = f.zc_dwell_ratio = f.pulse_count_per_cycle = f.peak_fluct_cv = 0.0f;
    f.midband_residual_rms = f.hf_band_energy_ratio = f.wpe_entropy = f.spec_entropy = f.thd_i = 0.0f;
  }

  const float apparentPowerVa = (vRms > 0.10f && f.irms > 0.001f) ? (vRms * f.irms) : 0.0f;
  const bool voltageNormal = (vFast >= VOLT_NORMAL_MIN_V && vFast <= VOLT_NORMAL_MAX_V);
  const bool arcEligible = (!gSafeMode && !paused && !bootSettling && !protectionInhibit && voltageNormal &&
                            arcInputStable(f.current_valid, f.feat_valid, irmsRawForLogic));

  network.pollControls(!gSafeMode && !paused && !portalActive && wifiConnected, portalActive);

  const bool faultClearRequested = (!gSafeMode) ? network.consumeFaultClearRequest() : false;
  const bool revertFirmwareRequested = network.consumeRevertFirmwareRequest();
  const bool otaCheckRequested = network.consumeOtaCheckRequest();
  if (otaCheckRequested) updater.requestCheckNow();
  if (faultClearRequested) { protection.resetLatch(); notification.notify(SND_RESET_ACK); notification.clearFaultAlert(); }
  if (revertFirmwareRequested) {
    network.logStatusEvent("FIRMWARE REVERT REQUESTED", 0.0f, 0.0f, 0.0f, 0.0f);
    if (!updater.rollbackToPrevious()) {
      network.logStatusEvent("FIRMWARE REVERT FAILED", 0.0f, 0.0f, 0.0f, 0.0f);
    }
  }

  int pred = 0;
  if (arcEligible) {
    pred = arcDetect.predict(f.cycle_nmse, f.zcv, f.zc_dwell_ratio, f.pulse_count_per_cycle, f.peak_fluct_cv,
                             f.midband_residual_rms, f.hf_band_energy_ratio, f.wpe_entropy, f.spec_entropy, f.thd_i,
                             f.vrms, irmsRawForLogic, f.temp_c);
  }
  f.model_pred = (uint8_t)pred;

  FaultState st = STATE_NORMAL;
  if (!bootSettling) st = protection.update(vFast, vRaw, tC, irmsRawForLogic, f.model_pred, arcEligible);
  if (gSafeMode) st = STATE_NORMAL;

  static uint32_t noPowerSinceMsCtl = 0;
  static bool unpluggedOffIssued = false;
  const bool unpluggedLiveCtl = classifyUnpluggedSocket(vRaw, vFast, irmsRawForLogic, f.current_valid != 0, st, &noPowerSinceMsCtl);
  if (unpluggedLiveCtl) {
    if (!unpluggedOffIssued) {
      protection.pulseRelayOff();
      unpluggedOffIssued = true;
    }
  } else {
    unpluggedOffIssued = false;
  }
  const bool controlsLocked = gSafeMode || paused || bootSettling || protectionInhibit || unpluggedLiveCtl || protection.webControlLocked() || protection.voltageLockoutActive();
  const bool portalRequested = (!gSafeMode) ? network.consumePortalRequest() : false;
  const bool relayOnRequested = (!gSafeMode) ? network.consumeRelayOnRequest() : false;
  const bool relayOffRequested = (!gSafeMode) ? network.consumeRelayOffRequest() : false;
  if (!gSafeMode) {
    if (portalRequested && !controlsLocked) wifiMgr.requestPortal(true);
    if (!controlsLocked) {
      if (relayOffRequested) {
        protection.pulseRelayOff();
        notification.notify(SND_RESET_ACK);
      } else if (relayOnRequested) {
        protection.pulseRelayOn();
        notification.notify(SND_RESET_ACK);
      }
    }
  }

  const bool tripOffEdge = protection.consumeTripOffEdge();
  const bool autoOnEdge  = protection.consumeAutoOnEdge();
  if (tripOffEdge) protection.pulseRelayOff();
  if (autoOnEdge && !controlsLocked) protection.pulseRelayOn();

  protection.apply(st, vRms, vFast, irmsRawForLogic, tC);
  notification.updateBuzzer(st, vFast, irmsRawForLogic, tC);
  const bool mainsPresentStable = debouncedMainsPresentForState(vFast);
  handleCueEvents(vRaw, vFast, irmsRawForLogic, mainsPresentStable, paused || gSafeMode || protectionInhibit, st);

  maybeStartAutoArcCapture(network, notification, paused || protectionInhibit, gSafeMode, (arcEligible && f.model_pred == 1));
  if (!paused && !gSafeMode) pollMlControl(network, notification); else network.setLogEnabled(false);
  if (!paused && !gSafeMode) {
    static uint32_t lastMl = 0;
    const uint32_t period = 1000UL / ML_LOG_RATE_HZ;
    if (millis() - lastMl >= period) {
      lastMl = millis(); f.epoch_ms = network.isSynced() ? network.nowEpochMs() : 0;
      FeatureFrame fLog = f; fLog.irms = irmsRawForLogic; network.ingestLog(fLog, st, protection.arcCounter());
    }
  }

  static FaultState displayFaultState = STATE_NORMAL;
  static uint32_t displayFaultUntil = 0;
  if (faultClearRequested) { displayFaultState = STATE_NORMAL; displayFaultUntil = 0; }
  if (st != STATE_NORMAL) { displayFaultState = st; displayFaultUntil = millis() + FAULT_ALERT_MIN_MS; }
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
  if (gPauseByOta) notification.setOta(true, 0); else if (!showWifiWait) notification.setOta(false, 0);
  notification.render();

  static FaultState lastImmediateFaultState = STATE_NORMAL;
  if (!gSafeMode && !paused && !bootSettling) {
    if (st != STATE_NORMAL && (st != lastImmediateFaultState || tripOffEdge)) {
      FeatureFrame fHist = f; fHist.irms = irmsRawForLogic; (void)network.logFeatureEvent(String(stateToCstr(st)), fHist, apparentPowerVa, tripOffEdge);
    }
    lastImmediateFaultState = st;
  }

  static uint32_t lastLive = 0;
  if (millis() - lastLive > 1000) {
    lastLive = millis();
    static uint32_t noPowerSinceMs = 0;
    const bool unpluggedLive = classifyUnpluggedSocket(vRaw, vFast, irmsRawForLogic, f.current_valid != 0, st, &noPowerSinceMs);
    String stateStr;
    if (gPauseByOta) stateStr = "OTA_UPDATING";
    else if (portalActive) stateStr = "CONFIG_PORTAL";
    else if (bootSettling || protectionInhibit) stateStr = "STARTUP_STABILIZING";
    else if (gSafeMode) stateStr = "SAFE_MODE";
    else if (unpluggedLive && st != STATE_ARCING && st != STATE_HEATING) stateStr = "UNPLUGGED";
    else stateStr = String(stateToCstr(st));
    network.requestLiveUpdate(vRms, f.irms, apparentPowerVa, tC,
                              f.cycle_nmse, f.zcv, f.zc_dwell_ratio,
                              f.pulse_count_per_cycle, f.peak_fluct_cv,
                              f.midband_residual_rms, f.hf_band_energy_ratio,
                              f.wpe_entropy, f.spec_entropy, f.thd_i,
                              f.model_pred, stateStr);
  }

  network.loop();
  delay(2);
}
