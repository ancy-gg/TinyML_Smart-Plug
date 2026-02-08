#include "NetworkManager.h"

void NetworkManager::begin(void (*apCallback)(WiFiManager*)) {
    // Enable debug to see exactly what is happening in Serial Monitor
    wm.setDebugOutput(true);

    // Set the callback
    if (apCallback != nullptr) {
        wm.setAPCallback(apCallback);
    }

    // Attempt to connect. 
    // If it fails, it creates "TinyML_Setup" AP.
    // Password is set to NULL (Open Network) for easier connecting.
    if (!wm.autoConnect("TinyML_Setup")) {
        Serial.println("[Net] Failed to connect and hit timeout");
        ESP.restart(); // Restart and try again
    }

    Serial.println("[Net] Connected to WiFi!");
    Serial.print("[Net] IP Address: ");
    Serial.println(WiFi.localIP());
}

void NetworkManager::update() {
    // WiFiManager handles itself usually, but we can add recovery logic here if needed.
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::resetSettings() {
    wm.resetSettings();
}