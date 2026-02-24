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

#include "VoltageSensor.h"
#include "TempSensor.h"
#include "FaultLogic.h"
#include "Actuators.h"
#include "ArcModel.h"

#if ENABLE_CURRENT_SENSOR
  #include "CurrentSensor.h"
  #include "ArcFeatures.h"
#endif

#if ENABLE_ML_LOGGER
  #include "DataLogger.h"
#endif

// ------------------- Firebase Configuration -------------------
#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"
#define DATABASE_URL "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"
static const char* FW_VERSION = "TSP-v0.1.5";

// OTA Paths (RTDB)
static const char* OTA_DESIRED_VERSION_PATH = "/ota/desired_version";
static const char* OTA_FIRMWARE_URL_PATH    = "/ota/firmware_url";
static const uint32_t OTA_CHECK_INTERVAL_MS = 60 * 1000;

// ------------------- Globals -------------------
NetworkManager netManager;
CloudHandler cloudHandler;
OLED_NOTIF oled(0x3C);
TimeSync timeSync;
PullOTA pullOta;

VoltageSensor voltSensor(PIN_VOLT_ADC);
TempSensor tempSensor(PIN_TEMP_ADC);

FaultLogic faultLogic;
Actuators actuators;

static bool oledOk = false;

#if ENABLE_CURRENT_SENSOR
// Core0 feature pipeline objects
CurrentSensor currentSensor;
ArcFeatures arcFeatures;

struct Core0Frame {
  uint32_t uptime_ms;
  uint64_t epoch_ms;
  float fs_hz;
  float irms_a;
  float thd_pct;
  float entropy;
  float zcv_ms;
};

static QueueHandle_t qFrame = nullptr;
static uint16_t curRaw[N_SAMP];
#endif

#if ENABLE_ML_LOGGER
DataLogger logger;
#endif

// ------------------- WiFiManager callback -------------------
void configModeCallback(WiFiManager *wm) {
  (void)wm;
  if (oledOk) oled.showStatus("WIFI SETUP", "AP: TinyML_Setup");
}

#if ENABLE_CURRENT_SENSOR
// ------------------- CORE 0 TASK -------------------
static void Core0Task(void* arg) {
  (void)arg;

  CurrentCalib cal; // from SmartPlugConfig.h
  ArcFeatOut featOut;

  for (;;) {
    Core0Frame fr;
    fr.uptime_ms = millis();
    fr.epoch_ms  = timeSync.nowEpochMs();

    float fs = 0.0f;
    const size_t got = currentSensor.capture(curRaw, N_SAMP, &fs);

    if (got == N_SAMP && fs > 20000.0f) {
      if (arcFeatures.compute(curRaw, N_SAMP, fs, cal, MAINS_F0_HZ, featOut)) {
        fr.fs_hz    = featOut.fs_hz;
        fr.irms_a   = featOut.irms_a;
        fr.thd_pct  = featOut.thd_pct;
        fr.entropy  = featOut.entropy;
        fr.zcv_ms   = featOut.zcv_ms;
        xQueueOverwrite(qFrame, &fr);
      }
    }

    vTaskDelay(1);
  }
}
#endif

// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin();
  Wire.setClock(400000);

  // ESP32 ADC 12-bit for voltage/temp
  analogReadResolution(12);

  // Start sensors (voltage/temp are ESP32 ADC)
  voltSensor.begin();
  tempSensor.begin();

  // OLED
  oledOk = oled.begin();
  if (oledOk) oled.showStatus("SYSTEM", "Starting...");

  // Outputs early (avoid floating outputs)
  actuators.begin(PIN_RELAY, PIN_BUZZER_PWM, PIN_RESET_BTN, &oled);

  // WiFiManager + time
  netManager.begin(configModeCallback);
  timeSync.begin("Asia/Manila");

  // Cloud + OTA
  cloudHandler.begin(API_KEY, DATABASE_URL);

  pullOta.begin(FW_VERSION, &cloudHandler);
  pullOta.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  pullOta.setCheckInterval(OTA_CHECK_INTERVAL_MS);
  pullOta.setInsecureTLS(true);
  pullOta.requestCheckNow();

#if ENABLE_ML_LOGGER
  logger.begin(&cloudHandler);
  logger.setEnabled(true);             // default ON
  logger.setDurationSeconds(ML_LOG_DURATION_S);
#endif

#if ENABLE_CURRENT_SENSOR
  // IMPORTANT: this is the only place ADS8684 is touched.
  // If you don’t have ADS8684 yet, keep ENABLE_CURRENT_SENSOR=0 in SmartPlugConfig.h.
  if (oledOk) oled.showStatus("CURRENT", "Init ADC...");
  bool ok = currentSensor.begin();

  if (!ok) {
    // Do NOT crash / restart. Continue without current features.
    if (oledOk) oled.showStatus("WARN", "No ADS8684");
    Serial.println("[WARN] ADS8684 init failed. Disabling current pipeline.");
  } else {
    qFrame = xQueueCreate(1, sizeof(Core0Frame));
    xTaskCreatePinnedToCore(Core0Task, "Core0Sense", 12288, nullptr, 3, nullptr, 0);
    if (oledOk) oled.showStatus("CURRENT", "Ready");
  }
#else
  Serial.println("[INFO] ENABLE_CURRENT_SENSOR=0 (no ADS8684).");
  if (oledOk) oled.showStatus("MODE", "No Current ADC");
#endif

  delay(500);
  if (oledOk) oled.showStatus("SYSTEM", "Ready");
}

// ------------------- Loop (Core 1) -------------------
void loop() {
  netManager.update();
  timeSync.update();

  // Latch reset button
  if (actuators.resetLongPressed()) {
    faultLogic.resetLatch();
    if (oledOk) oled.showStatus("RESET", "Latch cleared");
    delay(250);
  }

  // 2. Slow Sensors (Non-blocking)
  static float vRms = 0.0f;
  static float tC = 0.0f;
  static uint32_t tT = 0;

  float newV = voltSensor.update();
  if (newV >= 0.0f) vRms = newV;

  if (millis() - tT > 500) {
    tT = millis();
    tC = tempSensor.readTempC();
  }

  // Defaults if current pipeline not available
  float iRms = 0.0f;
  float thd = 0.0f;
  float entropy = 0.0f;
  float zcv = 0.0f;

#if ENABLE_CURRENT_SENSOR
  if (qFrame != nullptr) {
    Core0Frame fr;
    if (xQueueReceive(qFrame, &fr, 0) == pdTRUE) {
      iRms = fr.irms_a;
      thd = fr.thd_pct;
      entropy = fr.entropy;
      zcv = fr.zcv_ms;
    }
  }
#endif

  // Model + logic
  const int arcPred = ArcPredict(entropy, thd, zcv, vRms, iRms, tC);
  FaultState st = faultLogic.update(tC, iRms, arcPred);

  // Outputs
  actuators.apply(st, vRms, iRms, tC);

#if ENABLE_ML_LOGGER
  // Only log when enabled (your toggling mechanism can flip this)
  FeatureFrame f;
  f.epoch_ms  = timeSync.nowEpochMs();
  f.uptime_ms = millis();
  f.vrms = vRms;
  f.irms = iRms;
  f.temp_c = tC;
  f.zcv_ms = zcv;
  f.thd_pct = thd;
  f.entropy = entropy;
  f.model_pred = (uint8_t)arcPred;

  logger.ingest(f, st, faultLogic.arcCounter());
  logger.loop();
#endif

  // Cloud + OTA only if WiFi connected
  if (netManager.isConnected()) {
    cloudHandler.update(
      vRms, iRms, tC,
      zcv, thd, entropy,
      String(stateToCstr(st)),
      &timeSync
    );
    pullOta.loop();
  }

  delay(1);
}