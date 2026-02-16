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

  const float R_DIVIDER = 3300.0f;
  const float VCC = 3.3f;
  const float R_NTC_NOMINAL = 10000.0f;
  const float B_COEFF = 3950.0f;
  const float TEMP_NOMINAL = 25.0f;
};

#endif