#include "NetworkManager.h"

void NetworkManager::begin(void (*apCallback)(WiFiManager*)) {
    wm.setDebugOutput(true);

    // Set the callback
    if (apCallback != nullptr) {
        wm.setAPCallback(apCallback);
    }

    // Attempt to connect. If it fails, it will start the config portal and block until configuration is done.
    if (!wm.autoConnect("TinyML_Setup")) {
        ESP.restart(); // Restart and try again
    }
}

void NetworkManager::update() {}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::resetSettings() {
    wm.resetSettings();
}