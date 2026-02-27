#ifndef CLOUD_HANDLER_H
#define CLOUD_HANDLER_H

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include "TimeSync.h"

class CloudHandler {
public:
  void begin(const char* apiKey, const char* dbUrl);

  // Optional: show real running FW version in PWA
  void setFirmwareVersion(const char* fw);

  // Tune live send behavior (normal state throttling)
  void setNormalIntervalMs(uint32_t ms);  // default 6000ms
  void setFaultIntervalMs(uint32_t ms);   // default 1500ms

  // Live update payload
  void update(float v, float c, float t,
              float zcv, float thd, float entropy,
              float hf_ratio, float hf_var,
              uint8_t model_pred, int arc_cnt,
              const String& state, TimeSync* time);

  // For OTA + configs
  bool isReady() const;
  bool getString(const char* path, String& out);
  bool getBool(const char* path, bool& out);
  bool getInt(const char* path, int& out);

  // For ML logger upload
  bool pushJSON(const char* path, FirebaseJson& json);

private:
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;

  unsigned long _lastLiveSend = 0;
  uint32_t _normalIntervalMs = 6000; // makes dashboard feel responsive
  uint32_t _faultIntervalMs  = 1500; // only matters if you call update faster

  String _lastSentLiveState = "";
  String _lastLoggedFaultState = "";

  String _fwVersion = "—";
};

#endif