#include "CloudHandler.h"

void CloudHandler::begin(const char* apiKey, const char* dbUrl) {
  (void)apiKey;

  config.database_url = dbUrl;
  config.signer.test_mode = true;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("[Cloud] Firebase Initialized.");
}

bool CloudHandler::isReady() const {
  return Firebase.ready();
}

bool CloudHandler::getString(const char* path, String& out) {
  if (!Firebase.ready()) return false;
  if (!path || !*path) return false;

  if (!Firebase.RTDB.getString(&fbdo, path)) {
    Serial.print("[Cloud] getString FAIL: ");
    Serial.println(fbdo.errorReason());
    return false;
  }

  out = fbdo.stringData();
  return true;
}

void CloudHandler::update(float v, float c, float t, float zcv, float thd, float entropy,
                          const String& state, TimeSync* time) {
  if (!Firebase.ready()) return;

  // Treat your test string "HAPPY" as normal too (so it behaves like NORMAL)
  const bool isNormal = (state == "NORMAL" || state == "HAPPY");
  const bool stateChanged = (state != _lastSentLiveState);

  const unsigned long now = millis();
  bool shouldSendLive = false;

  if (isNormal) {
    if (stateChanged) shouldSendLive = true;
    else if (now - _lastLiveSend >= _normalIntervalMs) shouldSendLive = true;
  } else {
    // fault: send live only on change
    if (stateChanged) shouldSendLive = true;
  }

  if (!shouldSendLive) return;

  _lastLiveSend = now;
  _lastSentLiveState = state;

  uint64_t epochMs = 0;
  String iso = "";
  if (time) {
    epochMs = time->nowEpochMs();
    iso = time->nowISO8601Ms();
  }

  FirebaseJson json;
  json.set("wifi_connected", WiFi.status() == WL_CONNECTED);
  json.set("ip", WiFi.localIP().toString());
  json.set("mdns", "tinyml-smart-plug.local");
  json.set("ota_ready", WiFi.status() == WL_CONNECTED);

  json.set("voltage", v);
  json.set("current", c);
  json.set("temp", t);
  json.set("zcv", zcv);
  json.set("thd", thd);
  json.set("entropy", entropy);
  json.set("status", state);

  json.set("ts_epoch_ms", (double)epochMs);
  json.set("ts_iso", iso);
  json.set("uptime_ms", (int)millis());
  json.set("server_ts/.sv", "timestamp");

  if (!Firebase.RTDB.updateNode(&fbdo, "/live_data", &json)) {
    Serial.print("[Cloud] live_data FAIL: ");
    Serial.println(fbdo.errorReason());
    return;
  }

  // history: faults only, on change
  if (!isNormal) {
    if (state != _lastLoggedFaultState) {
      _lastLoggedFaultState = state;
      if (!Firebase.RTDB.pushJSON(&fbdo, "/history", &json)) {
        Serial.print("[Cloud] history FAIL: ");
        Serial.println(fbdo.errorReason());
      }
    }
  } else {
    _lastLoggedFaultState = "";
  }
}