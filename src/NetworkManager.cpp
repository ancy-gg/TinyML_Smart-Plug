#include "NetworkManager.h"

#define DRD_TIMEOUT 10000 // 10 seconds to clear the "reset flag"
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASS "YOUR_WIFI_PASS"

void NetworkManager::begin() {
    checkDoubleReset();

    if (_isAPMode) {
        setupAP();
    } else {
        // NON-BLOCKING CONNECTION:
        // We call begin() but DO NOT use a while loop to wait.
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        Serial.println("[Net] Attempting WiFi connection in background...");
    }
}

void NetworkManager::checkDoubleReset() {
    preferences.begin("boot_config", false);
    
    // Read the reset counter
    int boots = preferences.getInt("boots", 0);
    
    if (boots >= 1) {
        Serial.println("[Net] Double Reset Detected! Starting AP Mode...");
        _isAPMode = true;
        preferences.putInt("boots", 0); // Clear immediately
    } else {
        // Set flag to 1. We will clear it in update() after 6 seconds.
        Serial.println("[Net] Normal Boot. Waiting to see if user resets...");
        preferences.putInt("boots", 1);
    }
}

void NetworkManager::setupAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("TinyML_Config_AP", "12345678");
    Serial.println("[Net] AP Mode Started. IP: 192.168.4.1");
}

void NetworkManager::update() {
    // 1. Clear the Double Reset Flag after 6 seconds
    static bool flagCleared = false;
    if (!flagCleared && millis() > 6000) {
        preferences.putInt("boots", 0);
        preferences.end();
        flagCleared = true;
        if(!_isAPMode) Serial.println("[Net] Boot flag cleared. Normal operation confirmed.");
    }

    // 2. Optional: Reconnect logic if WiFi drops (every 5 seconds)
    if (!_isAPMode && millis() - _lastWiFiCheck > 5000) {
        _lastWiFiCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.print("."); // Just a heartbeat to show we are trying
            // WiFi.reconnect() is handled automatically by ESP32 usually
        }
    }
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::isAPMode() {
    return _isAPMode;
}