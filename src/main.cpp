#include <Arduino.h>
#include "monitor.h" // Include your custom header

void setup() {
    // Setup is now clean; it just calls the initializer
    initMonitor();
}

void loop() {
    // The main loop reads like English, not code
    float currentVoltage = readSystemVoltage();
    
    Serial.print("System Voltage: ");
    Serial.println(currentVoltage);
    
    blinkStatus(500);
}