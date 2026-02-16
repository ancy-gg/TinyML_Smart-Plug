#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
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
#include "CurrentSensor.h"

#include "ArcFeatures.h"
#include "ArcModel.h"
#include "FaultLogic.h"
#include "Actuators.h"
#include "Core0Pipeline.h"
#include "DataLogger.h"

// --- Firebase Configuration ---
#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"
#define DATABASE_URL "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"
static const char* FW_VERSION = "TSP-v0.1.3"; 

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
CurrentSensor   curSensor; 

ArcFeatures     arcFeat;
FaultLogic      faultLogic;
Actuators       actuators;
Core0Pipeline   core0;
DataLogger      logger;

// Inter-core Queue
QueueHandle_t qFeat = nullptr;
bool hardwareReady = false; // Flag to track if ADC is present

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  // Wait a moment for Serial to stabilize so you can see logs
  delay(2000); 
  Serial.println("\n\n--- BOARD BOOT ---");

  // 1. Hardware Init
  oled.begin();
  oled.showStatus("System", "Starting...");
  Serial.println("[Init] OLED Done");
  
  actuators.begin(PIN_RELAY, PIN_BUZZER_PWM, PIN_RESET_BTN, &oled);
  Serial.println("[Init] Actuators Done");
  
  // 2. Sensor Init
  voltSensor.begin();
  tempSensor.begin();
  
  // --- CRITICAL FIX: DISABLE SPI SENSOR FOR NOW ---
  // The line below is likely causing the crash because the pins are floating.
  // hardwareReady = curSensor.begin(); <--- COMMENTED OUT
  
  hardwareReady = false; // Force "Not Ready" manually
  Serial.println("[Init] SPI Sensor SKIPPED (Safe Mode)");
  
  // Show warning on screen so you know it's intentional
  oled.showStatus("Warning", "Sensors OFF");
  delay(1000); 

  // 3. Network Connection
  oled.showStatus("WiFi", "Connecting...");
  Serial.println("[WiFi] Attempting connection...");
  
  // This might block if it needs to generate the "TinyML_Setup" AP
  net.begin([](WiFiManager* wm) {
    oled.showStatus("Setup Mode", "Connect to AP");
    Serial.println("[WiFi] Entered AP Mode");
  });
  Serial.println("[WiFi] Connected!");

  // 4. Start Core 0 Pipeline
  // We strictly skip this if hardwareReady is false
  if (hardwareReady) {
    qFeat = xQueueCreate(3, sizeof(FeatureFrame));
    if (!core0.begin(qFeat, &curSensor, &arcFeat)) {
      Serial.println("[Core0] Start Failed!");
    } else {
      Serial.println("[Core0] Pipeline Started.");
    }
  } else {
    Serial.println("[Core0] SKIPPED (Safe Mode)");
  }
  
  // Default Calibration
  CurrentCalib cal; 
  core0.setCalib(cal);
  
  // Time Provider
  core0.setTimeProvider([]() -> uint64_t {
    return timeSync.nowEpochMs();
  });

  // 5. Cloud & Time
  oled.showStatus("Cloud", "Connecting...");
  Serial.println("[Cloud] Init...");
  cloud.begin(API_KEY, DATABASE_URL);
  timeSync.begin();
  
  logger.begin(&cloud);
  logger.setEnabled(false); 

  // OTA Setup
  ota.begin(FW_VERSION, &cloud);
  ota.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  ota.setCheckInterval(OTA_CHECK_INTERVAL_MS);

  // Ready!
  oled.showStatus("System", "Ready");
  Serial.println("Setup Complete. Entering Loop.");
}

// -------------------------------------------------------------------------
// Loop (Core 1)
// -------------------------------------------------------------------------
void loop() {
  // 1. Maintain Connections
  net.update();
  ota.loop();
  logger.loop();
  timeSync.update();

  if (actuators.resetLongPressed()) {
    faultLogic.resetLatch();
    oled.showStatus("RESET", "Latch cleared");
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

  // 3. Get Data from Core 0
  FeatureFrame f;
  bool got = false;

  // CRITICAL FIX: Only check queue if hardware was ready!
  if (hardwareReady && qFeat != nullptr) {
    got = (xQueueReceive(qFeat, &f, 0) == pdTRUE);
  }

  // If we didn't get data (queue empty OR hardware missing), handle it:
  if (!got) {
    if (!hardwareReady) {
      // SAFE MODE: Simulate a "Normal" state so OLED/Cloud still update
      FaultState st = STATE_NORMAL;
      actuators.apply(st, vRms, 0.0f, tC);
      
      // Update cloud occasionally even if no current data
      cloud.update(vRms, 0.0f, tC, 0, 0, 0, "NORMAL", &timeSync);
    }
    return; // Skip the rest of the logic
  }

  // If we DID get data (Hardware is working):
  f.vrms = vRms;
  f.temp_c = tC;

  FaultState state = faultLogic.update(tC, f.irms, f.model_pred);
  actuators.apply(state, vRms, f.irms, tC);
  cloud.update(vRms, f.irms, tC, f.zcv_ms, f.thd_pct, f.entropy,
               String(stateToCstr(state)), &timeSync);
  logger.ingest(f, state, faultLogic.arcCounter());
}