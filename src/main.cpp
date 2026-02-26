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

// Firebase Credentials
#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"
#define DATABASE_URL "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"
static const char* FW_VERSION = "TSP-v0.2.3"; 

// Firmware Updates
static const char* OTA_DESIRED_VERSION_PATH = "/ota/desired_version";
static const char* OTA_FIRMWARE_URL_PATH    = "/ota/firmware_url";
static const uint32_t OTA_CHECK_INTERVAL_MS = 60000;

// Global Objects
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


// Core 0 Task (Current Features)
void Core0Task(void* pvParameters) {
  ArcFeatOut out;
  
  Serial.println("\n[Core0] TASK HAS SUCCESSFULLY STARTED!");

  while (true) {
    FeatureFrame f;
    f.uptime_ms = millis();
    f.epoch_ms  = timeSync.isSynced() ? timeSync.nowEpochMs() : 0;

    float fs = 0.0f;
    const size_t got = curSensor.capture(s_raw, N_SAMP, &fs);

    if (got == N_SAMP && fs > 20000.0f) {
      
      bool success = arcFeat.compute(s_raw, N_SAMP, fs, curCalib, MAINS_F0_HZ, out);
      
      if (success) {
        f.irms    = out.irms_a;
        f.thd_pct = out.thd_pct;
        f.entropy = out.entropy;
        f.zcv_ms  = out.zcv_ms;

        // Push to Core 1 (Loop) via Queue
        if (qFeat != nullptr) {
          xQueueOverwrite(qFeat, &f);
        }
      } 
    } 
    vTaskDelay(1);
  }
}

// Setup
void setup() {
  Serial.begin(115200);
  delay(1000); 

  // Initialization
  // OLED
  oled.begin();
  oled.showStatus("System", "Starting...");
  
  // Actuators (relays off by default)
  actuators.begin(PIN_RELAY, PIN_BUZZER_PWM, PIN_RESET_BTN, &oled);
  
  // Sensors (connected to ESP ADC)
  voltSensor.begin();
  voltSensor.setWindowMs(200);                 // 200ms stable Vrms
  voltSensor.setClampHysteresis(25.0f, 35.0f); // clip-to-zero w/ hysteresis
  tempSensor.begin();
  
  // SPI ADC for current sensing
  bool hwReady = curSensor.begin();
  
  if (!hwReady) {
    Serial.println("!!! WARNING: ADS8684 Missing or Failed !!!");
    oled.showStatus("WARN", "No ADS8684");
    delay(2000); 
  } else {
    Serial.println("ADS8684 Initialized OK.");
    oled.showStatus("Sensors", "Ready");
    
    // Initialize Queue for Dual Core Communication
    qFeat = xQueueCreate(1, sizeof(FeatureFrame));
    
    // Launch the Task on Core 0
    xTaskCreatePinnedToCore(Core0Task, "Core0Sense", 12288, nullptr, 3, nullptr, 0);
  }

  // WiFi Setup
  oled.showStatus("WiFi", "Connecting...");
  net.begin([](WiFiManager* wm) {
    oled.showStatus("Setup Mode", "Connect to AP");
  });

  // Time Sync for Logging and OTA
  oled.showStatus("Cloud", "Connecting...");
  cloud.begin(API_KEY, DATABASE_URL);
  timeSync.begin();
  
#if ENABLE_ML_LOGGER
  logger.begin(&cloud);
  logger.setEnabled(false);          
  logger.setDurationSeconds(10);  
#endif

  // Firmware Update Setup
  ota.begin(FW_VERSION, &cloud);
  ota.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  ota.setCheckInterval(OTA_CHECK_INTERVAL_MS);

  // Ready!
  oled.showStatus("System", "Ready");
  Serial.println("Setup Complete.");
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

  // session required when enabled
  if (enabled && sid.length() < 3) {
    Serial.println("[ML_LOG] enabled but session_id missing; forcing off.");
    logger.setEnabled(false);
    return;
  }

  // Apply session context
  logger.setDurationSeconds((uint16_t)dur);
  logger.setSession(sid, load, labelOv);

  // Edge handling: enable/disable
  if (enabled != lastEnabled || sid != lastSession) {
    Serial.printf("[ML_LOG] enabled=%d sid=%s load=%s labelOv=%d dur=%d\n",
                  (int)enabled, sid.c_str(), load.c_str(), labelOv, dur);
    logger.setEnabled(enabled);
    lastEnabled = enabled;
    lastSession = sid;
  }
}
#endif

// Core1 Task (Main Loop)
void loop() {
  net.update();
  ota.loop();
  timeSync.update();
#if ENABLE_ML_LOGGER
  pollMlControl(cloud, logger);
#endif

  // Reset Button Logic
  if (actuators.resetLongPressed()) {
    faultLogic.resetLatch();
    oled.showStatus("RESET", "Latch cleared");
    delay(250);
  }

  // Slow Sensors (Non-blocking)
  static float vRms = 0.0f;
  static float tC = 0.0f;
  static uint32_t tT = 0;

  constexpr float V_EMA_ALPHA = 1.00f; 
  constexpr float T_EMA_ALPHA = 0.10f; 

  float newV = voltSensor.update();
  if (newV >= 0.0f) {
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

  // Get latest features from Core 0 
  static FeatureFrame lastF = {};
  static bool hasLast = false;

  FeatureFrame f;
  bool gotData = false;

  if (qFeat != nullptr) {
    gotData = (xQueueReceive(qFeat, &f, 0) == pdTRUE);
  }

  if (gotData) {
    lastF = f;
    hasLast = true;
  } else if (hasLast) {
    f = lastF; 
  } else {
    f.irms = 0.0f;
    f.zcv_ms = 0.0f;
    f.thd_pct = 0.0f;
    f.entropy = 0.0f;
  }

  // Fill core-1 values
  f.vrms   = vRms;
  f.temp_c = tC;

  // Transient Detection 
  static float prevIrms = 0.0f;
  const float dI = fabsf(f.irms - prevIrms);
  prevIrms = f.irms;

  static uint32_t transientUntil = 0;
  if (dI > 0.6f) transientUntil = millis() + 600;   // tune if needed
  const bool isTransient = (millis() < transientUntil);

  // Predict
  int pred = ArcPredict(f.entropy, f.thd_pct, f.zcv_ms, f.vrms, f.irms, f.temp_c);
  if (isTransient) pred = 0; // suppress arc vote during transient
  f.model_pred = (uint8_t)pred;

  FaultState state = faultLogic.update(tC, f.irms, f.model_pred);

  // Apply outputs
  FaultState stateOut = state;
#ifdef DATA_COLLECTION_MODE
  stateOut = STATE_NORMAL;  // never trip relay while collecting data
#endif
  actuators.apply(stateOut, vRms, f.irms, tC);

#if ENABLE_ML_LOGGER
  if (gotData) {
    logger.ingest(f, stateOut, faultLogic.arcCounter());
  }

  // REQUIRED: actually uploads buffered chunks when duration/full triggers
  logger.loop();
#endif

  // Live Dashboard
  static uint32_t lastCloudUpdate = 0;
  if (millis() - lastCloudUpdate > 5000) {
    lastCloudUpdate = millis();
    cloud.update(vRms, f.irms, tC, f.zcv_ms, f.thd_pct, f.entropy,
                 String(stateToCstr(stateOut)), &timeSync);
  }
}