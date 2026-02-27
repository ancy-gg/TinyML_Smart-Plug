#pragma once
#include <Arduino.h>
#include "SmartPlugTypes.h"

class OLED_NOTIF;

enum SoundEvent : uint8_t {
  SND_BOOT = 0,
  SND_WIFI_PORTAL,
  SND_WIFI_OK,
  SND_LOGGER_ON,
  SND_OTA_START,
  SND_OTA_OK,
  SND_OTA_FAIL,
  SND_MAINS_LOST,
  SND_DEVICE_PLUG,
  SND_FAULT_ARC,
  SND_FAULT_HEAT,
  SND_FAULT_OVER,
  SND_RESET_ACK
};

class Actuators {
public:
  void begin(int pinRelay, int pinBuzzer, int pinResetBtn, OLED_NOTIF* oled);
  void apply(FaultState st, float v, float i, float t);
  bool resetLongPressed(); // long press clears latch
  void notify(SoundEvent ev);
  
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