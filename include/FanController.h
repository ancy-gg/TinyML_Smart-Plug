#pragma once
#include <Arduino.h>

class FanController {
public:
  void begin(int pwmPin);
  void update(float tempC);
  uint8_t duty() const { return _lastDuty; }

private:
  int _pin = -1;
  uint8_t _lastDuty = 0;
};