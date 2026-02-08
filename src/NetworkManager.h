#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

class NetworkManager {
public:
    void begin();
    void update(); // Call this in loop()
    bool isConnected();
    bool isAPMode();

private:
    Preferences preferences;
    bool _isAPMode = false;
    unsigned long _lastWiFiCheck = 0;
    
    void checkDoubleReset();
    void setupAP();
};

#endif