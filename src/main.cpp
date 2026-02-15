#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

// Headers
#include "NetworkManager.h"
#include "CloudHandler.h"
#include "OLED_NOTIF.h"  
#include "TimeSync.h"
#include "PullOTA.h"
#include "VoltageSensor.h"
#include "CurrentSensor.h"
#include "TempSensor.h"

// Pin Definitions
#define PIN_VOLT    A0  
#define PIN_CURR    A1 
#define PIN_TEMP    A2  

// Firebase Configuration
#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"
#define DATABASE_URL "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"
static const char* FW_VERSION = "TSP-v0.0.3"; // Always update this

// OTA Paths
static const char* OTA_DESIRED_VERSION_PATH = "/ota/desired_version";
static const char* OTA_FIRMWARE_URL_PATH    = "/ota/firmware_url";
static const uint32_t OTA_CHECK_INTERVAL_MS = 60 * 1000; 

// Declarations
NetworkManager netManager;
CloudHandler cloudHandler;
OLED_NOTIF oled(0x3C);
TimeSync timeSync;
PullOTA pullOta;

VoltageSensor voltSensor(PIN_VOLT);
CurrentSensor currSensor(PIN_CURR);
TempSensor tempSensor(PIN_TEMP);

// Thresholds
const float TEMP_THRESHOLD = 70.0;
const float CURRENT_THRESHOLD = 10.0;

// Initial State
FaultState currentState = STATE_NORMAL;

// OLED Timer
static unsigned long lastOledUpdate = 0;
static const unsigned long OLED_HOLD_MS = 250;

// Boot Up Sequence
enum BootStage { BOOTING, SHOW_IP, SHOW_OTA, SHOW_FIREBASE, RUNNING };
static BootStage bootStage = BOOTING;
static unsigned long bootStageStart = 0;

// WiFi Manager
void configModeCallback(WiFiManager *myWiFiManager) {
  (void)myWiFiManager;
  Serial.println("[Main] AP Mode Active");
  oled.showStatus("WIFI SETUP", "AP: TinyML_Setup");
}

// Strings for FaultState
static String stateToString(FaultState s) {
  switch (s) {
    case STATE_ARCING:   return "ARCING";
    case STATE_HEATING:  return "HEATING";
    case STATE_OVERLOAD: return "OVERLOAD";
    default:             return "NORMAL"; 
  }
}

// SETUP
void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Sensors
  voltSensor.begin();
  currSensor.begin();
  tempSensor.begin();

  // OLED
  oled.begin();
  oled.showStatus("SYSTEM", "Starting...");
  bootStage = BOOTING;
  bootStageStart = millis();

  //WiFi
  netManager.begin(configModeCallback);

  //Time Synchronization
  timeSync.begin("Asia/Manila");

  //Firebase
  cloudHandler.begin(API_KEY, DATABASE_URL);

  //OTA Updates
  pullOta.begin(FW_VERSION, &cloudHandler);
  pullOta.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  pullOta.setCheckInterval(OTA_CHECK_INTERVAL_MS);
  pullOta.setInsecureTLS(true);     
  pullOta.requestCheckNow();        

  //WiFi Boot Up
  bootStage = SHOW_IP;
  bootStageStart = millis();
}

// LOOP
void loop() {
  netManager.update();
  timeSync.update();

  // Boot Up Sequence
  if (bootStage != RUNNING) {
    const unsigned long elapsed = millis() - bootStageStart;

    if (bootStage == SHOW_IP) {
      if (elapsed < 50) {
        String ipStr = "IP: ";
        ipStr += WiFi.localIP().toString();
        oled.showStatus("CONNECTED", ipStr.c_str());
        Serial.println(ipStr);
      }
      if (elapsed >= 2000) {
        bootStage = SHOW_OTA;
        bootStageStart = millis();
      }
      delay(1);
      return;
    }

    if (bootStage == SHOW_OTA) {
      if (elapsed < 50) {
        String msg = netManager.isConnected() ? "PULL READY" : "NO WIFI";
        oled.showStatus("OTA (HTTPS)", msg.c_str());
        Serial.printf("[Main] OTA %s\n", msg.c_str());
      }
      if (elapsed >= 2000) {
        bootStage = SHOW_FIREBASE;
        bootStageStart = millis();
      }
      delay(1);
      return;
    }

    if (bootStage == SHOW_FIREBASE) {
      if (elapsed < 50) {
        oled.showStatus("FIREBASE", "Engaged...");
        Serial.println("[Main] Firebase running...");
      }
      if (elapsed >= 1200) {
        bootStage = RUNNING;
      }
      delay(1);
      return;
    }
  }

  // MAIN LOOP
  float v = voltSensor.readVoltageRMS();
  float c = currSensor.readCurrentRMS();
  float t = tempSensor.readTempC();

  // TinyML Inference (Placeholder)
  float zcv = 0.0f;
  float thd = 0.0f;
  float entropy = 0.0f;

  // Determine Fault State
  if (c > CURRENT_THRESHOLD) currentState = STATE_OVERLOAD;
  else if (t > TEMP_THRESHOLD) currentState = STATE_HEATING;
  else currentState = STATE_NORMAL;

  // Update Display
  if (millis() - lastOledUpdate >= OLED_HOLD_MS) {
    lastOledUpdate = millis();
    oled.updateDashboard(v, c, t, currentState); 
  }

  //Cloud Updates
  if (netManager.isConnected()) {
    cloudHandler.update(
      v, c, t,
      zcv, thd, entropy,
      stateToString(currentState),
      &timeSync
    );

    pullOta.loop();
  }

  delay(1);
}