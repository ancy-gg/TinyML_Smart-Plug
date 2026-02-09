#ifndef VOLTAGE_SENSOR_H
#define VOLTAGE_SENSOR_H

#include <Arduino.h>

class VoltageSensor {
public:
    VoltageSensor(int pin);
    void begin();
    
    // Adjust this based on comparing with a real multimeter
    // Start with default ~550.0 and tweak.
    void setSensitivity(float factor);
    
    float readVoltageRMS();

private:
    int _pin;
    float _sensitivity;
};

#endif