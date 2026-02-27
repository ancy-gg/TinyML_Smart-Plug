#ifndef CLOUD_HANDLER_H
#define CLOUD_HANDLER_H

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include "TimeSync.h"

class CloudHandler {
public:
  void begin(const char* apiKey, const char* dbUrl);

  void setFirmwareVersion(const char* fw);
  void setNormalIntervalMs(uint32_t ms);
  void setFaultIntervalMs(uint32_t ms);

  void update(float v, float c, float t,
              float zcv, float thd, float entropy,
              float hf_ratio, float hf_var,
              float sf, float cyc_var,
              uint8_t model_pred,
              const String& state, TimeSync* time);

  bool isReady() const;
  bool getString(const char* path, String& out);
  bool getBool(const char* path, bool& out);
  bool getInt(const char* path, int& out);

  bool pushJSON(const char* path, FirebaseJson& json);

private:
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;

  unsigned long _lastLiveSend = 0;
  uint32_t _normalIntervalMs = 6000;
  uint32_t _faultIntervalMs  = 1500;

  String _lastSentLiveState = "";
  String _lastLoggedFaultState = "";
  String _fwVersion = "—";
};

#endif