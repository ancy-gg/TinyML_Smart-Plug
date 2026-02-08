#include <Arduino.h>
#include "NetworkManager.h"
#include "CloudHandler.h"
#include "Simulation.h"
#include "OLED_NOTIF.h"

// --- CONFIG ---
#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"
#define DATABASE_URL "https://tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"

// --- MODULES ---
NetworkManager netManager;
CloudHandler cloudHandler;
Simulation sim;
OLED_NOTIF oled(0x3C);

// --- LOGIC ---
const float TEMP_THRESHOLD = 70.0;    
const float CURRENT_THRESHOLD = 10.0; 
FaultState currentState = STATE_NORMAL;

void setup() {
    Serial.begin(115200);
    
    // 1. Hardware Init
    if (!oled.begin()) Serial.println("OLED Failed");

    // 2. Network Init (Non-Blocking!)
    // This will start trying to connect, but immediately return so code continues.
    // If you double-reset, it enters AP mode here.
    netManager.begin();

    // 3. Cloud Init
    cloudHandler.begin(API_KEY, DATABASE_URL);
}

void loop() {
    // 1. Housekeeping (Check Wifi/Reset flags)
    netManager.update();

    // 2. Get Data (From Simulation for now)
    SimData data = sim.getCycleData();

    // 3. Determine State
    if (data.tinyMLOutput == 1) currentState = STATE_ARCING;
    else if (data.temp > TEMP_THRESHOLD) currentState = STATE_HEATING;
    else if (data.current > CURRENT_THRESHOLD) currentState = STATE_OVERLOAD;
    else currentState = STATE_NORMAL;

    // 4. Update Hardware (Always runs, even if no WiFi)
    oled.updateDashboard(data.voltage, data.current, data.temp, currentState);

    // 5. Update Cloud (Only runs if WiFi is ready)
    if (netManager.isConnected()) {
    String stateStr = "NORMAL";
    if (currentState == STATE_ARCING) stateStr = "ARCING";
    else if (currentState == STATE_HEATING) stateStr = "HEATING";
    else if (currentState == STATE_OVERLOAD) stateStr = "OVERLOAD";
    
    // Pass ALL variables
    cloudHandler.update(
        data.voltage, 
        data.current, 
        data.temp, 
        data.zcv, 
        data.thd, 
        data.entropy, 
        stateStr
    );
}
    delay(50); // Small delay for stability
}