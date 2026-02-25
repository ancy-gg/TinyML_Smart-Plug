#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"

class VoltageSensor {
public:
  VoltageSensor(int pin);
  void begin();
  void setSensitivity(float factor);

  // Optional: set ADC full-scale volts used for counts->volts conversion.
  // Default 3.3V. You can tune this if your Vrms is scaled oddly.
  void setAdcFullScaleVolts(float vfs);

  // Window length (ms) for a stable Vrms. Default = 200ms.
  void setWindowMs(uint16_t ms);

  // Clip-to-zero hysteresis:
  // If Vrms < v_off => output forced to 0.
  // Output becomes active only when Vrms > v_on.
  void setClampHysteresis(float v_off, float v_on);

  // Returns -1.0 if still sampling window, else returns NEW Vrms value
  float update();

private:
  int _pin;
  float _sensitivity;

  float _adcFullScaleV = 3.3f;

  // Windowing
  uint32_t _windowUs = 200000; // 200ms default

  bool _sampling = false;
  uint32_t _startTime = 0;

  // Robust RMS (variance) accumulators
  uint32_t _count = 0;
  double   _sum   = 0.0;
  double   _sumSq = 0.0;

  // Idle noise learning (so unplugged reads ~0V)
  bool  _idleInit = false;
  float _idleEma  = 0.0f;      // learned noise-floor Vrms (mains volts)
  float _idleLearnMax = 60.0f; // only learn idle when Vrms < this (PH 220V safe)

  // Hysteresis clamp
  bool  _vActive = false;
  float _vOff    = 15.0f;
  float _vOn     = 20.0f;

  static inline int _median3(int a, int b, int c) {
    if (a > b) { int t = a; a = b; b = t; }
    if (b > c) { int t = b; b = c; c = t; }
    if (a > b) { int t = a; a = b; b = t; }
    return b;
  }
};