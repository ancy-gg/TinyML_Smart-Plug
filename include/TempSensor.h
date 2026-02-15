#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <Arduino.h>

class TempSensor {
public:
    TempSensor(int pin);
    void begin();
    
    float readTempC();

private:
    int _pin;
    // Circuit Parameters
    const float R_DIVIDER = 3300.0f; // 3.3k Resistor
    const float VCC = 3.3f;          // 3.3v 
    const float R_NTC_NOMINAL = 10000.0f; // 10k NTC
    const float B_COEFF = 3950.0f;   // Beta Coefficient
    const float TEMP_NOMINAL = 25.0f;
};

#endif