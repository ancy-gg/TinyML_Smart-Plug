#include "FanController.h"
#include "SmartPlugConfig.h"
#include <math.h>

static constexpr uint8_t FAN_PWM_CH = 1;

void FanController::begin(int pwmPin) {
  _pin = pwmPin;
  pinMode(_pin, OUTPUT);

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(_pin, FAN_PWM_HZ, FAN_PWM_BITS);
  ledcWrite(_pin, 0);
#else
  ledcSetup(FAN_PWM_CH, FAN_PWM_HZ, FAN_PWM_BITS);
  ledcAttachPin(_pin, FAN_PWM_CH);
  ledcWrite(FAN_PWM_CH, 0);
#endif

  _lastDuty = 0;
  _kickUntilMs = 0;
}

uint8_t FanController::dutyFromTemp_(float tempC) const {
  if (!(tempC > -40.0f && tempC < 150.0f)) return 0;
  if (tempC <= FAN_MIN_TEMP_C) return 0;
  if (tempC >= FAN_MAX_TEMP_C) return 255;

  const float x = (tempC - FAN_MIN_TEMP_C) / (FAN_MAX_TEMP_C - FAN_MIN_TEMP_C);
  uint8_t duty = (uint8_t)lroundf(x * 255.0f);
  if (duty > 0 && duty < FAN_MIN_SPIN_DUTY) duty = FAN_MIN_SPIN_DUTY;
  return duty;
}

void FanController::writeDuty_(uint8_t duty) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(_pin, duty);
#else
  ledcWrite(FAN_PWM_CH, duty);
#endif
  _lastDuty = duty;
}

void FanController::update(float tempC) {
  if (_pin < 0) return;

  uint8_t target = dutyFromTemp_(tempC);
  const uint32_t now = millis();

  if (target > 0 && _lastDuty == 0) {
    _kickUntilMs = now + 300UL;
    writeDuty_(255);
    return;
  }

  if (_kickUntilMs && (int32_t)(_kickUntilMs - now) > 0) return;
  _kickUntilMs = 0;

  if (target != _lastDuty) writeDuty_(target);
}