#include "NetworkManager.h"
#include "SmartPlugConfig.h"

RTC_DATA_ATTR static uint8_t s_bootTapCount = 0;
RTC_DATA_ATTR static uint8_t s_bootTapArmed = 0;

NetworkManager* NetworkManager::s_inst = nullptr;

void NetworkManager::startPortal_() {
  WiFi.mode(WIFI_AP_STA);
  _portalStarted = false;
  wm.startConfigPortal("TinyML-SmartPlug");
  _phase = PHASE_PORTAL_ACTIVE;
  _portalStarted = true;
  _portalStartMs = millis();
}

void NetworkManager::apTrampoline(WiFiManager* wmgr) {
  if (!s_inst) return;
  s_inst->_phase = PHASE_PORTAL_ACTIVE;
  s_inst->_portalStarted = true;
  s_inst->_portalStartMs = millis();
  if (s_inst->_userApCb) s_inst->_userApCb(wmgr);
}

void NetworkManager::begin(void (*apCallback)(WiFiManager*)) {
  s_inst = this;
  _userApCb = apCallback;
  _phaseStartMs = millis();

  if (s_bootTapArmed) {
    if (s_bootTapCount < 255) s_bootTapCount++;
  } else {
    s_bootTapCount = 1;
  }
  s_bootTapArmed = 1;
  _portalRequested = (s_bootTapCount >= WIFI_TRIPLE_TAP_COUNT);

  wm.setDebugOutput(false);
  wm.setConfigPortalBlocking(false);
  wm.setBreakAfterConfig(true);
  wm.setAPCallback(NetworkManager::apTrampoline);

  WiFi.persistent(true);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  if (_portalRequested) {
    startPortal_();
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin();
  _phase = PHASE_CONNECTING;
}

void NetworkManager::update() {
  const uint32_t now = millis();

  if (!_tapCleared && now >= WIFI_BOOT_BLOCK_MS) {
    s_bootTapCount = 0;
    s_bootTapArmed = 0;
    _tapCleared = true;
  }

  wm.process();

  if (_phase == PHASE_CONNECTING) {
    if (WiFi.status() == WL_CONNECTED) {
      _phase = PHASE_CONNECTED;
      return;
    }
    if ((now - _phaseStartMs) >= WIFI_CONNECT_TIMEOUT_MS) {
      _phase = PHASE_TIMEOUT;
      _phaseStartMs = now;
      return;
    }
    return;
  }

  if (_phase == PHASE_TIMEOUT) {
    if (WiFi.status() == WL_CONNECTED) {
      _phase = PHASE_CONNECTED;
      return;
    }
    if ((now - _phaseStartMs) >= 15000UL) {
      WiFi.mode(WIFI_STA);
      WiFi.begin();
      _phase = PHASE_CONNECTING;
      _phaseStartMs = now;
    }
    return;
  }

  if (_phase == PHASE_PORTAL_ACTIVE) {
    if (WiFi.status() == WL_CONNECTED) {
      _phase = PHASE_CONNECTED;
      s_bootTapCount = 0;
      s_bootTapArmed = 0;
      _tapCleared = true;
      return;
    }

    if (_portalStartMs && (now - _portalStartMs) >= WIFI_PORTAL_TIMEOUT_MS) {
      _phase = PHASE_TIMEOUT;
      _phaseStartMs = now;
      WiFi.softAPdisconnect(true);
      return;
    }
  }
}

bool NetworkManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::isBlockingPhase() const {
  return (_phase == PHASE_CONNECTING) || (_phase == PHASE_PORTAL_ACTIVE);
}

void NetworkManager::resetSettings() {
  wm.resetSettings();
}

int NetworkManager::rssi() const {
  return (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : -127;
}

const char* NetworkManager::phaseText() const {
  switch (_phase) {
    case PHASE_CONNECTING:    return "CONNECTING";
    case PHASE_CONNECTED:     return "CONNECTED";
    case PHASE_TIMEOUT:       return "TIMEOUT";
    case PHASE_AP_WAIT_CLIENT:return "AP_WAIT";
    case PHASE_PORTAL_ACTIVE: return "PORTAL";
    default:                  return "BOOT";
  }
}
