#include "WifiHandler.h"
#include "MainConfiguration.h"
#include <esp_wifi.h>

WifiHandler* WifiHandler::s_inst = nullptr;

bool WifiHandler::hasSavedCredentials_() const {
  wifi_config_t cfg = {};
  if (esp_wifi_get_config(WIFI_IF_STA, &cfg) == ESP_OK) return cfg.sta.ssid[0] != 0;
  String ssid = WiFi.SSID();
  ssid.trim();
  return ssid.length() > 0;
}

uint8_t WifiHandler::apStations_() const {
#if defined(ARDUINO_ARCH_ESP32)
  if ((((int)WiFi.getMode()) & (int)WIFI_AP) == 0) return 0;
  return (uint8_t)WiFi.softAPgetStationNum();
#else
  return 0;
#endif
}

void WifiHandler::startStaConnect_() {
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.softAPdisconnect(true);
  delay(30);
  WiFi.mode(WIFI_STA);
  delay(30);
  WiFi.begin();
  _portalRequested = false;
  _portalDisconnectSta = false;
  _portalStartMs = 0;
  _portalTimeoutMs = 0;
  _apWindowUntilMs = 0;
  _lastApStations = 0;
  _phase = PHASE_CONNECTING;
  _phaseStartMs = millis();
}

void WifiHandler::startApWait_(bool disconnectSta, bool manualRequest, uint32_t windowMs) {
  _portalRequested = manualRequest;
  _portalDisconnectSta = disconnectSta || manualRequest;
  _portalStartMs = 0;
  _portalTimeoutMs = manualRequest ? WIFI_MANUAL_PORTAL_TIMEOUT_MS : WIFI_PORTAL_TIMEOUT_MS;
  _lastApStations = 0;
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(!_portalDisconnectSta);
  if (_portalDisconnectSta || WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true, false);
    delay(40);
    WiFi.mode(WIFI_AP);
  } else {
    WiFi.mode(WIFI_AP_STA);
  }
  delay(60);
  WiFi.softAPdisconnect(true);
  delay(20);
  WiFi.softAP(WIFI_PORTAL_SSID);
  delay(60);
  _lastApStations = apStations_();
  const uint32_t openMs = (windowMs > 0UL) ? windowMs : (manualRequest ? WIFI_MANUAL_AP_WINDOW_MS : WIFI_PORTAL_TIMEOUT_MS);
  _apWindowUntilMs = millis() + openMs;
  _phase = PHASE_AP_WAIT_CLIENT;
  _phaseStartMs = millis();
}

void WifiHandler::startPortal_() {
  wm.setDebugOutput(false);
  wm.setConfigPortalBlocking(false);
  wm.setBreakAfterConfig(true);
  wm.setConfigPortalTimeout((int)(_portalTimeoutMs / 1000UL));
  _phase = PHASE_PORTAL_ACTIVE;
  _phaseStartMs = millis();
  _portalStartMs = _phaseStartMs;
  wm.startConfigPortal(WIFI_PORTAL_SSID);
}

void WifiHandler::closeApAndRecover_(uint32_t now) {
  WiFi.softAPdisconnect(true);
  delay(30);
  WiFi.mode(WIFI_STA);
  delay(30);
  _portalRequested = false;
  _portalDisconnectSta = false;
  _portalStartMs = 0;
  _portalTimeoutMs = 0;
  _apWindowUntilMs = 0;
  _lastApStations = 0;
  if (WiFi.status() == WL_CONNECTED) { _phase = PHASE_CONNECTED; _phaseStartMs = now; return; }
  if (hasSavedCredentials_()) { WiFi.begin(); _phase = PHASE_CONNECTING; _phaseStartMs = now; return; }
  _phase = PHASE_TIMEOUT;
  _phaseStartMs = now;
}

void WifiHandler::requestPortal(bool disconnectSta) { (void)disconnectSta; startApWait_(true, true, WIFI_MANUAL_AP_WINDOW_MS); }

void WifiHandler::apTrampoline(WiFiManager* wmgr) {
  if (!s_inst) return;
  s_inst->_phase = PHASE_PORTAL_ACTIVE;
  s_inst->_portalStartMs = millis();
  if (s_inst->_userApCb) s_inst->_userApCb(wmgr);
}

void WifiHandler::begin(void (*apCallback)(WiFiManager*)) {
  s_inst = this;
  _userApCb = apCallback;
  wm.setDebugOutput(false);
  wm.setConfigPortalBlocking(false);
  wm.setBreakAfterConfig(true);
  wm.setAPCallback(WifiHandler::apTrampoline);

  WiFi.persistent(true);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);
  delay(60);
  _phase = PHASE_BOOT_CONNECT;
  _phaseStartMs = millis();
  if (hasSavedCredentials_()) startStaConnect_();
  else startApWait_(true, false, WIFI_BOOT_NO_CRED_AP_WINDOW_MS);
}

void WifiHandler::update() {
  const uint32_t now = millis();
  if (_phase == PHASE_PORTAL_ACTIVE) {
    wm.process();
    if (WiFi.status() == WL_CONNECTED) { closeApAndRecover_(now); return; }
    if (_portalStartMs && (now - _portalStartMs) >= _portalTimeoutMs) { closeApAndRecover_(now); return; }
    return;
  }

  if (_phase == PHASE_AP_WAIT_CLIENT) {
    const uint8_t stations = apStations_();
    if (stations > 0 || stations > _lastApStations) { startPortal_(); return; }
    _lastApStations = stations;
    if ((int32_t)(_apWindowUntilMs - now) <= 0) { closeApAndRecover_(now); return; }
    return;
  }

  if (_phase == PHASE_CONNECTING || _phase == PHASE_BOOT_CONNECT) {
    if (WiFi.status() == WL_CONNECTED) { _phase = PHASE_CONNECTED; _phaseStartMs = now; return; }
    if ((now - _phaseStartMs) >= WIFI_CONNECT_TIMEOUT_MS) {
      _phase = PHASE_TIMEOUT;
      _phaseStartMs = now;
      startApWait_(true, false);
      return;
    }
    return;
  }

  if (_phase == PHASE_CONNECTED) {
    if (WiFi.status() == WL_CONNECTED) return;
    if (hasSavedCredentials_()) startStaConnect_();
    else startApWait_(true, false);
    return;
  }

  if (_phase == PHASE_TIMEOUT) {
    if (!hasSavedCredentials_()) {
      if ((now - _phaseStartMs) >= WIFI_BACKGROUND_RETRY_MS) startApWait_(true, false, WIFI_BOOT_NO_CRED_AP_WINDOW_MS);
      return;
    }
    if ((now - _phaseStartMs) >= WIFI_BACKGROUND_RETRY_MS) startStaConnect_();
  }
}

bool WifiHandler::isConnected() const { return WiFi.status() == WL_CONNECTED; }
int WifiHandler::rssi() const { return WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : -127; }
const char* WifiHandler::phaseText() const {
  switch (_phase) {
    case PHASE_BOOT_CONNECT: return "BOOT CONNECT";
    case PHASE_CONNECTING: return "CONNECTING";
    case PHASE_CONNECTED: return "CONNECTED";
    case PHASE_TIMEOUT: return "TIMEOUT";
    case PHASE_AP_WAIT_CLIENT: return "AP WAIT";
    case PHASE_PORTAL_ACTIVE: return "PORTAL";
    default: return "UNKNOWN";
  }
}
