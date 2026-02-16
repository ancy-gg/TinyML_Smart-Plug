#include "NetworkManager.h"
#include <WiFi.h>

void NetworkManager::begin(void (*apCallback)(WiFiManager*)) {
    wm.setDebugOutput(true);

    if (apCallback != nullptr) {
        wm.setAPCallback(apCallback);
    }

    // IMPORTANT: Do NOT restart on failure.
    // Also make it NON-BLOCKING so your device can continue running offline.
    wm.setConfigPortalBlocking(false);

    // Try to connect or start portal. In non-blocking mode this often returns quickly.
    wm.autoConnect("TinyML_Setup");

    // Optional: reduce random disconnect issues
    WiFi.setSleep(false);
}

void NetworkManager::update() {
    // REQUIRED when config portal is non-blocking
    wm.process();
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::resetSettings() {
    wm.resetSettings();
}