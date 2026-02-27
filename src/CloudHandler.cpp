#include "CloudHandler.h"

void CloudHandler::begin(const char* apiKey, const char* dbUrl) {
  (void)apiKey;

  config.database_url = dbUrl;
  config.signer.test_mode = true;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("[Cloud] Firebase Initialized.");
}

void CloudHandler::setFirmwareVersion(const char* fw) {
  if (fw && *fw) _fwVersion = fw;
}

void CloudHandler::setNormalIntervalMs(uint32_t ms) {
  if (ms < 1000) ms = 1000;
  _normalIntervalMs = ms;
}

void CloudHandler::setFaultIntervalMs(uint32_t ms) {
  if (ms < 200) ms = 200;
  _faultIntervalMs = ms;
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

void CloudHandler::update(float v, float c, float t,
                          float zcv, float thd, float entropy,
                          float hf_ratio, float hf_var,
                          uint8_t model_pred,
                          const String& state, TimeSync* time) {
  if (!Firebase.ready()) return;

  const bool isNormal = (state == "NORMAL");
  const bool stateChanged = (state != _lastSentLiveState);

  const unsigned long now = millis();
  const uint32_t interval = isNormal ? _normalIntervalMs : _faultIntervalMs;

  bool shouldSendLive = false;
  if (stateChanged) shouldSendLive = true;
  else if (now - _lastLiveSend >= interval) shouldSendLive = true;

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

  json.set("fw_version", _fwVersion);

  json.set("voltage", v);
  json.set("current", c);
  json.set("temp", t);

  json.set("zcv", zcv);
  json.set("thd", thd);
  json.set("entropy", entropy);

  json.set("hf_ratio", hf_ratio);
  json.set("hf_var", hf_var);

  json.set("model_pred", (int)model_pred);

  json.set("status", state);

  json.set("ts_epoch_ms", (double)epochMs);
  json.set("ts_iso", iso);
  json.set("uptime_ms", (int)millis());
  json.set("server_ts/.sv", "timestamp");

  if (!Firebase.RTDB.updateNode(&fbdo, "/live_data", &json)) return;

  // Push to /history only when entering a fault state or changing fault type
  if (!isNormal) {
    if (state != _lastLoggedFaultState) {
      _lastLoggedFaultState = state;
      Firebase.RTDB.pushJSON(&fbdo, "/history", &json);
    }
  } else {
    _lastLoggedFaultState = "";
  }
}