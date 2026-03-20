#include "NetworkManager.h"
#include "SmartPlugConfig.h"
#include <Preferences.h>

static constexpr const char* NVS_NS_WIFI  = "wifi_boot";
static constexpr const char* NVS_KEY_ARM  = "armed";
static constexpr const char* NVS_KEY_CNT  = "count";

static void loadBootWindow(bool& armed, uint8_t& count) {
  Preferences prefs;
  prefs.begin(NVS_NS_WIFI, false);
  armed = prefs.getBool(NVS_KEY_ARM, false);
  count = prefs.getUChar(NVS_KEY_CNT, 0);
  prefs.end();
}

static void saveBootWindow(bool armed, uint8_t count) {
  Preferences prefs;
  prefs.begin(NVS_NS_WIFI, false);
  prefs.putBool(NVS_KEY_ARM, armed);
  prefs.putUChar(NVS_KEY_CNT, count);
  prefs.end();
}

NetworkManager* NetworkManager::s_inst = nullptr;

void NetworkManager::clearBootWindow_() {
  saveBootWindow(false, 0);
  _bootWindowCleared = true;
}

void NetworkManager::startPortal_() {
  WiFi.disconnect(false, false);
  delay(80);
  WiFi.mode(WIFI_AP_STA);
  delay(120);

  _portalStarted = false;
  wm.setConfigPortalTimeout((int)(WIFI_PORTAL_TIMEOUT_MS / 1000UL));
  wm.startConfigPortal("TinyML-SmartPlug");

  _phase = PHASE_PORTAL_ACTIVE;
  _phaseStartMs = millis();
  _portalStarted = true;
  _portalStartMs = _phaseStartMs;
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
  _portalStarted = false;
  _portalStartMs = 0;
  _bootWindowCleared = false;

  bool armed = false;
  uint8_t count = 0;
  loadBootWindow(armed, count);
  count = armed ? (uint8_t)((count < 255) ? (count + 1) : 255) : 1;
  saveBootWindow(true, count);
  _portalRequested = (count >= WIFI_TRIPLE_TAP_COUNT);

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
  delay(50);
  WiFi.begin();
  _phase = PHASE_CONNECTING;
}

void NetworkManager::update() {
  const uint32_t now = millis();

  if (!_bootWindowCleared && now >= WIFI_BOOT_BLOCK_MS) {
    clearBootWindow_();
  }

  wm.process();

  if (_phase == PHASE_PORTAL_ACTIVE) {
    const bool apMode = (((int)WiFi.getMode() & (int)WIFI_AP) != 0);
    const bool apRunning = apMode && (WiFi.softAPIP() != IPAddress((uint32_t)0));

    if (_portalStartMs && (now - _portalStartMs) >= WIFI_PORTAL_TIMEOUT_MS) {
      WiFi.softAPdisconnect(true);
      _portalStarted = false;
      _portalStartMs = 0;

      if (WiFi.status() == WL_CONNECTED) {
        _phase = PHASE_CONNECTED;
        _phaseStartMs = now;
        clearBootWindow_();
      } else {
        WiFi.mode(WIFI_STA);
        delay(50);
        WiFi.begin();
        _phase = PHASE_CONNECTING;
        _phaseStartMs = now;
      }
      return;
    }

    if (!apRunning) {
      _portalStarted = false;
      _portalStartMs = 0;

      if (WiFi.status() == WL_CONNECTED) {
        _phase = PHASE_CONNECTED;
        _phaseStartMs = now;
        clearBootWindow_();
      } else {
        WiFi.mode(WIFI_STA);
        delay(50);
        WiFi.begin();
        _phase = PHASE_CONNECTING;
        _phaseStartMs = now;
      }
      return;
    }

    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (_phase != PHASE_CONNECTED) _phaseStartMs = now;
    _phase = PHASE_CONNECTED;
    if (!_bootWindowCleared) clearBootWindow_();
    return;
  }

  if (_phase == PHASE_CONNECTING) {
    if ((now - _phaseStartMs) >= WIFI_CONNECT_TIMEOUT_MS) {
      _phase = PHASE_TIMEOUT;
      _phaseStartMs = now;
    }
    return;
  }

  if (_phase == PHASE_TIMEOUT || _phase == PHASE_CONNECTED) {
    if ((now - _phaseStartMs) >= 15000UL) {
      WiFi.mode(WIFI_STA);
      delay(50);
      WiFi.begin();
      _phase = PHASE_CONNECTING;
      _phaseStartMs = now;
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
    case PHASE_CONNECTING:    return "CONNECTING";
    case PHASE_CONNECTED:     return "CONNECTED";
    case PHASE_TIMEOUT:       return "DISCONNECTED";
    case PHASE_AP_WAIT_CLIENT:return "AP_WAIT";
    case PHASE_PORTAL_ACTIVE: return "PORTAL";
    default:                  return "BOOT";
  }
}