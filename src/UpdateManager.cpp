#include "UpdateManager.h"
#include "FirebaseNetwork.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>
#include <esp_attr.h>
#include <esp_app_format.h>
#include <esp_http_client.h>

RTC_DATA_ATTR static uint32_t s_magic = 0;
RTC_DATA_ATTR static uint32_t s_lastAppAddr = 0;
RTC_DATA_ATTR static uint8_t  s_crashBoots = 0;
RTC_DATA_ATTR static uint8_t  s_safeMode = 0;

static constexpr uint32_t MAGIC = 0xC0FFEE42;
static const uint32_t OTA_HTTP_TIMEOUT_MS   = 60000;
static const uint32_t OTA_STREAM_IDLE_MS    = 30000;
static const uint32_t OTA_RESTART_DELAY_MS  = 1200;
static const size_t   OTA_MIN_BIN_BYTES     = 128 * 1024;

static bool isCrashReset(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_PANIC:
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
      return true;
    default:
      return false;
  }
}

static bool getPendingVerifyState_() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) return false;

  esp_ota_img_states_t st;
  if (esp_ota_get_state_partition(running, &st) != ESP_OK) return false;
  return (st == ESP_OTA_IMG_PENDING_VERIFY);
}

String UpdateManager::normalizeFirmwareUrl_(String url) {
  url.trim();
  url.replace("?raw=1", "");
  if (!url.startsWith("https://github.com/")) return url;

  const String prefix = "https://github.com/";
  String tail = url.substring(prefix.length());
  const int blobAt = tail.indexOf("/blob/");
  if (blobAt < 0) return url;

  const String head = tail.substring(0, blobAt);
  const String rest = tail.substring(blobAt + 6);

  String out = "https://raw.githubusercontent.com/";
  out += head;
  out += "/";
  out += rest;
  return out;
}

static inline String otaErr_(const char* msg, int code = 0) {
  String s = msg ? String(msg) : String("OTA_ERROR");
  if (code != 0) {
    s += " (";
    s += String(code);
    s += ")";
  }
  return s;
}

bool UpdateManager::findRollbackPartition_(const esp_partition_t*& out) const {
  out = nullptr;

  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) return false;

  const esp_partition_t* candidate = esp_ota_get_next_update_partition(running);
  if (!candidate || candidate == running) return false;

  esp_app_desc_t desc = {};
  if (esp_ota_get_partition_description(candidate, &desc) != ESP_OK) return false;

  uint8_t magic = 0;
  if (esp_partition_read(candidate, 0, &magic, sizeof(magic)) != ESP_OK) return false;
  if (magic != ESP_IMAGE_HEADER_MAGIC) return false;

  out = candidate;
  return true;
}

bool UpdateManager::rollbackToPrevious() {
  const esp_partition_t* part = nullptr;
  if (!findRollbackPartition_(part)) return false;

  const esp_err_t e = esp_ota_set_boot_partition(part);
  if (e != ESP_OK) return false;

  delay(120);
  ESP.restart();
  return true;
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

  if (s_magic != MAGIC) {
    s_magic = MAGIC;
    s_lastAppAddr = 0;
    s_crashBoots = 0;
    s_safeMode = 0;
  }

  const esp_partition_t* running = esp_ota_get_running_partition();
  const uint32_t runAddr = running ? running->address : 0;

  if (runAddr != 0 && s_lastAppAddr != 0 && runAddr != s_lastAppAddr) {
    s_crashBoots = 0;
    s_safeMode = 0;
  }
  if (runAddr != 0) s_lastAppAddr = runAddr;

  const bool crash = isCrashReset(esp_reset_reason());
  if (crash) {
    if (s_crashBoots < 255) s_crashBoots++;
  } else {
    s_crashBoots = 0;
  }

  _crashBoots = s_crashBoots;
  _pendingVerify = getPendingVerifyState_();

  if (_pendingVerify && crash && s_crashBoots >= _maxCrashBoots) {
    (void)esp_ota_mark_app_invalid_rollback_and_reboot();
  }

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

void UpdateManager::begin(const char* currentVersion, FirebaseNetwork* cloud, uint32_t stableWindowMs, uint8_t maxCrashBoots) {
  _currentVersion = currentVersion ? currentVersion : "TSP-v0.0.0";
  _cloud = cloud;
  _checkNow = true;
  _lastCheckMs = 0;
  _lastError = "";
  bootGuardBegin_(stableWindowMs, maxCrashBoots);
}

void UpdateManager::setPaths(const char* desiredVersionPath, const char* firmwareUrlPath) {
  if (desiredVersionPath && strlen(desiredVersionPath)) _pathDesired = desiredVersionPath;
  if (firmwareUrlPath && strlen(firmwareUrlPath)) _pathUrl = firmwareUrlPath;
}

void UpdateManager::setCheckInterval(uint32_t ms) {
  _intervalMs = ms;
}

void UpdateManager::requestCheckNow() {
  _checkNow = true;
}

void UpdateManager::setInsecureTLS(bool en) {
  _insecureTLS = en;
}

void UpdateManager::loop() {
  bootGuardLoop_();

  // Confirm as early as possible after a successful boot.
  if (!confirmNow()) return;

  if (WiFi.status() != WL_CONNECTED || !_cloud || !_cloud->isReady()) return;

  const uint32_t now = millis();
  if (!_checkNow && (uint32_t)(now - _lastCheckMs) < _intervalMs) return;

  _checkNow = false;
  _lastCheckMs = now;
  _lastError = "";

  String desiredVer, fwUrl;
  if (!fetchOtaTargets(desiredVer, fwUrl)) {
    _lastError = otaErr_("OTA_TARGET_FETCH_FAILED");
    return;
  }

  desiredVer.trim();
  fwUrl.trim();

  if (!desiredVer.length()) {
    _lastError = otaErr_("OTA_EMPTY_VERSION");
    return;
  }
  if (!fwUrl.length()) {
    _lastError = otaErr_("OTA_EMPTY_URL");
    return;
  }
  if (desiredVer.equals(_currentVersion)) {
    _lastError = otaErr_("OTA_SKIP_SAME_VERSION");
    return;
  }

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

bool UpdateManager::performUpdateFromUrl(const String& rawUrl) {
  const String url = normalizeFirmwareUrl_(rawUrl);
  const bool isHttps = url.startsWith("https://");
  const bool isHttp  = url.startsWith("http://");

  if (!isHttps && !isHttp) {
    _lastError = otaErr_("OTA_BAD_URL");
    return false;
  }

  const esp_partition_t* target = esp_ota_get_next_update_partition(nullptr);
  if (!target) {
    _lastError = otaErr_("OTA_NO_TARGET_PARTITION");
    return false;
  }

  HTTPClient http;
  http.setTimeout(OTA_HTTP_TIMEOUT_MS);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.useHTTP10(true);
  http.setReuse(false);
  http.addHeader("Cache-Control", "no-cache");

  static const char* hdrKeys[] = {"Content-Type", "Content-Length", "Location"};
  http.collectHeaders(hdrKeys, 3);

  bool begun = false;
  WiFiClient plainClient;
  WiFiClientSecure secureClient;

  if (isHttps) {
    secureClient.setTimeout(OTA_HTTP_TIMEOUT_MS);
    if (_insecureTLS) secureClient.setInsecure();
    begun = http.begin(secureClient, url);
  } else {
    plainClient.setTimeout(OTA_HTTP_TIMEOUT_MS);
    begun = http.begin(plainClient, url);
  }

  if (!begun) {
    _lastError = otaErr_("OTA_HTTP_BEGIN_FAILED");
    return false;
  }

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    _lastError = otaErr_("OTA_HTTP_GET_FAILED", code);
    http.end();
    return false;
  }

  const String ctype = http.header("Content-Type");
  if (ctype.startsWith("text/") || ctype.indexOf("html") >= 0 || ctype.indexOf("json") >= 0) {
    _lastError = otaErr_("OTA_BAD_CONTENT_TYPE");
    http.end();
    return false;
  }

  const int total = http.getSize();
  if (total > 0) {
    if ((size_t)total < OTA_MIN_BIN_BYTES) {
      _lastError = otaErr_("OTA_IMAGE_TOO_SMALL", total);
      http.end();
      return false;
    }
    if ((size_t)total > target->size) {
      _lastError = otaErr_("OTA_IMAGE_TOO_LARGE", total);
      http.end();
      return false;
    }
  }

  if (!Update.begin(total > 0 ? (size_t)total : UPDATE_SIZE_UNKNOWN, U_FLASH)) {
    _lastError = otaErr_("OTA_BEGIN_FAILED", Update.getError());
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  size_t written = 0;
  uint8_t buf[4096];
  uint32_t lastDataMs = millis();
  int lastPct = -1;
  bool firstChunk = true;

  while (http.connected() && (total < 0 || (int)written < total)) {
    const size_t avail = stream->available();
    if (!avail) {
      if ((millis() - lastDataMs) > OTA_STREAM_IDLE_MS) {
        _lastError = otaErr_("OTA_STREAM_IDLE_TIMEOUT");
        Update.abort();
        http.end();
        return false;
      }
      delay(1);
      continue;
    }

    const int r = stream->readBytes(buf, (avail > sizeof(buf)) ? sizeof(buf) : avail);
    if (r <= 0) {
      if ((millis() - lastDataMs) > OTA_STREAM_IDLE_MS) {
        _lastError = otaErr_("OTA_STREAM_READ_TIMEOUT");
        Update.abort();
        http.end();
        return false;
      }
      delay(1);
      continue;
    }

    if (firstChunk) {
      firstChunk = false;
      if (buf[0] != ESP_IMAGE_HEADER_MAGIC) {
        _lastError = otaErr_("OTA_BAD_IMAGE_HEADER", buf[0]);
        Update.abort();
        http.end();
        return false;
      }
    }

    lastDataMs = millis();

    const size_t w = Update.write(buf, (size_t)r);
    if (w != (size_t)r) {
      _lastError = otaErr_("OTA_WRITE_FAILED", Update.getError());
      Update.abort();
      http.end();
      return false;
    }

    written += w;

    if (total > 0 && _cb) {
      const int pct = (int)((100.0f * (float)written) / (float)total);
      if (pct != lastPct) {
        lastPct = pct;
        _cb(OtaEvent::PROGRESS, pct);
      }
    }
  }

  if (written < OTA_MIN_BIN_BYTES) {
    _lastError = otaErr_("OTA_WRITTEN_TOO_SMALL", (int)written);
    Update.abort();
    http.end();
    return false;
  }

  if (!Update.end(true) || !Update.isFinished()) {
    _lastError = otaErr_("OTA_END_FAILED", Update.getError());
    http.end();
    return false;
  }

  http.end();
  _lastError = "";
  return true;
}