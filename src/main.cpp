#include <Arduino.h>
#include <Wire.h>
#include "NetworkManager.h"
#include "CloudHandler.h"
#include "Simulation.h"
#include "OLED_NOTIF.h"
#include "TimeSync.h"

// --- CONFIG ---
#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"
#define DATABASE_URL "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"

// --- OBJECTS ---
NetworkManager netManager;
CloudHandler cloudHandler;
TimeSync timeSync;
Simulation sim;
OLED_NOTIF oled(0x3C);

// --- STATE ---
const float TEMP_THRESHOLD = 70.0;    
const float CURRENT_THRESHOLD = 10.0; 
FaultState currentState = STATE_NORMAL;

// --- CALLBACK: This runs the moment the AP opens ---
void configModeCallback(WiFiManager *myWiFiManager) {
    Serial.println("[Main] AP Mode Active");
    
    // This takes priority!
    oled.showStatus("WIFI SETUP MODE", "Connect to:");
    // We can even show the AP Name on the 3rd line
    oled.showStatus("WIFI SETUP MODE", "AP: TinyML_Setup");
}

void setup() {
    Serial.begin(115200);
    Wire.begin(); 
    oled.begin();
    oled.showStatus("SYSTEM", "Booting...");

    // 1. WiFi (Blocks until success)
    netManager.begin(configModeCallback);



    // 2. Show IP
    String ipStr = "IP: ";
    ipStr += WiFi.localIP().toString(); 
    oled.showStatus("CONNECTED!", ipStr.c_str());
    Serial.println(ipStr);
    delay(1000); // Reduced delay

    // 3. Initialize Cloud with a Serial check
    Serial.println("[Main] Initializing Firebase...");
    oled.showStatus("FIREBASE", "Connecting...");
    
    // Start non-blocking SNTP
    timeSync.begin("Asia/Manila"); // fallback if needed: "CST-8"
    oled.showStatus("Time Sync", "Manila");
    cloudHandler.begin(API_KEY, DATABASE_URL);
    
    Serial.println("[Main] Setup Complete. Entering Loop.");
}

// --- LOOP TIMER ---
unsigned long lastOledFrame = 0;
const int OLED_FRAME_RATE = 33; 

void loop() {
    // 1. Network Housekeeping
    netManager.update();
    timeSync.update();

    // 2. Get Data
    SimData data = sim.getCycleData();

    // 3. Logic
    if (data.tinyMLOutput == 1) currentState = STATE_ARCING;
    else if (data.temp > TEMP_THRESHOLD) currentState = STATE_HEATING;
    else if (data.current > CURRENT_THRESHOLD) currentState = STATE_OVERLOAD;
    else currentState = STATE_NORMAL;

    // 4. Update OLED (Throttled)
    if (millis() - lastOledFrame > OLED_FRAME_RATE) {
        lastOledFrame = millis();
        oled.updateDashboard(data.voltage, data.current, data.temp, currentState);
    }

    // 5. Firebase Upload
    if (netManager.isConnected()) {
        // Convert State to String
        String stateStr = "NORMAL";
        if (currentState == STATE_ARCING) stateStr = "ARCING";
        else if (currentState == STATE_HEATING) stateStr = "HEATING";
        else if (currentState == STATE_OVERLOAD) stateStr = "OVERLOAD";
        
        cloudHandler.update(data.voltage, data.current, data.temp, 
                          data.zcv, data.thd, data.entropy, stateStr, &timeSync);
    }
}