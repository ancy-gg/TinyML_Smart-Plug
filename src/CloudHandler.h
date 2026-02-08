#ifndef CLOUD_HANDLER_H
#define CLOUD_HANDLER_H

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include "TimeSync.h"

class CloudHandler {
public:
  void begin(const char* apiKey, const char* dbUrl);

  // live_data: NORMAL -> every 10s, faults -> only on state change
  // history: faults only, only when state changes into a fault (or between faults)
  void update(float v, float c, float t, float zcv, float thd, float entropy,
              const String& state, TimeSync* time);

  // --- OTA support helpers ---
  bool getString(const char* path, String& out);
  bool isReady() const { return Firebase.ready(); }

private:
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;

  unsigned long _lastLiveSend = 0;
  const unsigned long _normalIntervalMs = 10000;

  String _lastSentLiveState = "";
  String _lastLoggedFaultState = "";
};

#endif