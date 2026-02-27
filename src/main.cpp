#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "SmartPlugConfig.h"
#include "SmartPlugTypes.h"
#include "NetworkManager.h"
#include "CloudHandler.h"
#include "OLED_NOTIF.h"
#include "TimeSync.h"
#include "PullOTA.h"
#include "BootGuard.h"

#include "CurrentSensor.h"
#include "VoltageSensor.h"
#include "TempSensor.h"
#include "FaultLogic.h"
#include "Actuators.h"
#include "ArcModel.h"
#include "ArcFeatures.h"

#if ENABLE_ML_LOGGER
  #include "DataLogger.h"
#endif

#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"
#define DATABASE_URL "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"
static const char* FW_VERSION = "TSP-v0.2.10";

static const char* OTA_DESIRED_VERSION_PATH = "/ota/desired_version";
static const char* OTA_FIRMWARE_URL_PATH    = "/ota/firmware_url";
static const uint32_t OTA_CHECK_INTERVAL_MS = 60000;

OLED_NOTIF      oled(0x3C);
NetworkManager  net;
CloudHandler    cloud;
TimeSync        timeSync;
PullOTA         ota;

VoltageSensor   voltSensor(PIN_VOLT_ADC);
TempSensor      tempSensor(PIN_TEMP_ADC);

FaultLogic      faultLogic;
Actuators       actuators;

#if ENABLE_ML_LOGGER
DataLogger      logger;
#endif

CurrentSensor   curSensor;
ArcFeatures     arcFeat;
CurrentCalib    curCalib;
static uint16_t s_raw[N_SAMP];
QueueHandle_t   qFeat = nullptr;

static bool gSafeMode = false;

// ---- Sound hooks (non-capturing; safer/leaner for stability) ----
static void onOtaEventSound(OtaEvent ev) {
  switch (ev) {
    case OtaEvent::START:   actuators.notify(SND_OTA_START); break;
    case OtaEvent::SUCCESS: actuators.notify(SND_OTA_OK);    break;
    case OtaEvent::FAIL:    actuators.notify(SND_OTA_FAIL);  break;
    default: break;
  }
}

// ---------------- Core0 Task ----------------
void Core0Task(void* pvParameters) {
  (void)pvParameters;

  ArcFeatOut out;

  // last-good cache (so we can keep the queue populated)
  FeatureFrame lastGood = {};
  bool hasGood = false;

  while (true) {
    FeatureFrame f;
    f.uptime_ms = millis();
    f.feat_valid = 0;

    // IMPORTANT: start with a known Fs (your config)
    float measuredFs = FS_TARGET_HZ;

    size_t got = curSensor.capture(s_raw, N_SAMP, &measuredFs);

    // If driver doesn't populate Fs (or writes 0), fall back safely.
    float fs_hz = (measuredFs > 1000.0f) ? measuredFs : FS_TARGET_HZ;

    // Quick retry if burst was short (common with WiFi/SPI contention)
    if (got != N_SAMP) {
      vTaskDelay(1);
      measuredFs = FS_TARGET_HZ;
      got = curSensor.capture(s_raw, N_SAMP, &measuredFs);
      fs_hz = (measuredFs > 1000.0f) ? measuredFs : FS_TARGET_HZ;
    }

    bool ok = false;
    if (got == N_SAMP) {
      ok = arcFeat.compute(s_raw, N_SAMP, fs_hz, curCalib, MAINS_F0_HZ, out);
    }

    if (ok) {
      f.feat_valid = 1;
      f.irms     = out.irms_a;
      f.thd_pct  = out.thd_pct;
      f.entropy  = out.entropy;
      f.zcv_ms   = out.zcv_ms;
      f.hf_ratio = out.hf_ratio;
      f.hf_var   = out.hf_var;
      f.sf       = out.sf;
      f.cyc_var  = out.cyc_var;

      lastGood = f;
      hasGood = true;

      if (qFeat != nullptr) xQueueOverwrite(qFeat, &f);
    } else {
      // DO NOT publish zeros.
      // Just keep last good value in the queue so CSV stays clean.
      if (hasGood && qFeat != nullptr) xQueueOverwrite(qFeat, &lastGood);
    }

    vTaskDelay(1);
  }
}

static bool bootHoldForSafeModeMs(uint32_t holdMs = 1500) {
  pinMode(PIN_RESET_BTN, INPUT_PULLUP);
  if (digitalRead(PIN_RESET_BTN) != LOW) return false;

  const uint32_t t0 = millis();
  while (millis() - t0 < holdMs) {
    if (digitalRead(PIN_RESET_BTN) != LOW) return false;
    delay(10);
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  BootGuard::begin(/*stableWindowMs=*/45000, /*maxCrashBoots=*/3);
  gSafeMode = BootGuard::safeMode();
  if (bootHoldForSafeModeMs(1500)) gSafeMode = true;

  oled.begin();
  if (gSafeMode) oled.showStatus("SAFE MODE", "OTA only");
  else if (BootGuard::pendingVerify()) oled.showStatus("OTA", "Verifying...");
  else oled.showStatus("System", "Starting...");

  actuators.begin(PIN_RELAY, PIN_BUZZER_PWM, PIN_RESET_BTN, &oled);

  if (!gSafeMode) {
    voltSensor.begin();
    voltSensor.setWindowMs(200);
    voltSensor.setClampHysteresis(25.0f, 35.0f);
    tempSensor.begin();

    bool hwReady = curSensor.begin();
    if (!hwReady) {
      oled.showStatus("WARN", "No ADS8684");
      delay(800);
    } else {
      qFeat = xQueueCreate(1, sizeof(FeatureFrame));
      xTaskCreatePinnedToCore(Core0Task, "Core0Sense", 12288, nullptr, 3, nullptr, 0);
      oled.showStatus("Sensors", "Ready");
    }
  }

  oled.showStatus("WiFi", "Connecting...");
  net.begin([](WiFiManager* wm) {
    (void)wm;
    oled.showStatus("Setup Mode", "Connect to AP");
    actuators.notify(SND_WIFI_PORTAL);
  });

  oled.showStatus("Cloud", "Connecting...");
  cloud.begin(API_KEY, DATABASE_URL);
  cloud.setFirmwareVersion(FW_VERSION);
  cloud.setNormalIntervalMs(6000);
  timeSync.begin();

#if ENABLE_ML_LOGGER
  if (!gSafeMode) {
    logger.begin(&cloud);
    logger.setEnabled(false);
    logger.setDurationSeconds(10);
  }
#endif

  ota.begin(FW_VERSION, &cloud);
  ota.setEventCallback(onOtaEventSound);
  ota.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  ota.setCheckInterval(OTA_CHECK_INTERVAL_MS);

  if (gSafeMode) oled.showStatus("SAFE MODE", "Waiting OTA");
  else oled.showStatus("System", "Ready");
}

#if ENABLE_ML_LOGGER
static void pollMlControl(CloudHandler& cloud, DataLogger& logger) {
  static uint32_t lastPoll = 0;
  static bool lastEnabled = false;
  static String lastSession = "";

  if (millis() - lastPoll < ML_CTRL_POLL_MS) return;
  lastPoll = millis();

  bool enabled = false;
  int dur = 10;
  int labelOv = -1;
  String sid = "";
  String load = "unknown";

  cloud.getBool("/ml_log/enabled", enabled);
  cloud.getInt("/ml_log/duration_s", dur);
  cloud.getInt("/ml_log/label_override", labelOv);
  cloud.getString("/ml_log/session_id", sid);
  cloud.getString("/ml_log/load_type", load);

  if (dur < 5) dur = 5;
  if (dur > 60) dur = 60;

  if (enabled && sid.length() < 3) {
    logger.setEnabled(false);
    return;
  }

  logger.setDurationSeconds((uint16_t)dur);
  logger.setSession(sid, load, labelOv);

  if (enabled != lastEnabled || sid != lastSession) {
    // Sound cue only when logging actually starts (OFF -> ON).
    if (enabled && !lastEnabled) actuators.notify(SND_LOGGER_ON);

    logger.setEnabled(enabled);
    lastEnabled = enabled;
    lastSession = sid;
  }
}
#endif

void loop() {
  BootGuard::loop();

  net.update();

  // WiFi connected 
  static wl_status_t lastWifi = WL_DISCONNECTED;
  const wl_status_t wifiNow = WiFi.status();
  if (wifiNow == WL_CONNECTED && lastWifi != WL_CONNECTED) {
    actuators.notify(SND_WIFI_OK);
  }
  lastWifi = wifiNow;

  ota.loop();
  timeSync.update();

  if (gSafeMode) {
    static uint32_t lastCloudUpdate = 0;
    if (millis() - lastCloudUpdate > 5000) {
      lastCloudUpdate = millis();
      cloud.update(0,0,0, 0,0,0, 0,0, 0,0, 0, String("SAFE_MODE"), &timeSync);
    }
    delay(10);
    return;
  }

#if ENABLE_ML_LOGGER
  pollMlControl(cloud, logger);
#endif

  if (actuators.resetLongPressed()) {
    faultLogic.resetLatch();
    oled.showStatus("RESET", "Latch cleared");
    delay(250);
  }

static FeatureFrame lastGood = {};
static bool hasGood = false;
static uint32_t lastGoodMs = 0;

FeatureFrame f;
bool got = (qFeat != nullptr) && (xQueueReceive(qFeat, &f, 0) == pdTRUE);

if (got) {
  // With Core0Task patch, queue is always last-good.
  lastGood = f;
  hasGood = true;
  lastGoodMs = millis();
}

if (hasGood) {
  f = lastGood;
} else {
  memset(&f, 0, sizeof(f));
  f.uptime_ms = millis();
}

// Only zero out if truly stale (e.g., Core task died)
const bool trulyStale = hasGood && (millis() - lastGoodMs > 1200);
if (trulyStale) {
  f.irms = 0;
  f.entropy = 0;
  f.thd_pct = 0;
  f.zcv_ms = 0;
  f.hf_ratio = 0;
  f.hf_var = 0;
  f.sf = 0;
  f.cyc_var = 0;
}

  const float iHint = f.irms;

  // -------- Slow sensors --------
  static float vRms = 0.0f;
  static float tC = 0.0f;
  static uint32_t tT = 0;

  constexpr float V_EMA_ALPHA = 1.00f;
  constexpr float T_EMA_ALPHA = 0.10f;

  float newV = voltSensor.update();
  const bool vrmsLooksInvalid = (newV <= 1.0f && iHint > 0.05f);

  if (newV >= 0.0f && !vrmsLooksInvalid) {
    if (vRms <= 0.0f) vRms = newV;
    else vRms = (1.0f - V_EMA_ALPHA) * vRms + V_EMA_ALPHA * newV;
  }

  if (millis() - tT > 500) {
    tT = millis();
    float newT = tempSensor.readTempC();
    if (newT > -50.0f && newT < 120.0f) {
      if (tC <= -50.0f || tC >= 120.0f) tC = newT;
      else tC = (1.0f - T_EMA_ALPHA) * tC + T_EMA_ALPHA * newT;
    }
  }

  f.vrms   = vRms;
  f.temp_c = tC;

  // -------- Transient suppress --------
  static float prevIrms = 0.0f;
  const float dI = fabsf(f.irms - prevIrms);
  prevIrms = f.irms;

  static uint32_t transientUntil = 0;
  if (dI > 0.6f) transientUntil = millis() + 600;
  const bool isTransient = (millis() < transientUntil);

  int pred = ArcPredict(f.entropy, f.thd_pct, f.zcv_ms,
                        f.hf_ratio, f.hf_var,
                        f.sf, f.cyc_var,
                        f.vrms, f.irms, f.temp_c);
  if (isTransient) pred = 0;
  f.model_pred = (uint8_t)pred;

  FaultState state = faultLogic.update(tC, f.irms, f.model_pred);
  FaultState stateOut = state;

  actuators.apply(stateOut, vRms, f.irms, tC);

#if ENABLE_ML_LOGGER
  static uint32_t lastMlLogMs = 0;
  const uint32_t logPeriodMs = 1000UL / ML_LOG_RATE_HZ;

  if (millis() - lastMlLogMs >= logPeriodMs) {
    lastMlLogMs = millis();
    f.epoch_ms  = timeSync.isSynced() ? timeSync.nowEpochMs() : 0;
    logger.ingest(f, stateOut, faultLogic.arcCounter());
  }
  logger.loop();
#endif

  static uint32_t lastCloudUpdate = 0;
  if (millis() - lastCloudUpdate > 5000) {
    lastCloudUpdate = millis();
    cloud.update(vRms, f.irms, tC,
                 f.zcv_ms, f.thd_pct, f.entropy,
                 f.hf_ratio, f.hf_var,
                 f.sf, f.cyc_var,
                 f.model_pred,
                 String(stateToCstr(stateOut)), &timeSync);
  }
}