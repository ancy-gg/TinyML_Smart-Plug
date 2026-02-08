#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFiManager.h> // The magic library

class NetworkManager {
public:
    void begin(void (*apCallback)(WiFiManager*)); // We pass a function to update OLED
    void update(); 
    bool isConnected();
    void resetSettings(); // Helper to clear WiFi if needed

private:
    WiFiManager wm;
};

#endif