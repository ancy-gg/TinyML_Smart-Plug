#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <Arduino.h>
#include "MainConfiguration.h"

class TempSensor {
public:
  TempSensor(int pin);
  void begin();
  float readTempC();
  // Dynamic socket-hotspot estimate.
  // Uses measured NTC temperature plus a conservative current/time-based rise term.
  // This keeps low-current behavior near the NTC while separating more at heavy load.
  float estimateSocketTempC(float ntcTempC, float irmsA, bool mainsPresent);

  void setLongAverage(float tauS, float jumpC) {
    if (tauS < 0.6f) tauS = 0.6f;
    if (tauS > 30.0f) tauS = 30.0f;
    if (jumpC < 0.2f) jumpC = 0.2f;
    if (jumpC > 5.0f) jumpC = 5.0f;
    _avgTauS = tauS;
    _avgJumpC = jumpC;
  }

private:
  int _pin;

  const float R_DIVIDER = 3300.0f;
  const float VCC = 3.3f;
  const float R_NTC_NOMINAL = 10000.0f;
  const float TEMP_NOMINAL = 25.0f;

  bool _filtInit = false;
  float _filtTempC = 0.0f;
  uint32_t _lastReadMs = 0;

  float _avgTauS  = 3.0f;
  float _avgJumpC = 0.75f;

  bool _socketModelInit = false;
  float _socketDeltaC = 0.0f;
  uint32_t _lastSocketModelMs = 0;
};

#endif
