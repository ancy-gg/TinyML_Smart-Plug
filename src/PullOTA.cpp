#include "PullOTA.h"
#include "CloudHandler.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <esp_ota_ops.h>

static const uint32_t OTA_HTTP_TIMEOUT_MS = 60000;
static const uint32_t OTA_STREAM_IDLE_MS  = 12000;
static const uint32_t OTA_RESTART_DELAY_MS = 1200;

static bool confirmRunningImageIfPending() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) return true;

  esp_ota_img_states_t st;
  if (esp_ota_get_state_partition(running, &st) != ESP_OK) return true;
  if (st != ESP_OTA_IMG_PENDING_VERIFY) return true;

  return esp_ota_mark_app_valid_cancel_rollback() == ESP_OK;
}

void PullOTA::begin(const char* currentVersion, CloudHandler* cloud) {
  _currentVersion = currentVersion ? currentVersion : "TSP-v0.0.0";
  _cloud = cloud;
  _checkNow = true;
  _lastCheckMs = 0;
}

void PullOTA::setPaths(const char* desiredVersionPath, const char* firmwareUrlPath) {
  if (desiredVersionPath && strlen(desiredVersionPath)) _pathDesired = desiredVersionPath;
  if (firmwareUrlPath && strlen(firmwareUrlPath)) _pathUrl = firmwareUrlPath;
}

void PullOTA::setCheckInterval(uint32_t ms) { _intervalMs = ms; }
void PullOTA::requestCheckNow() { _checkNow = true; }
void PullOTA::setInsecureTLS(bool en) { _insecureTLS = en; }

void PullOTA::loop() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!_cloud || !_cloud->isReady()) return;

  // ESP-IDF blocks a new OTA while the current image is still pending verify.
  // Confirm it here as soon as the app is healthy enough to reach the OTA loop.
  if (!confirmRunningImageIfPending()) return;

  const uint32_t now = millis();
  if (!_checkNow && (uint32_t)(now - _lastCheckMs) < _intervalMs) return;

  _checkNow = false;
  _lastCheckMs = now;

  String desiredVer, fwUrl;
  if (!fetchOtaTargets(desiredVer, fwUrl)) return;
  desiredVer.trim();
  fwUrl.trim();

  if (!desiredVer.length() || !fwUrl.length()) return;
  if (desiredVer.equals(_currentVersion)) return;

  if (_cb) _cb(OtaEvent::START, 0);
  if (performUpdateFromUrl(fwUrl)) {
    if (_cb) _cb(OtaEvent::SUCCESS, 100);
    delay(OTA_RESTART_DELAY_MS);
    ESP.restart();
  } else {
    if (_cb) _cb(OtaEvent::FAIL, 0);
  }
}

bool PullOTA::fetchOtaTargets(String& desiredVersion, String& firmwareUrl) {
  if (!_cloud) return false;
  if (!_cloud->getString(_pathDesired, desiredVersion)) return false;
  if (!_cloud->getString(_pathUrl, firmwareUrl)) return false;
  return true;
}

bool PullOTA::performUpdateFromUrl(const String& url) {
  const bool isHttps = url.startsWith("https://");
  const bool isHttp  = url.startsWith("http://");
  if (!isHttps && !isHttp) return false;

  HTTPClient http;
  http.setTimeout(OTA_HTTP_TIMEOUT_MS);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.useHTTP10(true);

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
  if (!begun) return false;

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    http.end();
    return false;
  }

  const int total = http.getSize();
  if (!Update.begin(total > 0 ? (size_t)total : UPDATE_SIZE_UNKNOWN, U_FLASH)) {
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  size_t written = 0;
  uint8_t buf[2048];
  uint32_t lastDataMs = millis();
  int lastPct = -1;

  while (http.connected() && (total < 0 || (int)written < total)) {
    const size_t avail = stream->available();
    if (!avail) {
      if ((millis() - lastDataMs) > OTA_STREAM_IDLE_MS) {
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
        Update.abort();
        http.end();
        return false;
      }
      delay(1);
      continue;
    }

    lastDataMs = millis();
    const size_t w = Update.write(buf, (size_t)r);
    if (w != (size_t)r) {
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

  if (!Update.end(true)) {
    http.end();
    return false;
  }
  if (!Update.isFinished()) {
    http.end();
    return false;
  }

  http.end();
  return true;
}
