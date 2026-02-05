#include "monitor.h"

// Function Definitions (The "Kitchen")

void initMonitor() {
    pinMode(STATUS_LED, OUTPUT);
    Serial.begin(115200);
}

float readSystemVoltage() {
    // Simulating a sensor read for practice
    // In a real generic project, this might be analogRead(A0)
    int rawValue = analogRead(A0);
    return rawValue * (3.3 / 4095.0); // Example conversion
}

void blinkStatus(int delayTime) {
    digitalWrite(STATUS_LED, HIGH);
    delay(delayTime);
    digitalWrite(STATUS_LED, LOW);
    delay(delayTime);
}