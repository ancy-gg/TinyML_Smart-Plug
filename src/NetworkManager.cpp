#include "NetworkManager.h"
#include <WiFi.h>

NetworkManager* NetworkManager::s_inst = nullptr;

void NetworkManager::apTrampoline(WiFiManager* wmgr) {
    if (!s_inst) return;
    s_inst->_portalActive = true;
    if (s_inst->_userApCb) s_inst->_userApCb(wmgr);
}

void NetworkManager::begin(void (*apCallback)(WiFiManager*)) {
    s_inst = this;
    _userApCb = apCallback;

    // CRITICAL: CS is on RX -> never let WiFiManager print to Serial
    wm.setDebugOutput(false);

    wm.setConfigPortalBlocking(false);
    wm.setAPCallback(NetworkManager::apTrampoline);

    wm.autoConnect("TinyML SmartPlug");
    WiFi.setSleep(false);
}

void NetworkManager::update() {
    wm.process();

    if (_portalActive && WiFi.status() == WL_CONNECTED) {
        _portalActive = false;
    }
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::resetSettings() {
    wm.resetSettings();
}