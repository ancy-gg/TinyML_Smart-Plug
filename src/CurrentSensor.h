#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H

#include <Arduino.h>

class CurrentSensor {
public:
    CurrentSensor(int pin);
    void begin();
    
    // Set for your specific sensor model:
    // ACS712-05B: 185 mv/A
    // ACS712-20A: 100 mv/A
    // ACS712-30A: 66 mv/A
    void setMvPerAmp(float mvPerAmp);
    
    float readCurrentRMS();

private:
    int _pin;
    float _mvPerAmp;
};

#endif