#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <driver/gpio.h>

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
static const char* FW_VERSION = "TSP-v0.2.11";

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
static QueueHandle_t qFeat = nullptr;

static bool gSafeMode = false;
static volatile bool gPauseByPortal = false;
static volatile bool gPauseByOta = false;

static inline void reassertCsRxPin() {
  gpio_hold_dis((gpio_num_t)PIN_ADC_CS);
  pinMode(PIN_ADC_CS, OUTPUT);
  digitalWrite(PIN_ADC_CS, HIGH);
  gpio_set_pull_mode((gpio_num_t)PIN_ADC_CS, GPIO_PULLUP_ONLY);
  gpio_set_drive_capability((gpio_num_t)PIN_ADC_CS, GPIO_DRIVE_CAP_3);
  gpio_hold_en((gpio_num_t)PIN_ADC_CS);
}

static void onOtaEvent(OtaEvent ev) {
  if (ev == OtaEvent::START) {
    gPauseByOta = true;
    actuators.notify(SND_OTA_START);
  } else if (ev == OtaEvent::SUCCESS) {
    actuators.notify(SND_OTA_OK);
  } else if (ev == OtaEvent::FAIL) {
    gPauseByOta = false;
    actuators.notify(SND_OTA_FAIL);
  }
}

// Core0: capture + features (hold last good, never spam zeros)
static void Core0Task(void* pv) {
  (void)pv;

  ArcFeatOut out;
  FeatureFrame lastGood = {};
  bool hasGood = false;

  while (true) {
    const bool paused = gPauseByPortal || gPauseByOta;
    if (paused) {
      if (hasGood && qFeat) xQueueOverwrite(qFeat, &lastGood);
      vTaskDelay(20);
      continue;
    }

    reassertCsRxPin();

    FeatureFrame f;
    f.uptime_ms = millis();
    f.feat_valid = 0;

    float fs_hz = FS_TARGET_HZ;
    size_t got = curSensor.capture(s_raw, N_SAMP, &fs_hz);
    if (fs_hz < 1000.0f) fs_hz = FS_TARGET_HZ;

    // quick retry if short burst
    if (got != N_SAMP) {
      vTaskDelay(1);
      reassertCsRxPin();
      fs_hz = FS_TARGET_HZ;
      got = curSensor.capture(s_raw, N_SAMP, &fs_hz);
      if (fs_hz < 1000.0f) fs_hz = FS_TARGET_HZ;
    }

    bool ok = false;
    if (got == N_SAMP && fs_hz > 20000.0f) {
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
      if (qFeat) xQueueOverwrite(qFeat, &f);
    } else {
      if (hasGood && qFeat) xQueueOverwrite(qFeat, &lastGood);
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
  esp_log_level_set("*", ESP_LOG_NONE);

  BootGuard::begin(45000, 3);
  gSafeMode = BootGuard::safeMode();
  if (bootHoldForSafeModeMs(1500)) gSafeMode = true;

  oled.begin();
  actuators.begin(PIN_RELAY, PIN_BUZZER_PWM, PIN_RESET_BTN, &oled);

  reassertCsRxPin();

  if (!gSafeMode) {
    voltSensor.begin();
    voltSensor.setWindowMs(200);
    voltSensor.setClampHysteresis(25.0f, 35.0f);
    tempSensor.begin();

    if (curSensor.begin()) {
      qFeat = xQueueCreate(1, sizeof(FeatureFrame));
      xTaskCreatePinnedToCore(Core0Task, "Core0Sense", 12288, nullptr, 3, nullptr, 0);
      oled.showStatus("Sensors", "Ready");
    } else {
      oled.showStatus("WARN", "No ADS8684");
      delay(600);
    }
  } else {
    oled.showStatus("SAFE MODE", "OTA only");
  }

  net.begin([](WiFiManager* wm){
    (void)wm;
    oled.showStatus("Setup Mode", "Connect to AP");
    actuators.notify(SND_WIFI_PORTAL);
  });

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
  ota.setEventCallback(onOtaEvent);
  ota.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  ota.setCheckInterval(OTA_CHECK_INTERVAL_MS);

  oled.showStatus("System", "Ready");
}

#if ENABLE_ML_LOGGER
static void pollMlControl(CloudHandler& cloud, DataLogger& logger) {
  static uint32_t lastPoll = 0;
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

  if (enabled && sid.length() < 3) { logger.setEnabled(false); return; }

  logger.setDurationSeconds((uint16_t)dur);
  logger.setSession(sid, load, labelOv);
  logger.setEnabled(enabled);
}
#endif

void loop() {
  BootGuard::loop();

  net.update();
  ota.loop();
  timeSync.update();

  gPauseByPortal = net.inConfigPortal();
  const bool paused = gPauseByPortal || gPauseByOta;

  if (gSafeMode) {
    static uint32_t lastCloud = 0;
    if (millis() - lastCloud > 5000) {
      lastCloud = millis();
      cloud.update(0,0,0, 0,0,0, 0,0, 0,0, 0, String("SAFE_MODE"), &timeSync);
    }
    delay(10);
    return;
  }

#if ENABLE_ML_LOGGER
  if (!paused) pollMlControl(cloud, logger);
  else logger.setEnabled(false);
#endif

  if (actuators.resetLongPressed()) {
    faultLogic.resetLatch();
    oled.showStatus("RESET", "Latch cleared");
    delay(250);
  }

  // Read latest features (Core0Task always holds last-good)
  static FeatureFrame lastF = {};
  static bool hasLast = false;

  FeatureFrame f;
  if (qFeat && xQueueReceive(qFeat, &f, 0) == pdTRUE) {
    lastF = f;
    hasLast = true;
  } else if (hasLast) {
    f = lastF;
  } else {
    memset(&f, 0, sizeof(f));
    f.uptime_ms = millis();
  }

  // Slow sensors
  static float vRms = 0.0f;
  static float tC = 0.0f;
  static uint32_t tT = 0;

  const float iHint = f.irms;

  float newV = voltSensor.update();
  const bool vrmsLooksInvalid = (newV <= 1.0f && iHint > 0.05f);
  if (newV >= 0.0f && !vrmsLooksInvalid) vRms = newV;

  if (millis() - tT > 500) {
    tT = millis();
    float newT = tempSensor.readTempC();
    if (newT > -50.0f && newT < 120.0f) tC = newT;
  }

  f.vrms = vRms;
  f.temp_c = tC;

  int pred = 0;
  if (!paused) {
    pred = ArcPredict(f.entropy, f.thd_pct, f.zcv_ms,
                      f.hf_ratio, f.hf_var,
                      f.sf, f.cyc_var,
                      f.vrms, f.irms, f.temp_c);
  }
  f.model_pred = (uint8_t)pred;

  FaultState st = faultLogic.update(tC, f.irms, f.model_pred);

  // Report real state always; relay control handled in Actuators.cpp (DATA_COLLECTION_MODE safe)
  actuators.apply(st, vRms, f.irms, tC);

#if ENABLE_ML_LOGGER
  if (!paused) {
    static uint32_t lastMl = 0;
    const uint32_t period = 1000UL / ML_LOG_RATE_HZ;
    if (millis() - lastMl >= period) {
      lastMl = millis();
      f.epoch_ms = timeSync.isSynced() ? timeSync.nowEpochMs() : 0;
      logger.ingest(f, st, faultLogic.arcCounter());
    }
    logger.loop();
  }
#endif

  static uint32_t lastLive = 0;
  if (millis() - lastLive > 5000) {
    lastLive = millis();
    String stateStr = paused ? (gPauseByPortal ? "CONFIG_PORTAL" : "OTA_UPDATING")
                             : String(stateToCstr(st));
    cloud.update(vRms, f.irms, tC,
                 f.zcv_ms, f.thd_pct, f.entropy,
                 f.hf_ratio, f.hf_var,
                 f.sf, f.cyc_var,
                 f.model_pred,
                 stateStr, &timeSync);
  }
}