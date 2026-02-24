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
static const char* FW_VERSION = "TSP-v0.2.0"; 

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

    if (got == N_SAMP && fs > 20000.0f) {
      
      bool success = arcFeat.compute(s_raw, N_SAMP, fs, curCalib, MAINS_F0_HZ, out);
      
      if (success) {
        f.irms    = out.irms_a;
        f.thd_pct = out.thd_pct;
        f.entropy = out.entropy;
        f.zcv_ms  = out.zcv_ms;

        // Push to Core 1. We use xQueueOverwrite which requires a queue size of exactly 1.
        if (qFeat != nullptr) {
          xQueueOverwrite(qFeat, &f);
        }
      } else {
        // --- CATCH THE FAILURE ---
        Serial.println("[MATH FAILED] arcFeat.compute() actively rejected the data!");
      }
    } else {
      Serial.printf("[SPI ERROR] Incomplete capture or low sampling rate! Got: %d | FS: %.1f Hz\n", got, fs);
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

  // 3. Slow Sensors (Non-blocking) + smoothing
  static float vRms = 0.0f;
  static float tC = 0.0f;
  static uint32_t tT = 0;

  constexpr float V_EMA_ALPHA = 0.20f; // 0.1–0.3 good
  constexpr float T_EMA_ALPHA = 0.10f; // slower

  float newV = voltSensor.update();
  if (newV >= 0.0f) {
    if (vRms <= 0.0f) vRms = newV;
    else vRms = (1.0f - V_EMA_ALPHA) * vRms + V_EMA_ALPHA * newV;
  }

  if (millis() - tT > 500) {
    tT = millis();
    float newT = tempSensor.readTempC();
    if (newT > -50.0f && newT < 120.0f) { // ignore -99 + outliers
      if (tC <= -50.0f || tC >= 120.0f) tC = newT;
      else tC = (1.0f - T_EMA_ALPHA) * tC + T_EMA_ALPHA * newT;
    }
  }

  // 4. GET DATA FROM CORE 0 (KEEP LAST GOOD FRAME)
  static FeatureFrame lastF = {};
  static bool hasLast = false;

  FeatureFrame f;
  bool gotData = false;

#if ENABLE_CURRENT_SENSOR
  if (qFeat != nullptr) {
    gotData = (xQueueReceive(qFeat, &f, 0) == pdTRUE);
  }
#endif

  if (gotData) {
    lastF = f;
    hasLast = true;
  } else if (hasLast) {
    f = lastF; // reuse last valid frame instead of forcing zeros
  } else {
    // still waiting for first frame
    memset(&f, 0, sizeof(f));
  }

  // 3b. Moving average for slow sensors (see section 3 below)
  // Fill in core-1 gathered values
  f.vrms   = vRms;
  f.temp_c = tC;

  // 4b. TRANSIENT HOLDOFF (place it HERE: after f.irms is valid, before prediction)
  static float prevIrms = 0.0f;
  const float dI = fabsf(f.irms - prevIrms);
  prevIrms = f.irms;

  static uint32_t transientUntil = 0;
  // Tune: fan OFF spike seems big; start with 0.6A and 600ms
  if (dI > 0.6f) transientUntil = millis() + 600;
  const bool isTransient = (millis() < transientUntil);

  // Predict Fault State (suppress prediction during transient)
  int pred = ArcPredict(f.entropy, f.thd_pct, f.zcv_ms, f.vrms, f.irms, f.temp_c);
  if (isTransient) pred = 0;
  f.model_pred = pred;

  FaultState state = faultLogic.update(tC, f.irms, f.model_pred);

  // Apply outputs (Data collection mode: never trip relay)
  FaultState stateOut = state;
#ifdef DATA_COLLECTION_MODE
  stateOut = STATE_NORMAL;
#endif
  actuators.apply(stateOut, vRms, f.irms, tC);

#if ENABLE_ML_LOGGER
  // log only when a NEW frame arrived
  if (gotData) {
    logger.ingest(f, state, faultLogic.arcCounter());
  }
#endif

  // 5. SLOW LIVE DASHBOARD UPDATE (Every 5 seconds)
  static uint32_t lastCloudUpdate = 0;
  if (millis() - lastCloudUpdate > 5000) {
    lastCloudUpdate = millis();
    cloud.update(vRms, f.irms, tC, f.zcv_ms, f.thd_pct, f.entropy,
                 String(stateToCstr(stateOut)), &timeSync);
  }
}