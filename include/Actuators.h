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
  SND_MAINS_RESTORED,
  SND_VOLT_LOW,
  SND_VOLT_HIGH,
  SND_DEVICE_PLUG,
  SND_FAULT_ARC,
  SND_FAULT_HEAT,
  SND_FAULT_OVER,
  SND_FAULT_UNDERVOLT,
  SND_FAULT_OVERVOLT,
  SND_RESET_ACK
};

class Actuators {
public:
  void begin(int pinLatchOn, int pinLatchOff, int pinBuzzer, OLED_NOTIF* oled);
  void apply(FaultState st, float vDisplay, float vProtect, float i, float t);
  void notify(SoundEvent ev);

  void pulseRelayOn(uint32_t pulseMs = 0);
  void pulseRelayOff(uint32_t pulseMs = 0);

  bool relayLatchedOn() const { return _relayLatchedOn; }
  bool consumeRelayOnEdge();
  bool consumeRelayOffEdge();

private:
  int _pinLatchOn = -1;
  int _pinLatchOff = -1;
  int _pinBuzzer = -1;
  OLED_NOTIF* _oled = nullptr;

  bool _relayLatchedOn = false;
  uint32_t _pulseOnUntil = 0;
  uint32_t _pulseOffUntil = 0;
  uint32_t _lastRelayPulseMs = 0;
  bool _relayOnEdge = false;
  bool _relayOffEdge = false;

  uint32_t _loadDetectSince = 0;
  uint32_t _unpluggedSince = 0;

  void writeLatchOn_(bool asserted);
  void writeLatchOff_(bool asserted);
  void updateRelayPulse_();
};
