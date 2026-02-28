#include "NetworkManager.h"
#include <WiFi.h>

NetworkManager* NetworkManager::s_inst = nullptr;

void NetworkManager::apTrampoline(WiFiManager* wmgr) {
    if (s_inst) {
        s_inst->_portalActive = true;
        if (s_inst->_userApCb) s_inst->_userApCb(wmgr);
    }
}

void NetworkManager::begin(void (*apCallback)(WiFiManager*)) {
    s_inst = this;
    _userApCb = apCallback;

    // CRITICAL (your CS is on RX): do not enable serial debug output.
    wm.setDebugOutput(false);

    // Keep device running while portal is up
    wm.setConfigPortalBlocking(false);

    // Track portal state
    wm.setAPCallback(NetworkManager::apTrampoline);

    wm.autoConnect("TinyML Smart Plug");

    WiFi.setSleep(false);
}

void NetworkManager::update() {
    wm.process();

    // Clear portal flag once connected
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