#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <Arduino.h>

class TempSensor {
public:
  TempSensor(int pin);
  void begin();
  float readTempC();

  void setLongAverage(float tauS, float jumpC) {
    if (tauS < 1.0f) tauS = 1.0f;
    if (tauS > 30.0f) tauS = 30.0f;
    if (jumpC < 0.3f) jumpC = 0.3f;
    if (jumpC > 5.0f) jumpC = 5.0f;
    _avgTauS = tauS;
    _avgJumpC = jumpC;
  }

private:
  int _pin;

  const float R_DIVIDER = 3300.0f;
  const float VCC = 3.3f;
  const float R_NTC_NOMINAL = 10000.0f;
  const float B_COEFF = 3950.0f;
  const float TEMP_NOMINAL = 25.0f;

  bool _filtInit = false;
  float _filtTempC = 0.0f;
  uint32_t _lastReadMs = 0;

  float _avgTauS  = 8.0f;
  float _avgJumpC = 1.0f;
};

#endif
