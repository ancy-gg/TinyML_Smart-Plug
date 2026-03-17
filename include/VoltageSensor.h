#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

class VoltageSensor {
public:
  explicit VoltageSensor(int pin);
  void begin();
  void setSensitivity(float factor);
  void setLinearCalib(float slope, float intercept); // legacy helper -> maps to cubic1/cubic0
  void setCubicCalib(float c3, float c2, float c1, float c0);
  void setAdcFullScaleVolts(float vfs);
  void setWindowMs(uint16_t ms);
  void setClampHysteresis(float v_off, float v_on);
  void setLongAverage(float tauS, float jumpV);

  float update();
  float rawVrms() const     { return _rawVrms; }
  float protectVrms() const { return _protVrms; }

private:
  int _pin;
  VoltageCalib _cal;
  float _adcFullScaleV = 3.3f;

  uint32_t _windowUs = 200000;
  bool _sampling = false;
  uint32_t _startTime = 0;

  uint32_t _count = 0;
  double   _sum   = 0.0;
  double   _sumSq = 0.0;

  bool  _vActive = false;
  float _vOff    = 15.0f;
  float _vOn     = 25.0f;

  float _rawVrms  = 0.0f;
  bool  _protInit = false;
  float _protVrms = 0.0f;

  bool  _dispInit = false;
  float _dispVrms = 0.0f;
  float _avgTauS  = 18.0f;
  float _avgJumpV = 18.0f;

  uint8_t _lowWindows = 0;

  static inline int _median3(int a, int b, int c) {
    if (a > b) { int t = a; a = b; b = t; }
    if (b > c) { int t = b; b = c; c = t; }
    if (a > b) { int t = a; a = b; b = t; }
    return b;
  }
};
