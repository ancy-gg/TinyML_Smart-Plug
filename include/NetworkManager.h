#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

class NetworkManager {
public:
  enum Phase : uint8_t {
    PHASE_BOOT_BLOCK = 0,
    PHASE_CONNECTING,
    PHASE_CONNECTED,
    PHASE_TIMEOUT,
    PHASE_AP_WAIT_CLIENT,
    PHASE_PORTAL_ACTIVE
  };

  void begin(void (*apCallback)(WiFiManager*));
  void update();
  bool isConnected() const;
  bool inConfigPortal() const { return _phase == PHASE_PORTAL_ACTIVE; }
  bool isBlockingPhase() const;
  bool portalRequested() const { return _portalRequested; }
  void resetSettings();
  int  rssi() const;
  Phase phase() const { return _phase; }
  const char* phaseText() const;

private:
  WiFiManager wm;
  Phase _phase = PHASE_BOOT_BLOCK;
  volatile bool _portalRequested = false;
  bool _portalStarted = false;
  bool _bootWindowCleared = false;
  uint32_t _phaseStartMs = 0;
  uint32_t _portalStartMs = 0;
  void startPortal_();
  void clearBootWindow_();
  void (*_userApCb)(WiFiManager*) = nullptr;

  static NetworkManager* s_inst;
  static void apTrampoline(WiFiManager* wmgr);
};

#endif
