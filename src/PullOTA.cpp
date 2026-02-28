#include "PullOTA.h"
#include "CloudHandler.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>

static const uint32_t OTA_HTTP_TIMEOUT_MS = 60000;

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

  if (_cb) _cb(OtaEvent::START);
  if (performUpdateFromUrl(fwUrl)) {
    if (_cb) _cb(OtaEvent::SUCCESS);
    delay(300);
    ESP.restart();
  } else {
    if (_cb) _cb(OtaEvent::FAIL);
  }
}

bool PullOTA::fetchOtaTargets(String& desiredVersion, String& firmwareUrl) {
  if (!_cloud) return false;
  if (!_cloud->getString(_pathDesired, desiredVersion)) return false;
  if (!_cloud->getString(_pathUrl, firmwareUrl)) return false;
  return true;
}

bool PullOTA::performUpdateFromUrl(const String& url) {
  WiFiClientSecure client;
  if (_insecureTLS) client.setInsecure();

  HTTPClient http;
  http.setTimeout(OTA_HTTP_TIMEOUT_MS);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  if (!http.begin(client, url)) return false;

  int code = http.GET();
  if (code != HTTP_CODE_OK) { http.end(); return false; }

  int total = http.getSize();

  if (!Update.begin(total > 0 ? (size_t)total : UPDATE_SIZE_UNKNOWN, U_FLASH)) {
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  size_t written = 0;
  uint8_t buf[2048];

  while (http.connected() && (total < 0 || (int)written < total)) {
    size_t avail = stream->available();
    if (!avail) { delay(1); continue; }

    int r = stream->readBytes(buf, (avail > sizeof(buf)) ? sizeof(buf) : avail);
    if (r <= 0) break;

    size_t w = Update.write(buf, (size_t)r);
    if (w != (size_t)r) { Update.abort(); http.end(); return false; }
    written += w;
  }

  if (!Update.end(true)) { http.end(); return false; }
  if (!Update.isFinished()) { http.end(); return false; }

  http.end();
  return true;
}