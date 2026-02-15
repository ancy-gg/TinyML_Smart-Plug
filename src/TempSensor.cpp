#include "TempSensor.h"
#include <math.h>

TempSensor::TempSensor(int pin) {
    _pin = pin;
}

void TempSensor::begin() {
    pinMode(_pin, INPUT);
}

float TempSensor::readTempC() {
    uint32_t mv = analogReadMilliVolts(_pin);
    float vOut = mv / 1000.0f;

    // Error when uplugged or open
    if (vOut < 0.1) return -99.0f; 

    // Calculate Resistance of NTC
    if (vOut >= VCC - 0.1) vOut = VCC - 0.1; 
    float rNtc = (vOut * R_DIVIDER) / (VCC - vOut);

    // Steinhart-Hart Equation
    float steinhart;
    steinhart = rNtc / R_NTC_NOMINAL;             // (R/Ro)
    steinhart = log(steinhart);                   // ln(R/Ro)
    steinhart /= B_COEFF;                         // 1/B * ln(R/Ro)
    steinhart += 1.0f / (TEMP_NOMINAL + 273.15f); // + (1/To)
    steinhart = 1.0f / steinhart;                 // Invert
    steinhart -= 273.15f;                         // Convert to Celsius

    return steinhart;
}