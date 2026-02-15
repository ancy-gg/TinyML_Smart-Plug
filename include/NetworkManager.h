#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFiManager.h> 

class NetworkManager {
public:
    void begin(void (*apCallback)(WiFiManager*));
    void update(); 
    bool isConnected();
    void resetSettings(); 

private:
    WiFiManager wm;
};

#endif