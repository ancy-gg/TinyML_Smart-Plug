#include "TempSensor.h"
#include <math.h>

TempSensor::TempSensor(int pin) {
    _pin = pin;
}

void TempSensor::begin() {
    pinMode(_pin, INPUT);
}

float TempSensor::readTempC() {
    // Read voltage in mV directly from ESP32 calibration
    uint32_t mv = analogReadMilliVolts(_pin);
    float vOut = mv / 1000.0f; // Convert to Volts

    // Safety: If voltage is near 0 (unplugged) or max (shorted), return error
    if (vOut < 0.1) return -99.0f; 

    // Calculate Resistance of NTC
    // V_out = VCC * R_ntc / (R_divider + R_ntc)
    // => R_ntc = (V_out * R_divider) / (VCC - V_out)
    
    // Prevent divide by zero if V_out approx VCC (which shouldn't happen on 3.3V pin with 5V source unless clipped)
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