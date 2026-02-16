#pragma once
#include <Arduino.h>
#include "SmartPlugTypes.h"

class OLED_NOTIF;

class Actuators {
public:
  void begin(int pinRelay, int pinBuzzer, int pinResetBtn, OLED_NOTIF* oled);
  void apply(FaultState st, float v, float i, float t);
  bool resetLongPressed(); // long press clears latch

private:
  int _pinRelay = -1;
  int _pinBuzzer = -1;
  int _pinReset = -1;
  OLED_NOTIF* _oled = nullptr;

  uint32_t _buzzT0 = 0;
  bool _buzzOn = false;

  uint32_t _lastOled = 0;

  uint32_t _btnDown = 0;
  bool _btnHeld = false;

  void setRelay(bool on);
  void buzzerUpdate(bool enabled);
  void buzzerTone(bool on, uint16_t freqHz);
};