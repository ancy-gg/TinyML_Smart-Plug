#include "PowerHoldManager.h"
#include "SmartPlugConfig.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <driver/gpio.h>
#include <esp_sleep.h>
#endif

void PowerHoldManager::begin(int enPin) {
  _pinEn = enPin;
  _bootMs = millis();
  _mainsPresent = false;
  _mainsSeenOnce = false;
  _outageActive = false;
  _presentSinceMs = 0;
  _absentSinceMs = 0;
  _outageStartMs = 0;
  _shutdownTriggered = false;

  if (_pinEn < 0) return;

#if defined(ARDUINO_ARCH_ESP32)
  // Release any previous pin hold after a true power cycle.
  gpio_hold_dis((gpio_num_t)_pinEn);
  gpio_deep_sleep_hold_dis();
#endif

  pinMode(_pinEn, OUTPUT);
  driveEnable(true);
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

  noInterrupts();
  pinMode(_pinEn, OUTPUT);
  digitalWrite(_pinEn, LOW);

#if defined(ARDUINO_ARCH_ESP32)
  // Latch EN LOW through deep sleep so the TPS stays off until the user
  // physically power-cycles the battery rail.
  gpio_hold_dis((gpio_num_t)_pinEn);
  gpio_set_direction((gpio_num_t)_pinEn, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)_pinEn, 0);
  gpio_hold_en((gpio_num_t)_pinEn);
  gpio_deep_sleep_hold_en();

  delay(20);
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_deep_sleep_start();
#endif

  for (;;) {
    delay(1000);
  }
}

void PowerHoldManager::update(float vrmsFast) {
  if (_shutdownTriggered) return;

  const uint32_t now = millis();
  const bool bootArmed = (now - _bootMs) >= OUTAGE_BOOT_ARM_MS;

  const bool mainsNow = _mainsPresent ? (vrmsFast > MAINS_PRESENT_OFF_V)
                                      : (vrmsFast >= MAINS_PRESENT_ON_V);

  if (mainsNow) {
    _absentSinceMs = 0;

    if (!_mainsPresent) {
      if (_presentSinceMs == 0) _presentSinceMs = now;
      if ((now - _presentSinceMs) >= OUTAGE_PRESENT_DEBOUNCE_MS) {
        _mainsPresent = true;
        _mainsSeenOnce = true;
        _outageActive = false;
        _outageStartMs = 0;
      }
    } else {
      _presentSinceMs = now;
      _outageActive = false;
      _outageStartMs = 0;
    }

    driveEnable(true);
    return;
  }

  _presentSinceMs = 0;

  if (_mainsPresent) {
    if (_absentSinceMs == 0) _absentSinceMs = now;
    if ((now - _absentSinceMs) >= OUTAGE_ABSENT_DEBOUNCE_MS) {
      _mainsPresent = false;
      if (bootArmed && _mainsSeenOnce) {
        _outageActive = true;
        _outageStartMs = now;
      }
    }
  } else {
    // Never start the outage timer until the system has seen real mains once.
    if (!_mainsSeenOnce || !bootArmed) {
      driveEnable(true);
      return;
    }

    if (_absentSinceMs == 0) _absentSinceMs = now;
    if (!_outageActive && (now - _absentSinceMs) >= OUTAGE_ABSENT_DEBOUNCE_MS) {
      _outageActive = true;
      _outageStartMs = now;
    }
  }

  driveEnable(true);
  if (_outageActive && (now - _outageStartMs) >= OUTAGE_SHUTDOWN_MS) {
    shutdownNow();
  }
}
