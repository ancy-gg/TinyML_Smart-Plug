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

void PullOTA::setCheckInterval(uint32_t ms) {
  _intervalMs = ms;
}

void PullOTA::requestCheckNow() {
  _checkNow = true;
}

void PullOTA::setInsecureTLS(bool en) {
  _insecureTLS = en;
}

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

  if (!desiredVer.length() || !fwUrl.length()) {
    Serial.println("[PullOTA] Missing desired_version or firmware_url.");
    return;
  }

  if (desiredVer.equals(_currentVersion)) {
    Serial.printf("[PullOTA] Up-to-date (%s)\n", _currentVersion);
    return;
  }

  Serial.printf("[PullOTA] Update available: %s -> %s\n", _currentVersion, desiredVer.c_str());
  Serial.printf("[PullOTA] URL: %s\n", fwUrl.c_str());

  if (performUpdateFromUrl(fwUrl)) {
    Serial.println("[PullOTA] Update success. Rebooting...");
    delay(300);
    ESP.restart();
  } else {
    Serial.println("[PullOTA] Update failed.");
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
  if (_insecureTLS) client.setInsecure();   // most reliable for GitHub raw
  // else: you would need to set a correct root CA cert here

  HTTPClient http;
  http.setTimeout(OTA_HTTP_TIMEOUT_MS);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  Serial.println("[PullOTA] HTTP GET firmware...");
  if (!http.begin(client, url)) {
    Serial.println("[PullOTA] http.begin failed");
    return false;
  }

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("[PullOTA] HTTP code %d\n", code);
    http.end();
    return false;
  }

  int total = http.getSize(); // can be -1 (unknown)
  if (total > 0) Serial.printf("[PullOTA] Firmware size: %d bytes\n", total);
  else Serial.println("[PullOTA] Unknown content length (still ok).");

  // IMPORTANT: requires OTA partition scheme (2 app slots). If this fails, fix partitions.
  if (!Update.begin(total > 0 ? (size_t)total : UPDATE_SIZE_UNKNOWN, U_FLASH)) {
    Serial.printf("[PullOTA] Update.begin failed. Err=%u\n", Update.getError());
    Serial.println("[PullOTA] If Err indicates no partition, you MUST use an OTA partition table.");
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  size_t written = 0;
  uint8_t buf[2048];
  uint32_t lastPrint = millis();

  while (http.connected() && (total < 0 || (int)written < total)) {
    size_t avail = stream->available();
    if (!avail) {
      delay(1);
      continue;
    }

    int r = stream->readBytes(buf, (avail > sizeof(buf)) ? sizeof(buf) : avail);
    if (r <= 0) break;

    size_t w = Update.write(buf, (size_t)r);
    if (w != (size_t)r) {
      Serial.printf("[PullOTA] Update.write failed. Err=%u\n", Update.getError());
      Update.abort();
      http.end();
      return false;
    }

    written += w;

    if (total > 0 && (millis() - lastPrint) > 1000) {
      uint32_t pct = (uint32_t)((written * 100UL) / (size_t)total);
      Serial.printf("[PullOTA] Progress: %u%%\n", pct);
      lastPrint = millis();
    }
  }

  if (!Update.end(true)) {
    Serial.printf("[PullOTA] Update.end failed. Err=%u\n", Update.getError());
    http.end();
    return false;
  }

  if (!Update.isFinished()) {
    Serial.println("[PullOTA] Update not finished!");
    http.end();
    return false;
  }

  Serial.printf("[PullOTA] Update finished OK. Bytes=%u\n", (unsigned)written);
  http.end();
  return true;
}