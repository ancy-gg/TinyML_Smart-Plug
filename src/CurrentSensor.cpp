#include "CurrentSensor.h"

CurrentSensor::CurrentSensor(int pin) {
    _pin = pin;
    _mvPerAmp = 100.0f; // Default to ACS712-20A (100mV/A)
}

void CurrentSensor::begin() {
    pinMode(_pin, INPUT);
}

void CurrentSensor::setMvPerAmp(float mvPerAmp) {
    _mvPerAmp = mvPerAmp;
}

float CurrentSensor::readCurrentRMS() {
    int maxVal = 0;
    int minVal = 4096;
    unsigned long start = millis();

    // Sample for 40ms to catch peaks
    while (millis() - start < 40) {
        int val = analogRead(_pin);
        if (val > maxVal) maxVal = val;
        if (val < minVal) minVal = val;
    }

    // Filter noise
    if ((maxVal - minVal) < 10) return 0.0f;

    // Calculate Peak-to-Peak Voltage in mV
    // (ADC_Value * 3300mV / 4095)
    float voltagePkPk = (maxVal - minVal) * (3300.0f / 4095.0f);

    // RMS Voltage of the AC signal
    float voltageRMS = (voltagePkPk / 2.0f) * 0.7071f;

    // Convert to Amps
    float amps = voltageRMS / _mvPerAmp;

    // Noise Gate: Ignore tiny currents < 100mA
    if (amps < 0.10f) amps = 0.0f;

    return amps;
}