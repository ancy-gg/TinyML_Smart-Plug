#ifndef UPDATE_MANAGER_H
#define UPDATE_MANAGER_H

#include <Arduino.h>
#include <esp_partition.h>

class FirebaseNetwork;

enum class OtaEvent : uint8_t { START, PROGRESS, SUCCESS, FAIL };
using OtaEventCb = void (*)(OtaEvent, int progress);

class UpdateManager {
public:
  void begin(const char* currentVersion, FirebaseNetwork* cloud, uint32_t stableWindowMs = 12000, uint8_t maxCrashBoots = 3);
  void loop();

  void setPaths(const char* desiredVersionPath, const char* firmwareUrlPath);
  void setCheckInterval(uint32_t ms);
  void requestCheckNow();
  void setInsecureTLS(bool en);
  void setEventCallback(OtaEventCb cb) { _cb = cb; }

  bool safeMode() const { return _safeMode; }
  const String& lastError() const { return _lastError; }
  bool pendingVerify() const { return _pendingVerify; }
  bool running() const { return _taskRunning; }
  bool confirmNow();
  bool rollbackToPrevious();

private:
  bool fetchOtaTargets(String& desiredVersion, String& firmwareUrl);
  bool performUpdateFromUrl(const String& url);
  bool findRollbackPartition_(const esp_partition_t*& out) const;
  static String normalizeFirmwareUrl_(String url);
  void bootGuardBegin_(uint32_t stableWindowMs, uint8_t maxCrashBoots);
  void bootGuardLoop_();
  bool startOtaTask_(const String& desiredVersion, const String& firmwareUrl);
  void otaTask_();
  static void otaTaskThunk_(void* arg);

  const char* _currentVersion = "TSP-v0.0.0";
  FirebaseNetwork* _cloud = nullptr;
  const char* _pathDesired = "/ota/desired_version";
  const char* _pathUrl     = "/ota/firmware_url";
  uint32_t _intervalMs = 0;
  uint32_t _lastCheckMs = 0;
  bool _checkNow = false;
  bool _insecureTLS = true;
  OtaEventCb _cb = nullptr;

  uint32_t _stableWindowMs = 12000;
  uint8_t  _maxCrashBoots  = 3;
  uint32_t _bootMs0 = 0;
  bool _pendingVerify = false;
  bool _safeMode = false;
  uint8_t _crashBoots = 0;
  bool _taskRunning = false;
  String _pendingVersion = "";
  String _pendingUrl = "";
  String _lastError = "";
};

#endif
