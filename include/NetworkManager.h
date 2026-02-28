#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFiManager.h>

class NetworkManager {
public:
    void begin(void (*apCallback)(WiFiManager*));
    void update();
    bool isConnected();
    bool inConfigPortal() const { return _portalActive; }
    void resetSettings();

private:
    WiFiManager wm;
    volatile bool _portalActive = false;

    void (*_userApCb)(WiFiManager*) = nullptr;

    static NetworkManager* s_inst;
    static void apTrampoline(WiFiManager* wmgr);
};

#endif