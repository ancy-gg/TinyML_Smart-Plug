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
static const char* FW_VERSION = "TSP-v0.1.2-NoWDT"; 

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

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  
  // 1. Hardware Init
  oled.begin();
  oled.showStatus("System", "Starting...");
  
  actuators.begin(PIN_RELAY, PIN_BUZZER_PWM, PIN_RESET_BTN, &oled);
  
  // 2. Sensor Init
  voltSensor.begin();
  tempSensor.begin();
  
  // SPI Init for ADS8684
  if (!curSensor.begin()) {
    Serial.println("ADS8684 Init Failed!");
    oled.showStatus("Error", "ADS Init Fail");
    delay(2000); // Allow user to read error
  }

  // 3. Network Connection (Blocking)
  oled.showStatus("WiFi", "Connecting...");
  net.begin([](WiFiManager* wm) {
    oled.showStatus("Setup Mode", "Connect to AP");
  });

  // 4. Start Core 0 Pipeline (High Speed Data)
  qFeat = xQueueCreate(3, sizeof(FeatureFrame));
  if (!core0.begin(qFeat, &curSensor, &arcFeat)) {
    Serial.println("Core0 Start Failed!");
  }
  
  // Default Calibration
  CurrentCalib cal; 
  core0.setCalib(cal);
  
  // Time Provider for Logs
  core0.setTimeProvider([]() -> uint64_t {
    return timeSync.nowEpochMs();
  });

  // 5. Cloud & Time (Blocking)
  oled.showStatus("Cloud", "Connecting...");
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
  Serial.println("Setup Complete. No Watchdog.");
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

  // 2. Handle Manual Reset Button
  if (actuators.resetLongPressed()) {
    faultLogic.resetLatch();
    oled.showStatus("RESET", "Latch cleared");
    delay(250);
  }

  // 3. Slow Sensors (Non-blocking)
  static float vRms = 0.0f;
  static float tC = 0.0f;
  static uint32_t tT = 0;

  // Voltage: Call update() every loop. 
  // IMPORTANT: Ensure you updated VoltageSensor.h/.cpp to the non-blocking version!
  float newV = voltSensor.update();
  if (newV >= 0.0f) {
    vRms = newV;
  }

  // Temp: Read every 500ms
  if (millis() - tT > 500) {
    tT = millis();
    tC = tempSensor.readTempC();
  }

  // 4. Get Data from Core 0
  FeatureFrame f;
  // Wait 0 ticks so we don't block
  bool got = (xQueueReceive(qFeat, &f, 0) == pdTRUE);

  if (!got) {
    // Queue empty? Skip logic to avoid flickering 0 values.
    return; 
  }

  // Add slow sensor data to frame
  f.vrms = vRms;
  f.temp_c = tC;

  // 5. Fault Logic
  FaultState state = faultLogic.update(tC, f.irms, f.model_pred);

  // 6. Actuators & Display
  actuators.apply(state, vRms, f.irms, tC);

  // 7. Cloud Update
  cloud.update(vRms, f.irms, tC, f.zcv_ms, f.thd_pct, f.entropy,
               String(stateToCstr(state)), &timeSync);

  // 8. Data Logging
  logger.ingest(f, state, faultLogic.arcCounter());
}