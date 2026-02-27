#ifndef PULL_OTA_H
#define PULL_OTA_H

#include <Arduino.h>

class CloudHandler;

enum class OtaEvent : uint8_t { START, SUCCESS, FAIL };
using OtaEventCb = void (*)(OtaEvent);

class PullOTA {
public:
  void begin(const char* currentVersion, CloudHandler* cloud);

  void loop();

  void setPaths(const char* desiredVersionPath, const char* firmwareUrlPath);
  void setCheckInterval(uint32_t ms);
  void requestCheckNow();
  void setInsecureTLS(bool en);

  // NEW
  void setEventCallback(OtaEventCb cb) { _cb = cb; }

private:
  bool fetchOtaTargets(String& desiredVersion, String& firmwareUrl);
  bool performUpdateFromUrl(const String& url);

  const char* _currentVersion = "TSP-v0.0.0";
  CloudHandler* _cloud = nullptr;

  const char* _pathDesired = "/ota/desired_version";
  const char* _pathUrl     = "/ota/firmware_url";

  uint32_t _intervalMs = 60000;
  uint32_t _lastCheckMs = 0;
  bool _checkNow = true;

  bool _insecureTLS = true;

  OtaEventCb _cb = nullptr;
};

#endif