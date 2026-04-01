#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

class WifiHandler {
public:
  enum Phase : uint8_t {
    PHASE_BOOT_CONNECT = 0,
    PHASE_CONNECTING,
    PHASE_CONNECTED,
    PHASE_TIMEOUT,
    PHASE_AP_WAIT_CLIENT,
    PHASE_PORTAL_ACTIVE
  };

  void begin(void (*apCallback)(WiFiManager*));
  void update();
  void requestPortal(bool disconnectSta = false);
  bool isConnected() const;
  bool inConfigPortal() const { return _phase == PHASE_PORTAL_ACTIVE; }
  bool isBlockingPhase() const { return _phase == PHASE_PORTAL_ACTIVE; }
  int  rssi() const;
  Phase phase() const { return _phase; }
  const char* phaseText() const;

private:
  WiFiManager wm;
  Phase _phase = PHASE_BOOT_CONNECT;
  volatile bool _portalRequested = false;
  bool _portalDisconnectSta = false;
  uint32_t _phaseStartMs = 0;
  uint32_t _portalStartMs = 0;
  uint32_t _portalTimeoutMs = 0;
  uint32_t _apWindowUntilMs = 0;
  uint8_t  _lastApStations = 0;
  void (*_userApCb)(WiFiManager*) = nullptr;

  void startStaConnect_();
  void startApWait_(bool disconnectSta, bool manualRequest, uint32_t windowMs = 0);
  void startPortal_();
  void closeApAndRecover_(uint32_t now);
  uint8_t apStations_() const;
  bool hasSavedCredentials_() const;

  static WifiHandler* s_inst;
  static void apTrampoline(WiFiManager* wmgr);
};

#endif
