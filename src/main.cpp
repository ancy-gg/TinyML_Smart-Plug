#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include "NetworkManager.h"
#include "CloudHandler.h"
#include "Simulation.h"
#include "OLED_NOTIF.h"
#include "TimeSync.h"
#include "PullOTA.h"

// ======================
// FIREBASE CONFIG
// ======================
// NOTE: CloudHandler in your project uses test_mode=true, so API_KEY isn't actually required,
// but keeping it is fine.
#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"

// IMPORTANT: use full https URL + trailing slash for Firebase RTDB
#define DATABASE_URL "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"

// ======================
// FIRMWARE VERSION (BUMP PER RELEASE)
// ======================
static const char* FW_VERSION = "TSPfw-v0.2.0";

// ======================
// PULL-OTA RTDB PATHS (Option B from PWA)
// ======================
static const char* OTA_DESIRED_VERSION_PATH = "/ota/desired_version";
static const char* OTA_FIRMWARE_URL_PATH    = "/ota/firmware_url";

// How often ESP checks Firebase for updates
static const uint32_t OTA_CHECK_INTERVAL_MS = 60 * 1000; // 60 seconds

// ======================
// APP OBJECTS
// ======================
NetworkManager netManager;
CloudHandler cloudHandler;
Simulation sim;
OLED_NOTIF oled(0x3C);
TimeSync timeSync;
PullOTA pullOta;

// ======================
// THRESHOLDS / STATE
// ======================
const float TEMP_THRESHOLD = 70.0;
const float CURRENT_THRESHOLD = 10.0;

FaultState currentState = STATE_NORMAL;

// ---- OLED Sample-and-hold ----
static unsigned long lastOledUpdate = 0;
static const unsigned long OLED_HOLD_MS = 250;

// ---- Startup screen state machine (non-blocking) ----
enum BootStage { BOOTING, SHOW_IP, SHOW_OTA, SHOW_FIREBASE, RUNNING };
static BootStage bootStage = BOOTING;
static unsigned long bootStageStart = 0;

void configModeCallback(WiFiManager *myWiFiManager) {
  (void)myWiFiManager;
  Serial.println("[Main] AP Mode Active");
  oled.showStatus("WIFI SETUP", "AP: TinyML_Setup");
}

static String stateToString(FaultState s) {
  switch (s) {
    case STATE_ARCING:   return "ARCING";
    case STATE_HEATING:  return "HEATING";
    case STATE_OVERLOAD: return "OVERLOAD";
    default:             return "HAPPY";
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  oled.begin();

  oled.showStatus("SYSTEM", "Booting...");
  bootStage = BOOTING;
  bootStageStart = millis();

  // WiFi connect (WiFiManager blocks here until connected or AP mode)
  netManager.begin(configModeCallback);

  // Start time sync (non-blocking)
  timeSync.begin("Asia/Manila");

  // Start Firebase
  cloudHandler.begin(API_KEY, DATABASE_URL);

  // Start Pull-OTA (HTTPS)
  pullOta.begin(FW_VERSION, &cloudHandler);
  pullOta.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  pullOta.setCheckInterval(OTA_CHECK_INTERVAL_MS);
  pullOta.requestCheckNow();

  // Show boot screens sequence (non-blocking in loop)
  bootStage = SHOW_IP;
  bootStageStart = millis();

  Serial.println("\n==============================");
  Serial.printf("[Main] FW_VERSION: %s\n", FW_VERSION);
  Serial.println("[Main] OTA mode: HTTPS Pull-OTA");
  Serial.println("[Main] PWA writes: /ota/desired_version + /ota/firmware_url");
  Serial.println("==============================\n");
}

void loop() {
  // Keep these ALWAYS running
  netManager.update();
  timeSync.update();

  // Non-blocking boot screen flow (each shows ~2 seconds)
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
        // Pull-OTA "ready" means WiFi connected; it checks Firebase on interval
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
        oled.showStatus("FIREBASE", "Running...");
        Serial.println("[Main] Firebase running...");
      }
      if (elapsed >= 1200) {
        bootStage = RUNNING;
      }
      delay(1);
      return;
    }
  }

  // ----- MAIN RUNNING -----
  SimData data = sim.getCycleData();

  // State logic
  if (data.tinyMLOutput == 1) currentState = STATE_ARCING;
  else if (data.temp > TEMP_THRESHOLD) currentState = STATE_HEATING;
  else if (data.current > CURRENT_THRESHOLD) currentState = STATE_OVERLOAD;
  else currentState = STATE_NORMAL;

  // OLED sample-and-hold (update only every OLED_HOLD_MS)
  if (millis() - lastOledUpdate >= OLED_HOLD_MS) {
    lastOledUpdate = millis();
    oled.updateDashboard(data.voltage, data.current, data.temp, currentState);
  }

  // Cloud upload (CloudHandler handles 10s/ fault rules internally)
  if (netManager.isConnected()) {
    cloudHandler.update(
      data.voltage, data.current, data.temp,
      data.zcv, data.thd, data.entropy,
      stateToString(currentState),
      &timeSync
    );

    // HTTPS Pull-OTA check + update
    pullOta.loop();
  }

  delay(1);
}