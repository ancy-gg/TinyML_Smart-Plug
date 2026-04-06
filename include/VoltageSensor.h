#pragma once
#include <Arduino.h>
#include "MainConfiguration.h"

class VoltageSensor {
public:
  explicit VoltageSensor(int pin);
  void begin();
  void setSensitivity(float factor);
  void setLinearCalib(float slope, float intercept);
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
  uint32_t _windowUs = 500000;
  bool _sampling = false;
  uint32_t _startTime = 0;
  uint32_t _nextSampleUs = 0;
  uint32_t _count = 0;
  double   _sum   = 0.0;
  double   _sumSq = 0.0;
  int      _sampleMin = 4095;
  int      _sampleMax = 0;
  bool  _vActive = false;
  float _vOff    = 12.0f;
  float _vOn     = 24.0f;
  float _rawVrms = 0.0f;
  bool  _protInit = false;
  float _protVrms = 0.0f;
  bool  _dispInit = false;
  float _dispVrms = 0.0f;
  float _avgTauS  = 2.6f;
  float _avgJumpV = 18.0f;
  uint8_t _lowWindows = 0;
  uint8_t _noSignalWindows = 0;
  uint8_t _faultVoteCount = 0;
  uint8_t _healthyVoteCount = 0;

  static constexpr uint8_t WINDOW_RING = 5;
  static constexpr uint16_t SAMPLE_INTERVAL_US = 250;
  static constexpr uint8_t MAX_FILTERED_SAMPLES_PER_CALL = 32;
  float _winVrms[WINDOW_RING] = {0};
  uint8_t _winValid[WINDOW_RING] = {0};
  uint8_t _winPos = 0;
  uint8_t _winCount = 0;

  static inline int _median5(int a, int b, int c, int d, int e) {
    int v[5] = {a, b, c, d, e};
    for (int i = 0; i < 4; ++i) {
      for (int j = i + 1; j < 5; ++j) {
        if (v[j] < v[i]) { int t = v[i]; v[i] = v[j]; v[j] = t; }
      }
    }
    return v[2];
  }
};
