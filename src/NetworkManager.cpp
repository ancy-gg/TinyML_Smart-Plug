#include "NetworkManager.h"

void NetworkManager::begin(void (*apCallback)(WiFiManager*)) {
    // 1. Setup the callback (so we can update OLED when AP opens)
    if (apCallback != nullptr) {
        wm.setAPCallback(apCallback);
    }

    // 2. Optional: Remove debug clutter
    wm.setDebugOutput(true); 

    // 3. THE MAGIC LINE
    // It tries to connect to saved WiFi. 
    // If it fails, it opens an AP named "TinyML_Setup".
    // It BLOCKS here until the user configures it.
    if (!wm.autoConnect("TinyML WiFi")) {
        Serial.println("failed to connect and hit timeout");
        // If we get here, we timed out or failed. Restart.
        ESP.restart();
    }

    // 4. If we get here, we are connected!
    Serial.println("connected...yeey :)");
}

void NetworkManager::update() {
    // WiFiManager handles reconnections automatically in the background.
    // We don't need to do anything here!
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::resetSettings() {
    wm.resetSettings();
}