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
  void requestPortal(bool disconnectSta = true);
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
  bool _portalDisconnectSta = false;
  bool _autoApWindowOffered = false;

  uint32_t _phaseStartMs = 0;
  uint32_t _portalStartMs = 0;
  uint32_t _apWindowUntilMs = 0;
  uint8_t  _lastApStations = 0;

  void (*_userApCb)(WiFiManager*) = nullptr;

  void startStaConnect_();
  void startApWait_(bool disconnectSta, bool manualRequest);
  void startPortal_();
  void closeApAndReconnect_(uint32_t now);
  uint8_t apStations_() const;

  static NetworkManager* s_inst;
  static void apTrampoline(WiFiManager* wmgr);
};

#endif
