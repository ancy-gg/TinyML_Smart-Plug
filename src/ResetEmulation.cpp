#include "ResetEmulation.h"

void ResetEmulation::write_(bool asserted) {
  if (_pin < 0) return;

  const bool level = _activeHigh ? asserted : !asserted;
  digitalWrite(_pin, level ? HIGH : LOW);
  _active = asserted;
}

void ResetEmulation::begin(int pin, bool activeHigh) {
  _pin = pin;
  _activeHigh = activeHigh;
  pinMode(_pin, OUTPUT);
  release();
}

void ResetEmulation::triggerPulse(uint32_t pulseMs) {
  if (_pin < 0) return;
  if (pulseMs < 20UL) pulseMs = 20UL;

  write_(true);
  _untilMs = millis() + pulseMs;
}

void ResetEmulation::release() {
  write_(false);
  _untilMs = 0;
}

void ResetEmulation::update() {
  if (!_active) return;
  if (_untilMs == 0) return;

  const uint32_t now = millis();
  if ((int32_t)(now - _untilMs) >= 0) {
    release();
  }
}
