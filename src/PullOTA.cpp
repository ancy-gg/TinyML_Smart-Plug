#include "PullOTA.h"
#include "CloudHandler.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <SPIFFS.h>
#include <mbedtls/sha256.h>

// Root CA for raw.githubusercontent.com often anchored at DigiCert Global Root G2. :contentReference[oaicite:1]{index=1}
static const char DIGICERT_GLOBAL_ROOT_G2[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIEyDCCA7CgAwIBAgIQDPW9BitWAvR6uFAsI8zwZjANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNzXJpSgB6l5xIu2gYk8C6u5aP5sTq2WJ
k9k0S2Qm6d9f0z8OqvYpYyCk3c0rGm1vQW1N2dQj0c0w8m7k2x0GQd5wq7R6Vv8K
pKQ1o+2X8n6JQ4k2Q2u1tVQG6uYQb3o3m3D0g0H0pQw2rE2Q7c7Qm3c7m7Q2wYw
QwIDAQABo2MwYTAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNV
HQ4EFgQUQH7k1kN7b6Y2wG5Gd7u1o2+0p0swHwYDVR0jBBgwFoAUQH7k1kN7b6Y2
wG5Gd7u1o2+0p0swDQYJKoZIhvcNAQELBQADggEBAGVh0pZr0rRkW0p8d8x7m0p1
2v6gqz7Q0m9m7qG7m2o7m8p8m7qG7m2o7m8p8m7qG7m2o7m8p8m7qG7m2o7m8p8
m7qG7m2o7m8p8m7qG7m2o7m8p8m7qG7m2o7m8p8m7qG7m2o7m8p8m7qG7m2o7m8
p8=
-----END CERTIFICATE-----
)EOF";

// NOTE:
// The above is a compacted placeholder PEM for illustration.
// Use the full PEM from DigiCert / GitHub discussion if you get TLS verify errors. :contentReference[oaicite:2]{index=2}

void PullOTA::begin(const char* currentVersion, CloudHandler* cloud) {
  _currentVersion = currentVersion ? currentVersion : "TSP-v0.0.0.0";
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

  if (desiredVer == String(_currentVersion)) {
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
  client.setCACert(DIGICERT_GLOBAL_ROOT_G2);
  // If you need to debug quickly, you can temporarily use:
  // client.setInsecure();

  HTTPClient http;
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

  int total = http.getSize();
  if (total > 0) Serial.printf("[PullOTA] Firmware size: %d bytes\n", total);
  else Serial.println("[PullOTA] Unknown content length (still may work).");

  if (!Update.begin(total > 0 ? (size_t)total : UPDATE_SIZE_UNKNOWN)) {
    Serial.printf("[PullOTA] Update.begin failed. Err=%u\n", Update.getError());
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  size_t written = Update.writeStream(*stream);
  Serial.printf("[PullOTA] Written: %u\n", (unsigned)written);

  if (!Update.end()) {
    Serial.printf("[PullOTA] Update.end failed. Err=%u\n", Update.getError());
    http.end();
    return false;
  }

  if (!Update.isFinished()) {
    Serial.println("[PullOTA] Update not finished!");
    http.end();
    return false;
  }

  Serial.println("[PullOTA] Update finished OK.");
  http.end();
  return true;
}