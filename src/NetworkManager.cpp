#include "NetworkManager.h"
#include "SmartPlugConfig.h"
#include <esp_wifi.h>

static constexpr uint32_t WIFI_BOOT_AP_DISCOVERY_WINDOW_MS   = WIFI_BOOT_BLOCK_MS;
static constexpr uint32_t WIFI_MANUAL_AP_DISCOVERY_WINDOW_MS = 20000UL;
static constexpr uint32_t WIFI_BACKGROUND_RETRY_MS           = 60000UL;

NetworkManager* NetworkManager::s_inst = nullptr;

namespace {
  static bool hasSavedCredentials_() {
    wifi_config_t cfg = {};
    if (esp_wifi_get_config(WIFI_IF_STA, &cfg) == ESP_OK) {
      return cfg.sta.ssid[0] != 0;
    }

    String ssid = WiFi.SSID();
    ssid.trim();
    return ssid.length() > 0;
  }
}

uint8_t NetworkManager::apStations_() const {
#if defined(ARDUINO_ARCH_ESP32)
  if ((((int)WiFi.getMode()) & (int)WIFI_AP) == 0) return 0;
  return (uint8_t)WiFi.softAPgetStationNum();
#else
  return 0;
#endif
}

void NetworkManager::startStaConnect_() {
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  WiFi.softAPdisconnect(true);
  delay(40);
  WiFi.mode(WIFI_STA);
  delay(40);
  WiFi.begin();

  _portalRequested = false;
  _portalDisconnectSta = false;
  _portalStartMs = 0;
  _apWindowUntilMs = 0;
  _lastApStations = 0;

  _phase = PHASE_CONNECTING;
  _phaseStartMs = millis();
}

void NetworkManager::startApWait_(bool disconnectSta, bool manualRequest) {
  _portalRequested = manualRequest;
  _portalDisconnectSta = disconnectSta;
  _portalStartMs = 0;
  _lastApStations = 0;

  WiFi.setSleep(false);
  WiFi.setAutoReconnect(!disconnectSta);

  if (disconnectSta || WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true, false);
    delay(60);
    WiFi.mode(WIFI_AP);
    delay(80);
  } else {
    WiFi.mode(WIFI_AP_STA);
    delay(80);
  }

  WiFi.softAPdisconnect(true);
  delay(30);
  WiFi.softAP(WIFI_PORTAL_SSID);
  delay(80);

  _lastApStations = apStations_();
  _apWindowUntilMs = millis() + (manualRequest ? WIFI_MANUAL_AP_DISCOVERY_WINDOW_MS
                                               : WIFI_BOOT_AP_DISCOVERY_WINDOW_MS);
  _phase = PHASE_AP_WAIT_CLIENT;
  _phaseStartMs = millis();
}

void NetworkManager::startPortal_() {
  wm.setDebugOutput(false);
  wm.setConfigPortalBlocking(false);
  wm.setBreakAfterConfig(true);
  wm.setConfigPortalTimeout((int)(WIFI_PORTAL_TIMEOUT_MS / 1000UL));

  _phase = PHASE_PORTAL_ACTIVE;
  _phaseStartMs = millis();
  _portalStartMs = _phaseStartMs;
  wm.startConfigPortal(WIFI_PORTAL_SSID);
}

void NetworkManager::closeApAndRecover_(uint32_t now) {
  const bool wasPortal = (_phase == PHASE_PORTAL_ACTIVE);
  const bool hadSaved = hasSavedCredentials_();

  WiFi.softAPdisconnect(true);
  delay(40);
  WiFi.mode(WIFI_STA);
  delay(40);

  _portalRequested = false;
  _portalDisconnectSta = false;
  _portalStartMs = 0;
  _apWindowUntilMs = 0;
  _lastApStations = 0;

  if (WiFi.status() == WL_CONNECTED) {
    _phase = PHASE_CONNECTED;
    _phaseStartMs = now;
    return;
  }

  // Deterministic recovery: if the user entered the portal but left without
  // getting the STA link up, reboot the device so boot flow becomes:
  // AP first -> optional portal -> saved STA connect.
  if (wasPortal) {
    delay(120);
    ESP.restart();
    return;
  }

  if (hadSaved) {
    WiFi.begin();
    _phase = PHASE_CONNECTING;
    _phaseStartMs = now;
    return;
  }

  // No saved credentials yet: keep the hotspot available instead of going
  // into a disconnected idle state.
  startApWait_(true, false);
}

void NetworkManager::requestPortal(bool disconnectSta) {
  startApWait_(disconnectSta, true);
}

void NetworkManager::apTrampoline(WiFiManager* wmgr) {
  if (!s_inst) return;
  s_inst->_phase = PHASE_PORTAL_ACTIVE;
  s_inst->_portalStartMs = millis();
  if (s_inst->_userApCb) s_inst->_userApCb(wmgr);
}

void NetworkManager::begin(void (*apCallback)(WiFiManager*)) {
  s_inst = this;
  _userApCb = apCallback;

  _phase = PHASE_BOOT_BLOCK;
  _phaseStartMs = millis();
  _portalRequested = false;
  _portalDisconnectSta = false;
  _portalStartMs = 0;
  _apWindowUntilMs = 0;
  _lastRetryMs = 0;
  _lastApStations = 0;

  wm.setDebugOutput(false);
  wm.setConfigPortalBlocking(false);
  wm.setBreakAfterConfig(true);
  wm.setAPCallback(NetworkManager::apTrampoline);

  WiFi.persistent(true);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);
  delay(60);

  // Deterministic boot rule:
  // always open the AP first for a short discovery window, then either
  // connect to saved Wi-Fi or keep the AP running if there are no creds yet.
  startApWait_(true, false);
}

void NetworkManager::update() {
  const uint32_t now = millis();

  if (_phase == PHASE_PORTAL_ACTIVE) {
    wm.process();

    if (WiFi.status() == WL_CONNECTED) {
      closeApAndRecover_(now);
      return;
    }

    if (_portalStartMs && (now - _portalStartMs) >= WIFI_PORTAL_TIMEOUT_MS) {
      closeApAndRecover_(now);
      return;
    }
    return;
  }

  if (_phase == PHASE_AP_WAIT_CLIENT) {
    const uint8_t stations = apStations_();
    if (stations > 0 || stations > _lastApStations) {
      startPortal_();
      return;
    }
    _lastApStations = stations;

    if ((int32_t)(_apWindowUntilMs - now) <= 0) {
      closeApAndRecover_(now);
      return;
    }
    return;
  }

  if (_phase == PHASE_CONNECTING) {
    if (WiFi.status() == WL_CONNECTED) {
      _phase = PHASE_CONNECTED;
      _phaseStartMs = now;
      return;
    }

    if ((now - _phaseStartMs) >= WIFI_CONNECT_TIMEOUT_MS) {
#if WIFI_OPEN_PORTAL_ON_TIMEOUT
      startApWait_(true, false);
#else
      _phase = PHASE_TIMEOUT;
      _phaseStartMs = now;
      _lastRetryMs = now;
#endif
      return;
    }
    return;
  }

  if (_phase == PHASE_CONNECTED) {
    if (WiFi.status() == WL_CONNECTED) return;

    if (hasSavedCredentials_()) {
      startStaConnect_();
    } else {
      startApWait_(true, false);
    }
    return;
  }

  if (_phase == PHASE_TIMEOUT || _phase == PHASE_BOOT_BLOCK) {
    if (!hasSavedCredentials_()) {
      if ((now - _lastRetryMs) >= WIFI_BACKGROUND_RETRY_MS) {
        _lastRetryMs = now;
        startApWait_(true, false);
      }
      return;
    }

    if ((now - _phaseStartMs) >= WIFI_BACKGROUND_RETRY_MS) {
      startStaConnect_();
    }
    return;
  }
}

bool NetworkManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::isBlockingPhase() const {
  return (_phase == PHASE_PORTAL_ACTIVE);
}

void NetworkManager::resetSettings() {
  wm.resetSettings();
}

int NetworkManager::rssi() const {
  return (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : -127;
}

const char* NetworkManager::phaseText() const {
  switch (_phase) {
    case PHASE_CONNECTING:     return "CONNECTING";
    case PHASE_CONNECTED:      return "CONNECTED";
    case PHASE_TIMEOUT:        return "DISCONNECTED";
    case PHASE_AP_WAIT_CLIENT: return "AP_WAIT";
    case PHASE_PORTAL_ACTIVE:  return "PORTAL";
    default:                   return "BOOT";
  }
}
