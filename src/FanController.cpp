#include "FanController.h"

void FanController::begin(int pwmPin) {
  _pin = pwmPin;
  pinMode(_pin, OUTPUT);

  // 2-wire brushless fan: avoid PWM entirely.
  // Drive it with a steady HIGH so it runs at full speed.
  digitalWrite(_pin, HIGH);
  _lastDuty = 255;
}

void FanController::update(float tempC) {
  (void)tempC;

  if (_pin < 0) return;

  // Keep fan forced ON at 100% all the time.
  digitalWrite(_pin, HIGH);
  _lastDuty = 255;
}