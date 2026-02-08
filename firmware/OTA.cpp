#include "OTA.h"

#include <WiFi.h>
#include <ArduinoOTA.h>

bool OTAHandler::begin(const char* mdnsName, const char* password, uint16_t port) {
  if (WiFi.status() != WL_CONNECTED) {
    _enabled = false;
    _started = false;
    return false;
  }

  _ip = WiFi.localIP().toString();

  ArduinoOTA.setHostname(mdnsName);
  ArduinoOTA.setPort(port);
  ArduinoOTA.setPassword(password);

  ArduinoOTA.onStart([]() { Serial.println("[OTA] Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("\n[OTA] End"); });
  ArduinoOTA.onProgress([](unsigned int p, unsigned int t) {
    Serial.printf("[OTA] %u%%\r", (t ? (p * 100 / t) : 0));
  });
  ArduinoOTA.onError([](ota_error_t e) {
    Serial.printf("[OTA] Error[%u]\n", e);
  });

  ArduinoOTA.begin();

  _enabled = true;
  _started = true;
  Serial.printf("[OTA] Ready @ %s (port %u)\n", _ip.c_str(), port);
  return true;
}

void OTAHandler::handle() {
  if (!_enabled || !_started) return;
  ArduinoOTA.handle();
}

void OTAHandler::setEnabled(bool en) {
  _enabled = en;
}