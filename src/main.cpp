#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
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
#include "CurrentSensor.h"

#include "ArcFeatures.h"
#include "ArcModel.h"
#include "FaultLogic.h"
#include "Actuators.h"
#include "Core0Pipeline.h"
#include "DataLogger.h"
#include <esp_task_wdt.h>

// Firebase Configuration
#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"
#define DATABASE_URL "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"
static const char* FW_VERSION = "TSP-v0.1.0";

static const char* OTA_DESIRED_VERSION_PATH = "/ota/desired_version";
static const char* OTA_FIRMWARE_URL_PATH    = "/ota/firmware_url";
static const uint32_t OTA_CHECK_INTERVAL_MS = 60 * 1000;

// Globals
NetworkManager netManager;
CloudHandler cloudHandler;
OLED_NOTIF oled(0x3C);
TimeSync timeSync;
PullOTA pullOta;

VoltageSensor voltSensor(PIN_VOLT_ADC);
TempSensor tempSensor(PIN_TEMP_ADC);
CurrentSensor currentSensor;

ArcFeatures arcFeatures;
FaultLogic faultLogic;
Actuators actuators;

Core0Pipeline core0;
QueueHandle_t qFeat = nullptr;

DataLogger logger;

// WiFiManager callback
void configModeCallback(WiFiManager *wm) {
  (void)wm;
  oled.showStatus("WIFI SETUP", "AP: TinyML_Setup");
}

static uint64_t epochProvider() {
  return timeSync.nowEpochMs();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  analogReadResolution(12);

  oled.begin();
  oled.showStatus("SYSTEM", "Starting...");

  voltSensor.begin();
  tempSensor.begin();

  netManager.begin(configModeCallback);
  timeSync.begin("Asia/Manila");

  cloudHandler.begin(API_KEY, DATABASE_URL);

  pullOta.begin(FW_VERSION, &cloudHandler);
  pullOta.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  pullOta.setCheckInterval(OTA_CHECK_INTERVAL_MS);
  pullOta.setInsecureTLS(true);
  pullOta.requestCheckNow();

  currentSensor.begin();

  actuators.begin(PIN_RELAY, PIN_BUZZER_PWM, PIN_RESET_BTN, &oled);

  // queue holds only latest frame
  qFeat = xQueueCreate(1, sizeof(FeatureFrame));

  // core0 pipeline
  core0.setTimeProvider(epochProvider);
  core0.setCalib(CurrentCalib{}); // default; tune later
  core0.begin(qFeat, &currentSensor, &arcFeatures);

  esp_task_wdt_init(5, true); // 5 second timeout, panic on trigger
  esp_task_wdt_add(NULL); 

  logger.begin(&cloudHandler);
#if ENABLE_ML_LOGGER
  logger.setEnabled(false);         // default off
  logger.setDurationSeconds(10);
#endif

  oled.showStatus("READY", WiFi.localIP().toString().c_str());
}

void loop() {
  netManager.update();
  timeSync.update();

  if (actuators.resetLongPressed()) {
    faultLogic.resetLatch();
    oled.showStatus("RESET", "Latch cleared");
    delay(250);
  }

  // Voltage and Temperature Sensor
  static float vRms = 0.0f;
  static float tC = 0.0f;
  static uint32_t tV = 0, tT = 0;

  // Voltage
  float newV = voltSensor.update();
  if (newV >= 0.0f) {
    vRms = newV; 
  }

  // Temperature 
  if (millis() - tT > 500) {
    tT = millis();
    tC = tempSensor.readTempC();
  }

  // get latest feature frame from core0
  FeatureFrame f;
  bool got = (xQueueReceive(qFeat, &f, 0) == pdTRUE);

  if (!got) {
    // still show basic dashboard
    FaultState st = STATE_NORMAL;
    actuators.apply(st, vRms, 0.0f, tC);
    delay(1);
    return;
  }

  // fill core1-only fields
  f.vrms = vRms;
  f.temp_c = tC;

  // tinyml / baseline rule model
  f.model_pred = (uint8_t)ArcPredict(f.entropy, f.thd_pct, f.zcv_ms, f.vrms, f.irms, f.temp_c);

  // state decision
  FaultState st = faultLogic.update(f.temp_c, f.irms, (int)f.model_pred);

  // outputs
  actuators.apply(st, f.vrms, f.irms, f.temp_c);

  // optional logger
#if ENABLE_ML_LOGGER
  logger.ingest(f, st, faultLogic.arcCounter());
  logger.loop();
#endif

  // cloud + OTA
  if (netManager.isConnected()) {
    cloudHandler.update(
      f.vrms, f.irms, f.temp_c,
      f.zcv_ms, f.thd_pct, f.entropy,
      String(stateToCstr(st)),
      &timeSync
    );
    pullOta.loop();
  }
  
esp_task_wdt_reset();
  delay(1);
}