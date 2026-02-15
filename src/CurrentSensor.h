#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H

#include <Arduino.h>

class CurrentSensor {
public:
    CurrentSensor(int pin);
    void begin();
    void setMvPerAmp(float mvPerAmp);
    float readCurrentRMS();

private:
    int _pin;
    float _mvPerAmp;
};

#endif