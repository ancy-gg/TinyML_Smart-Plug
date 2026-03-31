#include "UpdateManager.h"
#include "FirebaseHandler.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>
#include <esp_attr.h>

RTC_DATA_ATTR static uint32_t s_magic = 0;
RTC_DATA_ATTR static uint32_t s_lastAppAddr = 0;
RTC_DATA_ATTR static uint8_t  s_crashBoots = 0;
RTC_DATA_ATTR static uint8_t  s_safeMode = 0;
static constexpr uint32_t MAGIC = 0xC0FFEE42;
static const uint32_t OTA_HTTP_TIMEOUT_MS = 60000;
static const uint32_t OTA_STREAM_IDLE_MS  = 12000;
static const uint32_t OTA_RESTART_DELAY_MS = 1200;

static bool isCrashReset(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_PANIC:
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT: return true;
    default: return false;
  }
}

static bool getPendingVerifyState_() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) return false;
  esp_ota_img_states_t st;
  if (esp_ota_get_state_partition(running, &st) != ESP_OK) return false;
  return (st == ESP_OTA_IMG_PENDING_VERIFY);
}

bool UpdateManager::confirmNow() {
  if (!_pendingVerify) return true;
  const esp_err_t e = esp_ota_mark_app_valid_cancel_rollback();
  if (e == ESP_OK) {
    _pendingVerify = false;
    _crashBoots = 0;
    s_crashBoots = 0;
    return true;
  }
  return false;
}

void UpdateManager::bootGuardBegin_(uint32_t stableWindowMs, uint8_t maxCrashBoots) {
  _stableWindowMs = (stableWindowMs < 5000) ? 5000 : stableWindowMs;
  _maxCrashBoots  = (maxCrashBoots < 1) ? 1 : maxCrashBoots;
  _bootMs0 = millis();

  if (s_magic != MAGIC) { s_magic = MAGIC; s_lastAppAddr = 0; s_crashBoots = 0; s_safeMode = 0; }
  const esp_partition_t* running = esp_ota_get_running_partition();
  const uint32_t runAddr = running ? running->address : 0;
  if (runAddr != 0 && s_lastAppAddr != 0 && runAddr != s_lastAppAddr) { s_crashBoots = 0; s_safeMode = 0; }
  if (runAddr != 0) s_lastAppAddr = runAddr;

  const bool crash = isCrashReset(esp_reset_reason());
  if (crash) { if (s_crashBoots < 255) s_crashBoots++; } else s_crashBoots = 0;
  _crashBoots = s_crashBoots;
  _pendingVerify = getPendingVerifyState_();
  if (_pendingVerify && crash && s_crashBoots >= _maxCrashBoots) (void)esp_ota_mark_app_invalid_rollback_and_reboot();
  if (crash && s_crashBoots >= _maxCrashBoots) s_safeMode = 1;
  _safeMode = (s_safeMode != 0);
}

void UpdateManager::bootGuardLoop_() {
  if (_safeMode) return;
  const uint32_t up = millis() - _bootMs0;
  if (up < _stableWindowMs) return;
  s_crashBoots = 0;
  _crashBoots = 0;
  if (_pendingVerify) (void)confirmNow();
}

void UpdateManager::begin(const char* currentVersion, FirebaseHandler* cloud, uint32_t stableWindowMs, uint8_t maxCrashBoots) {
  _currentVersion = currentVersion ? currentVersion : "TSP-v0.0.0";
  _cloud = cloud;
  _checkNow = true;
  _lastCheckMs = 0;
  bootGuardBegin_(stableWindowMs, maxCrashBoots);
}
void UpdateManager::setPaths(const char* desiredVersionPath, const char* firmwareUrlPath) {
  if (desiredVersionPath && strlen(desiredVersionPath)) _pathDesired = desiredVersionPath;
  if (firmwareUrlPath && strlen(firmwareUrlPath)) _pathUrl = firmwareUrlPath;
}
void UpdateManager::setCheckInterval(uint32_t ms) { _intervalMs = ms; }
void UpdateManager::requestCheckNow() { _checkNow = true; }
void UpdateManager::setInsecureTLS(bool en) { _insecureTLS = en; }

void UpdateManager::loop() {
  bootGuardLoop_();
  if (WiFi.status() != WL_CONNECTED || !_cloud || !_cloud->isReady()) return;
  if (!confirmNow()) return;
  const uint32_t now = millis();
  if (!_checkNow && (uint32_t)(now - _lastCheckMs) < _intervalMs) return;
  _checkNow = false; _lastCheckMs = now;

  String desiredVer, fwUrl;
  if (!fetchOtaTargets(desiredVer, fwUrl)) return;
  desiredVer.trim(); fwUrl.trim();
  if (!desiredVer.length() || !fwUrl.length() || desiredVer.equals(_currentVersion)) return;

  if (_cb) _cb(OtaEvent::START, 0);
  if (performUpdateFromUrl(fwUrl)) {
    if (_cb) _cb(OtaEvent::SUCCESS, 100);
    delay(OTA_RESTART_DELAY_MS);
    ESP.restart();
  } else {
    if (_cb) _cb(OtaEvent::FAIL, 0);
  }
}

bool UpdateManager::fetchOtaTargets(String& desiredVersion, String& firmwareUrl) {
  if (!_cloud) return false;
  if (!_cloud->getString(_pathDesired, desiredVersion)) return false;
  if (!_cloud->getString(_pathUrl, firmwareUrl)) return false;
  return true;
}

bool UpdateManager::performUpdateFromUrl(const String& url) {
  const bool isHttps = url.startsWith("https://");
  const bool isHttp  = url.startsWith("http://");
  if (!isHttps && !isHttp) return false;
  HTTPClient http;
  http.setTimeout(OTA_HTTP_TIMEOUT_MS);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.useHTTP10(true);

  bool begun = false;
  WiFiClient plainClient; WiFiClientSecure secureClient;
  if (isHttps) { secureClient.setTimeout(OTA_HTTP_TIMEOUT_MS); if (_insecureTLS) secureClient.setInsecure(); begun = http.begin(secureClient, url); }
  else { plainClient.setTimeout(OTA_HTTP_TIMEOUT_MS); begun = http.begin(plainClient, url); }
  if (!begun) return false;

  const int code = http.GET();
  if (code != HTTP_CODE_OK) { http.end(); return false; }
  const int total = http.getSize();
  if (!Update.begin(total > 0 ? (size_t)total : UPDATE_SIZE_UNKNOWN, U_FLASH)) { http.end(); return false; }

  WiFiClient* stream = http.getStreamPtr();
  size_t written = 0; uint8_t buf[2048]; uint32_t lastDataMs = millis(); int lastPct = -1;
  while (http.connected() && (total < 0 || (int)written < total)) {
    const size_t avail = stream->available();
    if (!avail) {
      if ((millis() - lastDataMs) > OTA_STREAM_IDLE_MS) { Update.abort(); http.end(); return false; }
      delay(1); continue;
    }
    const int r = stream->readBytes(buf, (avail > sizeof(buf)) ? sizeof(buf) : avail);
    if (r <= 0) {
      if ((millis() - lastDataMs) > OTA_STREAM_IDLE_MS) { Update.abort(); http.end(); return false; }
      delay(1); continue;
    }
    lastDataMs = millis();
    const size_t w = Update.write(buf, (size_t)r);
    if (w != (size_t)r) { Update.abort(); http.end(); return false; }
    written += w;
    if (total > 0 && _cb) {
      const int pct = (int)((100.0f * (float)written) / (float)total);
      if (pct != lastPct) { lastPct = pct; _cb(OtaEvent::PROGRESS, pct); }
    }
  }
  if (!Update.end(true) || !Update.isFinished()) { http.end(); return false; }
  http.end();
  return true;
}
