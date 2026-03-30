#pragma once
#include <Arduino.h>

class ResetEmulation {
public:
  void begin(int pin, bool activeHigh = true);
  void triggerPulse(uint32_t pulseMs);
  void release();
  void update();

  bool active() const { return _active; }

private:
  int _pin = -1;
  bool _activeHigh = true;
  bool _active = false;
  uint32_t _untilMs = 0;

  void write_(bool asserted);
};
