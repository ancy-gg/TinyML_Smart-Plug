#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

class VoltageSensor {
public:
  explicit VoltageSensor(int pin);
  void begin();
  void setSensitivity(float factor);
  void setAdcFullScaleVolts(float vfs);
  void setWindowMs(uint16_t ms);
  void setClampHysteresis(float v_off, float v_on);

  // Long-term display smoothing.
  // When the newly measured Vrms changes by less than jumpV, use a slow average with tauS.
  // When the change is large, the output snaps quickly to the new value.
  void setLongAverage(float tauS, float jumpV);

  // Returns -1.0 while still accumulating the current RMS window, otherwise the filtered Vrms.
  float update();

private:
  int _pin;
  float _sensitivity;
  float _adcFullScaleV = 3.3f;

  uint32_t _windowUs = 200000;
  bool _sampling = false;
  uint32_t _startTime = 0;

  uint32_t _count = 0;
  double   _sum   = 0.0;
  double   _sumSq = 0.0;

  bool  _vActive = false;
  float _vOff    = 15.0f;
  float _vOn     = 20.0f;

  bool  _avgInit   = false;
  float _avgVrms   = 0.0f;
  float _avgTauS   = 10.0f;
  float _avgJumpV  = 10.0f;

  static inline int _median3(int a, int b, int c) {
    if (a > b) { int t = a; a = b; b = t; }
    if (b > c) { int t = b; b = c; c = t; }
    if (a > b) { int t = a; a = b; b = t; }
    return b;
  }
};
