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
  if (!Firebase.ready() || !path || !*path) return false;
  if (!Firebase.RTDB.getString(&fbdo, path)) return false;
  out = fbdo.stringData();
  return true;
}

bool CloudHandler::getBool(const char* path, bool& out) {
  if (!Firebase.ready() || !path || !*path) return false;
  if (!Firebase.RTDB.getBool(&fbdo, path)) return false;
  out = fbdo.boolData();
  return true;
}

bool CloudHandler::getInt(const char* path, int& out) {
  if (!Firebase.ready() || !path || !*path) return false;
  if (!Firebase.RTDB.getInt(&fbdo, path)) return false;
  out = fbdo.intData();
  return true;
}

bool CloudHandler::pushJSON(const char* path, FirebaseJson& json) {
  if (!Firebase.ready() || !path || !*path) return false;
  return Firebase.RTDB.pushJSON(&fbdo, path, &json);
}

void CloudHandler::update(float v, float c, float t, float zcv, float thd, float entropy,
                          const String& state, TimeSync* time) {
  if (!Firebase.ready()) return;

  const bool isNormal = (state == "NORMAL");
  const bool stateChanged = (state != _lastSentLiveState);

  const unsigned long now = millis();
  bool shouldSendLive = false;

  if (isNormal) {
    if (stateChanged) shouldSendLive = true;
    else if (now - _lastLiveSend >= _normalIntervalMs) shouldSendLive = true;
  } else {
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

  if (!Firebase.RTDB.updateNode(&fbdo, "/live_data", &json)) return;

  if (!isNormal) {
    if (state != _lastLoggedFaultState) {
      _lastLoggedFaultState = state;
      Firebase.RTDB.pushJSON(&fbdo, "/history", &json);
    }
  } else {
    _lastLoggedFaultState = "";
  }
}