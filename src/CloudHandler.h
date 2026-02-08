#ifndef CLOUD_HANDLER_H
#define CLOUD_HANDLER_H

#include <Firebase_ESP_Client.h>

class CloudHandler {
public:
    void begin(const char* apiKey, const char* dbUrl);
    void update(float v, float c, float t, float zcv, float thd, float entropy, String state); // Handles timing internally

private:
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;
    unsigned long _lastSend = 0;
    const unsigned long _interval = 3000; // 3 seconds
};

#endif