#include "VoltageSensor.h"

VoltageSensor::VoltageSensor(int pin) {
    _pin = pin;
    _sensitivity = 580.0f; // Adjust
}

void VoltageSensor::begin() {
    pinMode(_pin, INPUT);
}

void VoltageSensor::setSensitivity(float factor) {
    _sensitivity = factor;
}

float VoltageSensor::readVoltageRMS() {
    int maxVal = 0;
    int minVal = 4096;
    unsigned long start = millis();

    // Sample for 40ms (2 full cycles at 50Hz)
    while (millis() - start < 40) { 
        int val = analogRead(_pin);
        if (val > maxVal) maxVal = val;
        if (val < minVal) minVal = val;
    }

    // Filter out tiny noise (if difference is < 5 bits)
    if (maxVal - minVal < 5) return 0.0f;

    // ADC Conversion to Voltage
    float vPeakToPeak = (maxVal - minVal) * (3.3f / 4095.0f);
    
    // Calculate RMS
    float vRMS_Sensor = (vPeakToPeak / 2.0f) * 0.7071f;
    
    // Scale to Mains Voltage (e.g. 220V)
    return vRMS_Sensor * _sensitivity;
}