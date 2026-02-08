#ifndef CLOUD_HANDLER_H
#define CLOUD_HANDLER_H

#include <Firebase_ESP_Client.h>

class TimeSync; // forward decl

class CloudHandler {
public:
    void begin(const char* apiKey, const char* dbUrl);

    // Pass TimeSync so cloud packets can include real wall-clock time.
    void update(float v, float c, float t, float zcv, float thd, float entropy,
                const String& state, TimeSync* time);

private:
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;
    unsigned long _lastSend = 0;
    const unsigned long _interval = 5000; // ms
};

#endif