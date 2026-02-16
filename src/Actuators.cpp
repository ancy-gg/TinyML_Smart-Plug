#include "Actuators.h"
#include "OLED_NOTIF.h"

static constexpr uint32_t OLED_MS = 250;
static constexpr uint32_t BUZZ_ON_MS  = 140;
static constexpr uint32_t BUZZ_OFF_MS = 110;

void Actuators::begin(int pinRelay, int pinBuzzer, int pinResetBtn, OLED_NOTIF* oled) {
  _pinRelay = pinRelay;
  _pinBuzzer = pinBuzzer;
  _pinReset = pinResetBtn;
  _oled = oled;

  pinMode(_pinRelay, OUTPUT);
  digitalWrite(_pinRelay, LOW);

  pinMode(_pinBuzzer, OUTPUT);
  digitalWrite(_pinBuzzer, LOW);

  pinMode(_pinReset, INPUT_PULLUP);
}

void Actuators::setRelay(bool on) {
  digitalWrite(_pinRelay, on ? HIGH : LOW);
}

void Actuators::buzzerTone(bool on, uint16_t freqHz) {
  static bool inited = false;
  static const int ch = 0;
  static const int res = 8;

  if (!inited) {
    ledcSetup(ch, 2400, res);
    ledcAttachPin(_pinBuzzer, ch);
    inited = true;
  }

  if (!on) {
    ledcWrite(ch, 0);
    _buzzOn = false;
    return;
  }

  ledcWriteTone(ch, freqHz);
  ledcWrite(ch, 128); // 50% duty
  _buzzOn = true;
}

void Actuators::buzzerUpdate(bool enabled) {
  if (!enabled) {
    buzzerTone(false, 2400);
    _buzzT0 = 0;
    return;
  }

  const uint32_t now = millis();
  if (_buzzT0 == 0) _buzzT0 = now;

  const uint32_t dt = now - _buzzT0;

  if (_buzzOn) {
    if (dt >= BUZZ_ON_MS) {
      buzzerTone(false, 2400);
      _buzzT0 = now;
    }
  } else {
    if (dt >= BUZZ_OFF_MS) {
      buzzerTone(true, 2400);
      _buzzT0 = now;
    }
  }
}

void Actuators::apply(FaultState st, float v, float i, float t) {
  const bool relayOn = (st == STATE_NORMAL || st == STATE_OVERLOAD);
  setRelay(relayOn);

  buzzerUpdate(st != STATE_NORMAL);

  if (_oled && (millis() - _lastOled) >= OLED_MS) {
    _lastOled = millis();
    _oled->updateDashboard(v, i, t, st);
  }
}

bool Actuators::resetLongPressed() {
  const bool down = (digitalRead(_pinReset) == LOW);
  const uint32_t now = millis();

  if (down) {
    if (_btnDown == 0) _btnDown = now;
    if (!_btnHeld && (now - _btnDown) > 1200) {
      _btnHeld = true;
      return true;
    }
  } else {
    _btnDown = 0;
    _btnHeld = false;
  }
  return false;
}