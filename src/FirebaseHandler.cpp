#include "FirebaseHandler.h"
#include <time.h>
#include <sys/time.h>

static inline bool cloudNetReady() {
  return WiFi.status() == WL_CONNECTED;
}

static bool timeLooksValid(time_t t) {
  return t > 1577836800;
}

static String powerConditionForState(const String& state, float v) {
  if (state == "ARCING" || state == "HEATING" || state == "OVERLOAD" || state == "SUSTAINED OVERLOAD") return state;
  if (state == "UNPLUGGED" || v <= MAINS_PRESENT_OFF_V) return "UNPLUGGED";
  if (state == "OVERVOLTAGE" || v >= VOLT_OV_DELAY_V) return "OVERVOLTAGE";
  if (state == "UNDERVOLTAGE" || (v >= VOLT_UV_CANDIDATE_RAW_MIN_V && v < VOLT_NORMAL_MIN_V)) return "UNDERVOLTAGE";
  return "NORMAL";
}

static bool isTransitionState(const String& state) {
  return state == "STARTUP_STABILIZING" || state == "WIFI_CONNECTING" || state == "CONFIG_PORTAL" ||
         state == "OTA_UPDATING" || state == "SAFE_MODE";
}

void FirebaseHandler::begin(const char* apiKey, const char* dbUrl, const char* tz, const char* ntp1, const char* ntp2) {
  (void)apiKey;
  config.database_url = dbUrl;
  config.signer.test_mode = true;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!_started) {
    _started = true;
    setenv("TZ", tz, 1);
    tzset();
    configTime(0, 0, ntp1, ntp2);
  }
}

void FirebaseHandler::updateClock() {
  if (!_started || _synced) return;
  time_t now = time(nullptr);
  if (timeLooksValid(now)) _synced = true;
}

uint64_t FirebaseHandler::nowEpochMs() const {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  if (!timeLooksValid(tv.tv_sec)) return 0;
  return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000ULL);
}

String FirebaseHandler::nowISO8601Ms() const {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  if (!timeLooksValid(tv.tv_sec)) return String("");

  struct tm tmLocal;
  localtime_r(&tv.tv_sec, &tmLocal);
  char buf[40];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
           tmLocal.tm_year + 1900, tmLocal.tm_mon + 1, tmLocal.tm_mday,
           tmLocal.tm_hour, tmLocal.tm_min, tmLocal.tm_sec, (int)(tv.tv_usec / 1000));
  return String(buf);
}

void FirebaseHandler::setFirmwareVersion(const char* fw) {
  if (fw && *fw) _fwVersion = fw;
}
void FirebaseHandler::setNormalIntervalMs(uint32_t ms) {
  if (ms < 1000) ms = 1000;
  _normalIntervalMs = ms;
}
void FirebaseHandler::setFaultIntervalMs(uint32_t ms) {
  if (ms < 200) ms = 200;
  _faultIntervalMs = ms;
}

bool FirebaseHandler::isReady() const { return cloudNetReady() && Firebase.ready(); }

bool FirebaseHandler::getString(const char* path, String& out) {
  if (!isReady() || !path || !*path) return false;
  if (!Firebase.RTDB.getString(&fbRead, path)) return false;
  out = fbRead.stringData();
  return true;
}
bool FirebaseHandler::getBool(const char* path, bool& out) {
  if (!isReady() || !path || !*path) return false;
  if (!Firebase.RTDB.getBool(&fbRead, path)) return false;
  out = fbRead.boolData();
  return true;
}
bool FirebaseHandler::getInt(const char* path, int& out) {
  if (!isReady() || !path || !*path) return false;
  if (!Firebase.RTDB.getInt(&fbRead, path)) return false;
  out = fbRead.intData();
  return true;
}
bool FirebaseHandler::pushJSON(const char* path, FirebaseJson& json) {
  if (!isReady() || !path || !*path) return false;
  return Firebase.RTDB.pushJSON(&fbLog, path, &json);
}

bool FirebaseHandler::updateJSON(const char* path, FirebaseJson& json) {
  if (!isReady() || !path || !*path) return false;
  return Firebase.RTDB.updateNode(&fbLog, path, &json);
}

bool FirebaseHandler::pushHistoryRecord(const String& status, float v, float c, float apparentPower, float t,
                                        float cycle_nmse, float zcv, float zc_dwell_ratio,
                                        float pulse_count_per_cycle, float peak_fluct_cv,
                                        float midband_residual_rms, float hf_band_energy_ratio,
                                        float wpe_entropy, float spec_entropy, float thd_i,
                                        uint8_t model_pred) {
  if (!isReady()) return false;

  const uint64_t epochMs = nowEpochMs();
  const String iso = nowISO8601Ms();

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
  return Firebase.RTDB.pushJSON(&fbHistory, "/history", &json);
}

bool FirebaseHandler::logStatusEvent(const String& status, float v, float c, float apparentPower, float t) {
  return pushHistoryRecord(status, v, c, apparentPower, t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

bool FirebaseHandler::logFeatureEvent(const String& status, const FeatureFrame& f, float apparentPower, bool relayTrip) {
  if (!isReady()) return false;

  const uint64_t epochMs = nowEpochMs();
  const String iso = nowISO8601Ms();
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
  return Firebase.RTDB.pushJSON(&fbHistory, "/history", &json);
}

void FirebaseHandler::update(float v, float c, float apparentPower, float t,
                             float cycle_nmse, float zcv, float zc_dwell_ratio,
                             float pulse_count_per_cycle, float peak_fluct_cv,
                             float midband_residual_rms, float hf_band_energy_ratio,
                             float wpe_entropy, float spec_entropy, float thd_i,
                             uint8_t model_pred,
                             const String& state) {
  if (!isReady()) return;

  const bool isNormal = (state == "NORMAL");
  const bool stateChanged = (state != _lastSentLiveState);
  const unsigned long now = millis();
  const uint32_t interval = isNormal ? _normalIntervalMs : _faultIntervalMs;
  const bool shouldSend = stateChanged || (now - _lastLiveSend >= interval) || (now - _lastLiveSend >= CLOUD_REFRESH_KEEPALIVE_MS && !isNormal);
  if (!shouldSend) return;

  const uint64_t epochMs = nowEpochMs();
  const String iso = nowISO8601Ms();
  const bool mainsPresent = (v >= MAINS_PRESENT_ON_V);
  const String powerCondition = powerConditionForState(state, v);
  const String loadState = (c >= LOAD_ON_DETECT_A) ? String("LOAD ON") : String("LOAD OFF");
  String devicePhase = loadState;
  if (isTransitionState(state)) devicePhase = state;
  else if (state == "UNPLUGGED") devicePhase = "UNPLUGGED";

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
  json.set("load_state", loadState);
  json.set("last_transition", _lastTransitionEvent);
  json.set("last_transition_epoch_ms", (double)_lastTransitionEpochMs);
  json.set("ts_epoch_ms", (double)epochMs);
  json.set("ts_iso", iso);
  json.set("uptime_ms", (int)millis());
  json.set("server_ts/.sv", "timestamp");

  if (!Firebase.RTDB.updateNode(&fbLive, "/live_data", &json)) return;

  _lastLiveSend = now;
  _lastSentLiveState = state;

  String historyStatus = "";
  bool pushHistory = false;

  if (!_bootEventLogged && mainsPresent && !isTransitionState(state) && state != "UNPLUGGED") {
    _bootEventLogged = true;
    _lastTransitionEvent = "DEVICE ON";
    _lastTransitionEpochMs = epochMs;
    logStatusEvent(_lastTransitionEvent, v, c, apparentPower, t);
  } else if (_haveLastMains && !_lastMainsPresent && mainsPresent && !isTransitionState(state) && state != "UNPLUGGED") {
    _lastTransitionEvent = "DEVICE PLUGGED IN";
    _lastTransitionEpochMs = epochMs;
    logStatusEvent(_lastTransitionEvent, v, c, apparentPower, t);
  }

  if (state == "ARCING" || state == "HEATING" || state == "OVERLOAD" ||
      state == "SUSTAINED OVERLOAD" || state == "UNDERVOLTAGE" || state == "OVERVOLTAGE" ||
      state == "SAFE_MODE") {
    historyStatus = state;
    pushHistory = (historyStatus != _lastLoggedFaultState);
    if (pushHistory) _lastLoggedFaultState = historyStatus;
  } else if (state == "UNPLUGGED") {
    historyStatus = "DEVICE UNPLUGGED";
    pushHistory = (historyStatus != _lastLoggedFaultState);
    if (pushHistory) _lastLoggedFaultState = historyStatus;
  } else if (state == "NORMAL") {
    _lastLoggedFaultState = "";

    if (epochMs > 0 && v >= MAINS_PRESENT_ON_V) {
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
                      model_pred);
  }

  _haveLastMains = true;
  _lastMainsPresent = mainsPresent;
}
