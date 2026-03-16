#pragma once
#include <Arduino.h>

class PowerHoldManager {
public:
  void begin(int enPin);
  void update(float vrmsFast);
  bool outageActive() const { return _outageActive; }
  uint32_t outageElapsedMs() const;
  bool shutdownTriggered() const { return _shutdownTriggered; }

private:
  int _pinEn = -1;
  uint32_t _bootMs = 0;

  bool _mainsPresent = false;
  bool _mainsSeenOnce = false;
  bool _outageActive = false;
  uint32_t _presentSinceMs = 0;
  uint32_t _absentSinceMs = 0;
  uint32_t _outageStartMs = 0;
  bool _shutdownTriggered = false;

  void driveEnable(bool on);
  void shutdownNow();
};
