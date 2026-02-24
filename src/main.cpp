#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// --- Config & Types ---
#include "SmartPlugConfig.h"
#include "SmartPlugTypes.h"

// --- Modules ---
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

// Include ML Logger if enabled
#if ENABLE_ML_LOGGER
  #include "DataLogger.h"
#endif

// Conditionally include Current Sensor / FFT if enabled
#if ENABLE_CURRENT_SENSOR
  #include "CurrentSensor.h"
  #include "ArcFeatures.h"
#endif

// --- Firebase Configuration ---
#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"
#define DATABASE_URL "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"
static const char* FW_VERSION = "TSP-v0.1.13"; 

// --- OTA Constants ---
static const char* OTA_DESIRED_VERSION_PATH = "/ota/desired_version";
static const char* OTA_FIRMWARE_URL_PATH    = "/ota/firmware_url";
static const uint32_t OTA_CHECK_INTERVAL_MS = 60000;

// --- Global Objects ---
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

#if ENABLE_CURRENT_SENSOR
CurrentSensor   curSensor; 
ArcFeatures     arcFeat;
CurrentCalib    curCalib; // default calibration
static uint16_t s_raw[N_SAMP]; // RAW ADC Buffer
QueueHandle_t   qFeat = nullptr;

// -------------------------------------------------------------------------
// Core 0 Task (High Speed Pipeline)
// -------------------------------------------------------------------------
void Core0Task(void* pvParameters) {
  ArcFeatOut out;
  
  Serial.println("\n[Core0] TASK HAS SUCCESSFULLY STARTED!");

  while (true) {
    FeatureFrame f;
    f.uptime_ms = millis();
    f.epoch_ms  = timeSync.isSynced() ? timeSync.nowEpochMs() : 0;

    float fs = 0.0f;
    const size_t got = curSensor.capture(s_raw, N_SAMP, &fs);

// --- DEBUG BLOCK ---
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 1000) {
      lastPrint = millis();
      // Added 'fs' to the printout so we can see the exact speed!
      Serial.printf("[SPI DEBUG] Captured: %d | RAW: %u | FS: %.1f Hz\n", got, s_raw[0], fs);
    }
    // -------------------

    // LOWERED THRESHOLD TO 10 kHz to accommodate 1 MHz SPI
    if (got == N_SAMP && fs > 10000.0f) {
      if (arcFeat.compute(s_raw, N_SAMP, fs, curCalib, MAINS_F0_HZ, out)) {
        if (arcFeat.compute(s_raw, N_SAMP, fs, curCalib, MAINS_F0_HZ, out)) {
          f.irms    = out.irms_a;
          f.thd_pct = out.thd_pct;
          f.entropy = out.entropy;
          f.zcv_ms  = out.zcv_ms;

          // Push to Core 1. We use xQueueOverwrite which requires a queue size of exactly 1.
          if (qFeat != nullptr) {
            xQueueOverwrite(qFeat, &f);
          }
        }
      }
    }

    // Give FreeRTOS a tiny breather to feed the idle task and prevent watchdog resets
    vTaskDelay(1);
  }
}
#endif

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000); 

  // 1. Hardware Init
  oled.begin();
  oled.showStatus("System", "Starting...");
  
  actuators.begin(PIN_RELAY, PIN_BUZZER_PWM, PIN_RESET_BTN, &oled);
  
  // 2. Sensor Init
  voltSensor.begin();
  tempSensor.begin();
  
  // 3. SPI Sensor Init & Core 0 Start
#if ENABLE_CURRENT_SENSOR
  oled.showStatus("CURRENT", "Init ADC...");
  bool hwReady = curSensor.begin();
  
  if (!hwReady) {
    Serial.println("!!! WARNING: ADS8684 Missing or Failed !!!");
    oled.showStatus("WARN", "No ADS8684");
    delay(2000); 
  } else {
    Serial.println("ADS8684 Initialized OK.");
    oled.showStatus("CURRENT", "Ready");
    
    // Create queue with exactly 1 element so xQueueOverwrite works
    qFeat = xQueueCreate(1, sizeof(FeatureFrame));
    
    // Launch the Pipeline Task on Core 0
    xTaskCreatePinnedToCore(Core0Task, "Core0Sense", 12288, nullptr, 3, nullptr, 0);
  }
#else
  Serial.println("[Init] SPI Sensor Disabled in Config.");
  oled.showStatus("Warning", "Sensor OFF");
  delay(1000);
#endif

  // 4. Network Connection
  oled.showStatus("WiFi", "Connecting...");
  net.begin([](WiFiManager* wm) {
    oled.showStatus("Setup Mode", "Connect to AP");
  });

  // 5. Cloud & Time
  oled.showStatus("Cloud", "Connecting...");
  cloud.begin(API_KEY, DATABASE_URL);
  timeSync.begin();
  
#if ENABLE_ML_LOGGER
  logger.begin(&cloud);
  logger.setEnabled(true); // Force true for testing
  logger.setDurationSeconds(10); 
#endif

  // 6. OTA Setup
  ota.begin(FW_VERSION, &cloud);
  ota.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  ota.setCheckInterval(OTA_CHECK_INTERVAL_MS);

  // Ready!
  oled.showStatus("System", "Ready");
  Serial.println("Setup Complete.");
}

// -------------------------------------------------------------------------
// Loop (Core 1)
// -------------------------------------------------------------------------
void loop() {
  // 1. Maintain background tasks
  net.update();
  ota.loop();
  timeSync.update();
#if ENABLE_ML_LOGGER
  logger.loop();
#endif

  // 2. Reset Button Logic
  if (actuators.resetLongPressed()) {
    faultLogic.resetLatch();
    oled.showStatus("RESET", "Latch cleared");
    delay(250);
  }

  // 3. Slow Sensors (Non-blocking)
  static float vRms = 0.0f;
  static float tC = 0.0f;
  static uint32_t tT = 0;

  float newV = voltSensor.update();
  if (newV >= 0.0f) vRms = newV;

  if (millis() - tT > 500) {
    tT = millis();
    tC = tempSensor.readTempC();
  }

  // 4. GET DATA FROM CORE 0 AND INGEST AS FAST AS POSSIBLE
  FeatureFrame f;
  bool gotData = false;
  
#if ENABLE_CURRENT_SENSOR
  if (qFeat != nullptr) {
    gotData = (xQueueReceive(qFeat, &f, 0) == pdTRUE);
  }
#endif

  // If no hardware or queue is empty, populate with zeroes to keep loop alive
  if (!gotData) {
    f.irms = 0.0f;
    f.zcv_ms = 0.0f;
    f.thd_pct = 0.0f;
    f.entropy = 0.0f;
  }
  
  // Fill in core-1 gathered values
  f.vrms = vRms;
  f.temp_c = tC;

  // Predict Fault State
  f.model_pred = ArcPredict(f.entropy, f.thd_pct, f.zcv_ms, f.vrms, f.irms, f.temp_c);
  FaultState state = faultLogic.update(tC, f.irms, f.model_pred);

  // Apply outputs
  actuators.apply(state, vRms, f.irms, tC);

#if ENABLE_ML_LOGGER
  if (gotData) {
    // THIS HAPPENS ~30 TIMES A SECOND (Does not block)
    logger.ingest(f, state, faultLogic.arcCounter()); 
  }
#endif

  // 5. SLOW LIVE DASHBOARD UPDATE (Every 5 seconds)
  static uint32_t lastCloudUpdate = 0;
  if (millis() - lastCloudUpdate > 5000) { 
    lastCloudUpdate = millis();
    
    // This blocks for ~1-2 seconds, but the logger buffer is safe.
    cloud.update(vRms, f.irms, tC, f.zcv_ms, f.thd_pct, f.entropy, 
                 String(stateToCstr(state)), &timeSync);
  }
}