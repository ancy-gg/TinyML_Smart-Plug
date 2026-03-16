#include "PowerHoldManager.h"
#include "SmartPlugConfig.h"

#include <driver/gpio.h>
#include <esp_sleep.h>

void PowerHoldManager::begin(int enPin) {
  _pinEn = enPin;
  if (_pinEn < 0) return;

  pinMode(_pinEn, OUTPUT);
  driveEnable(true); // keep TPS enabled during normal operation
}

void PowerHoldManager::driveEnable(bool on) {
  if (_pinEn < 0) return;
  digitalWrite(_pinEn, on ? HIGH : LOW);
}

uint32_t PowerHoldManager::outageElapsedMs() const {
  if (!_outageActive) return 0;
  return millis() - _outageStartMs;
}

void PowerHoldManager::shutdownNow() {
  if (_shutdownTriggered || _pinEn < 0) return;
  _shutdownTriggered = true;

  driveEnable(false); // disable TPS / stop battery discharge

#if defined(ARDUINO_ARCH_ESP32)
  gpio_hold_dis((gpio_num_t)_pinEn);
  gpio_hold_en((gpio_num_t)_pinEn);
  gpio_deep_sleep_hold_en();
#endif

  delay(20);
  esp_deep_sleep_start();
}

void PowerHoldManager::update(float vrms) {
  if (_shutdownTriggered) return;

  static bool mainsPresent = false;
  if (mainsPresent) {
    if (vrms <= MAINS_PRESENT_OFF_V) mainsPresent = false;
  } else {
    if (vrms >= MAINS_PRESENT_ON_V) mainsPresent = true;
  }

  if (mainsPresent) {
    _outageActive = false;
    _outageStartMs = 0;
    driveEnable(true);
    return;
  }

  if (!_outageActive) {
    _outageActive = true;
    _outageStartMs = millis();
  }

  driveEnable(true); // stay alive while counting down

  if ((millis() - _outageStartMs) >= OUTAGE_SHUTDOWN_MS) {
    shutdownNow();
  }
}
