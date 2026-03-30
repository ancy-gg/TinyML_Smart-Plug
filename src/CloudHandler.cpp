#include "CloudHandler.h"
#include "SmartPlugConfig.h"
#include <time.h>

static String powerConditionForState(const String& state, float v) {
  if (state == "ARCING" || state == "HEATING" || state == "OVERLOAD") return state;
  if (state == "MANUAL RELAY OFF") return "MANUAL RELAY OFF";
  if (state == "UNPLUGGED") return "UNPLUGGED";
  if (state == "OVERVOLTAGE" || v >= VOLT_OVERVOLT_TRIP_V) return "OVERVOLTAGE";
  if (state == "UNDERVOLTAGE" || (v > VOLT_UNDERVOLT_MIN_V && v < VOLT_UNDERVOLT_MAX_V)) return "UNDERVOLTAGE";
  if (v <= MAINS_PRESENT_OFF_V) return "UNPLUGGED";
  return "NORMAL";
}

static bool isTransitionState(const String& state) {
  return state == "STARTUP_STABILIZING" || state == "WIFI_CONNECTING" || state == "CONFIG_PORTAL" ||
         state == "OTA_UPDATING" || state == "SAFE_MODE";
}

void CloudHandler::begin(const char* apiKey, const char* dbUrl) {
  (void)apiKey;
  config.database_url = dbUrl;
  config.signer.test_mode = true;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
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

bool CloudHandler::isReady() const { return Firebase.ready(); }

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

bool CloudHandler::pushHistoryRecord(const String& status, float v, float c, float apparentPower, float t,
                                     float cycle_nmse, float zcv, float zc_dwell_ratio,
                                     float pulse_count_per_cycle, float peak_fluct_cv,
                                     float midband_residual_rms, float hf_band_energy_ratio,
                                     float wpe_entropy, float spec_entropy, float thd_i,
                                     uint8_t model_pred,
                                     TimeSync* time) {
  if (!Firebase.ready()) return false;

  uint64_t epochMs = 0;
  String iso = "";
  if (time) {
    epochMs = time->nowEpochMs();
    iso = time->nowISO8601Ms();
  }

  FirebaseJson json;
  json.set("wifi_connected", WiFi.status() == WL_CONNECTED);
  json.set("wifi_rssi", (int)WiFi.RSSI());
  json.set("ip", WiFi.localIP().toString());
  json.set("mdns", "tinyml-smart-plug.local");
  json.set("ota_ready", WiFi.status() == WL_CONNECTED);
  json.set("fw_version", _fwVersion);

  json.set("voltage", v);
  json.set("current", c);
  json.set("apparent_power", apparentPower);
  json.set("temp", t);

  json.set("cycle_nmse", cycle_nmse);
  json.set("zcv", zcv);
  json.set("zc_dwell_ratio", zc_dwell_ratio);
  json.set("pulse_count_per_cycle", pulse_count_per_cycle);
  json.set("peak_fluct_cv", peak_fluct_cv);
  json.set("midband_residual_rms", midband_residual_rms);
  json.set("hf_band_energy_ratio", hf_band_energy_ratio);
  json.set("wpe_entropy", wpe_entropy);
  json.set("spec_entropy", spec_entropy);
  json.set("thd_i", thd_i);

  json.set("model_pred", (int)model_pred);
  json.set("status", status);

  json.set("mains_present", v >= MAINS_PRESENT_ON_V);
  json.set("power_condition", powerConditionForState(status, v));
  json.set("ts_epoch_ms", (double)epochMs);
  json.set("ts_iso", iso);
  json.set("uptime_ms", (int)millis());
  json.set("server_ts/.sv", "timestamp");

  return Firebase.RTDB.pushJSON(&fbdo, "/history", &json);
}

bool CloudHandler::logStatusEvent(const String& status, float v, float c, float apparentPower, float t, TimeSync* time) {
  return pushHistoryRecord(status, v, c, apparentPower, t,
                           0.0f, 0.0f, 0.0f,
                           0.0f, 0.0f,
                           0.0f, 0.0f,
                           0.0f, 0.0f, 0.0f,
                           0, time);
}

bool CloudHandler::logFeatureEvent(const String& status, const FeatureFrame& f, float apparentPower, bool relayTrip, TimeSync* time) {
  if (!Firebase.ready()) return false;

  uint64_t epochMs = 0;
  String iso = "";
  if (time) {
    epochMs = time->nowEpochMs();
    iso = time->nowISO8601Ms();
  }

  FirebaseJson json;
  json.set("wifi_connected", WiFi.status() == WL_CONNECTED);
  json.set("wifi_rssi", (int)WiFi.RSSI());
  json.set("ip", WiFi.localIP().toString());
  json.set("mdns", "tinyml-smart-plug.local");
  json.set("ota_ready", WiFi.status() == WL_CONNECTED);
  json.set("fw_version", _fwVersion);

  json.set("voltage", f.vrms);
  json.set("current", f.irms);
  json.set("apparent_power", apparentPower);
  json.set("temp", f.temp_c);

  json.set("cycle_nmse", f.cycle_nmse);
  json.set("zcv", f.zcv);
  json.set("zc_dwell_ratio", f.zc_dwell_ratio);
  json.set("pulse_count_per_cycle", f.pulse_count_per_cycle);
  json.set("peak_fluct_cv", f.peak_fluct_cv);
  json.set("midband_residual_rms", f.midband_residual_rms);
  json.set("hf_band_energy_ratio", f.hf_band_energy_ratio);
  json.set("wpe_entropy", f.wpe_entropy);
  json.set("spec_entropy", f.spec_entropy);
  json.set("thd_i", f.thd_i);
  json.set("adc_fs_hz", f.adc_fs_hz);

  json.set("feat_valid", (int)f.feat_valid);
  json.set("current_valid", (int)f.current_valid);
  json.set("model_pred", (int)f.model_pred);
  json.set("relay_trip", relayTrip);
  json.set("status", status);

  json.set("mains_present", f.vrms >= MAINS_PRESENT_ON_V);
  json.set("power_condition", powerConditionForState(status, f.vrms));
  json.set("ts_epoch_ms", (double)epochMs);
  json.set("ts_iso", iso);
  json.set("uptime_ms", (int)millis());
  json.set("server_ts/.sv", "timestamp");

  return Firebase.RTDB.pushJSON(&fbdo, "/history", &json);
}

void CloudHandler::update(float v, float c, float apparentPower, float t,
                          float cycle_nmse, float zcv, float zc_dwell_ratio,
                          float pulse_count_per_cycle, float peak_fluct_cv,
                          float midband_residual_rms, float hf_band_energy_ratio,
                          float wpe_entropy, float spec_entropy, float thd_i,
                          uint8_t model_pred,
                          const String& state, TimeSync* time) {
  if (!Firebase.ready()) return;

  const bool isNormal = (state == "NORMAL");
  const bool stateChanged = (state != _lastSentLiveState);
  const unsigned long now = millis();
  const uint32_t interval = isNormal ? _normalIntervalMs : _faultIntervalMs;

  bool shouldSend = false;
  if (stateChanged) shouldSend = true;
  else if (now - _lastLiveSend >= interval) shouldSend = true;
  if (!shouldSend) return;

  _lastLiveSend = now;
  _lastSentLiveState = state;

  uint64_t epochMs = 0;
  String iso = "";
  if (time) {
    epochMs = time->nowEpochMs();
    iso = time->nowISO8601Ms();
  }

  const bool mainsPresent = (v >= MAINS_PRESENT_ON_V);
  const String powerCondition = powerConditionForState(state, v);
  String devicePhase = "ACTIVE";
  if (isTransitionState(state)) devicePhase = state;
  else if (state == "UNPLUGGED") devicePhase = "UNPLUGGED";
  else if (state == "MANUAL RELAY OFF") devicePhase = "MANUAL RELAY OFF";

  FirebaseJson json;
  json.set("wifi_connected", WiFi.status() == WL_CONNECTED);
  json.set("wifi_rssi", (int)WiFi.RSSI());
  json.set("ip", WiFi.localIP().toString());
  json.set("mdns", "tinyml-smart-plug.local");
  json.set("ota_ready", WiFi.status() == WL_CONNECTED);
  json.set("fw_version", _fwVersion);

  json.set("voltage", v);
  json.set("current", c);
  json.set("apparent_power", apparentPower);
  json.set("temp", t);

  json.set("cycle_nmse", cycle_nmse);
  json.set("zcv", zcv);
  json.set("zc_dwell_ratio", zc_dwell_ratio);
  json.set("pulse_count_per_cycle", pulse_count_per_cycle);
  json.set("peak_fluct_cv", peak_fluct_cv);
  json.set("midband_residual_rms", midband_residual_rms);
  json.set("hf_band_energy_ratio", hf_band_energy_ratio);
  json.set("wpe_entropy", wpe_entropy);
  json.set("spec_entropy", spec_entropy);
  json.set("thd_i", thd_i);

  json.set("model_pred", (int)model_pred);
  json.set("status", state);
  json.set("device_online", true);
  json.set("mains_present", mainsPresent);
  json.set("power_condition", powerCondition);
  json.set("device_phase", devicePhase);
  json.set("last_transition", _lastTransitionEvent);
  json.set("last_transition_epoch_ms", (double)_lastTransitionEpochMs);

  json.set("ts_epoch_ms", (double)epochMs);
  json.set("ts_iso", iso);
  json.set("uptime_ms", (int)millis());
  json.set("server_ts/.sv", "timestamp");

  if (!Firebase.RTDB.updateNode(&fbdo, "/live_data", &json)) return;

  String historyStatus = "";
  bool pushHistory = false;

  if (!_bootEventLogged && mainsPresent && !isTransitionState(state) && state != "UNPLUGGED") {
    _bootEventLogged = true;
    _lastTransitionEvent = "DEVICE ON";
    _lastTransitionEpochMs = epochMs;
    logStatusEvent(_lastTransitionEvent, v, c, apparentPower, t, time);
  } else if (_haveLastMains && !_lastMainsPresent && mainsPresent && !isTransitionState(state) && state != "UNPLUGGED") {
    _lastTransitionEvent = "DEVICE PLUGGED IN";
    _lastTransitionEpochMs = epochMs;
    logStatusEvent(_lastTransitionEvent, v, c, apparentPower, t, time);
  }

  if (state == "ARCING" || state == "HEATING" || state == "OVERLOAD" ||
      state == "UNDERVOLTAGE" || state == "OVERVOLTAGE" ||
      state == "SURGE" || state == "SAFE_MODE" || state == "MANUAL RELAY OFF") {
    historyStatus = state;
    pushHistory = (historyStatus != _lastLoggedFaultState);
    if (pushHistory) _lastLoggedFaultState = historyStatus;
  } else if (state == "UNPLUGGED") {
    historyStatus = "DEVICE UNPLUGGED";
    pushHistory = (historyStatus != _lastLoggedFaultState);
    if (pushHistory) _lastLoggedFaultState = historyStatus;
  } else if (state == "NORMAL") {
    _lastLoggedFaultState = "";

    if (time && epochMs > 0 && v >= MAINS_PRESENT_ON_V) {
      time_t sec = (time_t)(epochMs / 1000ULL);
      struct tm tmLocal;
      localtime_r(&sec, &tmLocal);

      char hourKey[16];
      snprintf(hourKey, sizeof(hourKey), "%04d%02d%02d%02d",
               tmLocal.tm_year + 1900, tmLocal.tm_mon + 1, tmLocal.tm_mday, tmLocal.tm_hour);

      if (tmLocal.tm_min == 0 && tmLocal.tm_sec < 10 && _lastHourlyNormalKey != hourKey) {
        _lastHourlyNormalKey = hourKey;
        historyStatus = "NORMAL";
        pushHistory = true;
      }
    }
  }

  if (pushHistory && historyStatus.length()) {
    pushHistoryRecord(historyStatus, v, c, apparentPower, t,
                      cycle_nmse, zcv, zc_dwell_ratio,
                      pulse_count_per_cycle, peak_fluct_cv,
                      midband_residual_rms, hf_band_energy_ratio,
                      wpe_entropy, spec_entropy, thd_i,
                      model_pred, time);
  }

  _haveLastMains = true;
  _lastMainsPresent = mainsPresent;
}
