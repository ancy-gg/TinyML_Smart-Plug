#include <Arduino.h>
#include "OLED_NOTIF.h"

// --- CONFIGURATION ---
const float TEMP_THRESHOLD = 70.0;    
const float CURRENT_THRESHOLD = 10.0; 
const int TINYML_PIN = 15; 

OLED_NOTIF oled(0x3C);

// Sensor Variables
float voltageVal = 0.0;
float currentVal = 0.0;
float tempVal = 0.0;
int tinyMLOutput = 0;

FaultState currentState = STATE_NORMAL;

void setup() {
    Serial.begin(115200);
    pinMode(TINYML_PIN, INPUT); 
    if (!oled.begin()) {
        Serial.println("OLED Failed");
        while (1);
    }
}

void loop() {
    // ============================================================
    // 1. SIMULATION CYCLER (Changes values every 3 seconds)
    // ============================================================
    // We use integer division of millis() to create time slots.
    // % 4 gives us a cycle of 0 -> 1 -> 2 -> 3 -> 0 ...
    
    int cyclePhase = (millis() / 6000) % 4;

    switch (cyclePhase) {
        case 0: // PHASE 1: NORMAL
            voltageVal = 220.5;
            currentVal = 5.2;   // Below 10A
            tempVal = 45.0;     // Below 70C
            tinyMLOutput = 0;   // No Arcing
            break;

        case 1: // PHASE 2: OVERLOAD
            voltageVal = 218.0;
            currentVal = 15.5;  // ABove 10A!
            tempVal = 50.0;     // Normal Temp
            tinyMLOutput = 0;   // No Arcing
            break;

        case 2: // PHASE 3: HEATING
            voltageVal = 220.0;
            currentVal = 8.0;   // Normal Current
            tempVal = 85.5;     // Above 70C!
            tinyMLOutput = 0;   // No Arcing
            break;

        case 3: // PHASE 4: ARCING (Highest Priority)
            voltageVal = 215.0; // Voltage often dips during arcing
            currentVal = 2.0;   // Current might be low/erratic
            tempVal = 60.0;     // Temp might be normal
            tinyMLOutput = 1;   // ARCING DETECTED!
            break;
    }

    // ============================================================
    // 2. LOGIC LADDER (Determines the State)
    // ============================================================
    
    // PRIORITY 1: ARCING
    if (tinyMLOutput == 1) {
        currentState = STATE_ARCING;
    }
    // PRIORITY 2: HEATING
    else if (tempVal > TEMP_THRESHOLD) {
        currentState = STATE_HEATING;
    }
    // PRIORITY 3: OVERLOAD
    else if (currentVal > CURRENT_THRESHOLD) {
        currentState = STATE_OVERLOAD;
    }
    // PRIORITY 4: NORMAL
    else {
        currentState = STATE_NORMAL;
    }

    // ============================================================
    // 3. UPDATE DISPLAY
    // ============================================================
    oled.updateDashboard(voltageVal, currentVal, tempVal, currentState);

    // Keep delay short to allow the blinking animation to update smoothly
    delay(50);
}