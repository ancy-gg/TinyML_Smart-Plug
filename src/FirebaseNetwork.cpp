#include "FirebaseNetwork.h"
#include <time.h>
#include <sys/time.h>
#include <esp_heap_caps.h>
#include <string.h>

static inline bool cloudNetReady_() {
  return WiFi.status() == WL_CONNECTED;
}

bool FirebaseNetwork::timeLooksValid(time_t t) {
  return t > 1577836800;
}

String FirebaseNetwork::powerConditionForState(const String& state, float v) {
  if (state == "ARCING" || state == "HEATING" || state == "OVERLOAD" || state == "SUSTAINED OVERLOAD") return state;
  if (state == "UNPLUGGED" || v <= MAINS_PRESENT_OFF_V) return "UNPLUGGED";
  if (state == "OVERVOLTAGE" || v >= VOLT_OV_DELAY_V) return "OVERVOLTAGE";
  if (state == "UNDERVOLTAGE" || (v >= VOLT_UV_CANDIDATE_RAW_MIN_V && v < VOLT_NORMAL_MIN_V)) return "UNDERVOLTAGE";
  return "NORMAL";
}

bool FirebaseNetwork::isTransitionState(const String& state) {
  return state == "STARTUP_STABILIZING" || state == "WIFI_CONNECTING" || state == "CONFIG_PORTAL" ||
         state == "OTA_UPDATING" || state == "SAFE_MODE";
}

String FirebaseNetwork::sanitizeToken(const String& s) {
  String o = s;
  o.trim();
  if (o.length() == 0) return "unknown";
  o.replace(",", "_");
  o.replace(" ", "_");
  o.replace("/", "_");
  o.replace("\\", "_");
  if (o.length() > 48) o = o.substring(0, 48);
  return o;
}

void FirebaseNetwork::updateControlToken_(const String& tokenIn, bool& primed, String& cache, String& handled, bool& pendingFlag) {
  String token = tokenIn;
  token.trim();

  if (!primed) {
    cache = token;
    primed = true;
    pendingFlag = false;
    return;
  }

  if (token.length() == 0) {
    cache = "";
    pendingFlag = false;
    return;
  }

  if (token == handled) {
    cache = token;
    pendingFlag = false;
    return;
  }

  if (token != cache) pendingFlag = true;
  cache = token;
}

void FirebaseNetwork::ensureBuffersAllocated_() {
  if (_buf) return;

  void* mem = nullptr;
#if defined(ARDUINO_ARCH_ESP32)
  mem = heap_caps_malloc(sizeof(Rec) * MAX_REC_LIMIT, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#endif
  if (mem) {
    _buf = static_cast<Rec*>(mem);
    _maxRec = MAX_REC_LIMIT;
    memset(_buf, 0, sizeof(Rec) * _maxRec);
    return;
  }

  static const uint16_t fallbackCaps[] = {384, 256, 128, 64};
  for (uint16_t cap : fallbackCaps) {
    mem = heap_caps_malloc(sizeof(Rec) * cap, MALLOC_CAP_8BIT);
    if (mem) {
      _buf = static_cast<Rec*>(mem);
      _maxRec = cap;
      memset(_buf, 0, sizeof(Rec) * _maxRec);
      return;
    }
  }

  _buf = nullptr;
  _maxRec = 0;
}

void FirebaseNetwork::begin(const char* apiKey, const char* dbUrl, const char* tz, const char* ntp1, const char* ntp2) {
  (void)apiKey;
  config.database_url = dbUrl;
  config.signer.test_mode = true;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  ensureBuffersAllocated_();

  if (!_started) {
    _started = true;
    setenv("TZ", tz, 1);
    tzset();
    configTime(0, 0, ntp1, ntp2);
  }
}

void FirebaseNetwork::updateClock() {
  if (!_started || _synced) return;
  time_t now = time(nullptr);
  if (timeLooksValid(now)) _synced = true;
}

uint64_t FirebaseNetwork::nowEpochMs() const {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  if (!timeLooksValid(tv.tv_sec)) return 0;
  return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000ULL);
}

String FirebaseNetwork::nowISO8601Ms() const {
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

void FirebaseNetwork::setFirmwareVersion(const char* fw) {
  if (fw && *fw) _fwVersion = fw;
}

void FirebaseNetwork::setNormalIntervalMs(uint32_t ms) {
  if (ms < 1000) ms = 1000;
  _normalIntervalMs = ms;
}

void FirebaseNetwork::setFaultIntervalMs(uint32_t ms) {
  if (ms < 300) ms = 300;
  _faultIntervalMs = ms;
}

bool FirebaseNetwork::isReady() const {
  return cloudNetReady_() && Firebase.ready();
}

bool FirebaseNetwork::getString(const char* path, String& out) {
  if (!isReady() || !path || !*path) return false;
  if (!Firebase.RTDB.getString(&fbRead, path)) return false;
  out = fbRead.stringData();
  return true;
}

bool FirebaseNetwork::getBool(const char* path, bool& out) {
  if (!isReady() || !path || !*path) return false;
  if (!Firebase.RTDB.getBool(&fbRead, path)) return false;
  out = fbRead.boolData();
  return true;
}

bool FirebaseNetwork::getInt(const char* path, int& out) {
  if (!isReady() || !path || !*path) return false;
  if (!Firebase.RTDB.getInt(&fbRead, path)) return false;
  out = fbRead.intData();
  return true;
}

void FirebaseNetwork::stopAllClients() {
  fbLive.stopWiFiClient();
  fbRead.stopWiFiClient();
  fbHistory.stopWiFiClient();
  fbLog.stopWiFiClient();
}

bool FirebaseNetwork::publishOtaDebug(const String& phase, const String& detail, int progress) {
  if (!isReady()) return false;

  const uint64_t epochMs = nowEpochMs();
  const String iso = nowISO8601Ms();

  FirebaseJson json;
  json.set("phase", phase);
  json.set("detail", detail);
  if (progress >= 0) json.set("progress", progress);
  json.set("wifi_connected", WiFi.status() == WL_CONNECTED);
  json.set("fw_version", _fwVersion);
  if (epochMs > 0) json.set("ts_epoch_ms", (double)epochMs);
  if (iso.length()) json.set("ts_iso", iso);
  json.set("server_ts/.sv", "timestamp");

  bool okDbg = Firebase.RTDB.updateNode(&fbLog, "/debug/ota", &json);

  FirebaseJson live;
  live.set("ota_phase", phase);
  live.set("ota_last_error", detail);
  if (progress >= 0) live.set("ota_progress", progress);
  if (epochMs > 0) live.set("ota_debug_epoch_ms", (double)epochMs);
  if (iso.length()) live.set("ota_debug_iso", iso);
  live.set("server_ts/.sv", "timestamp");

  bool okLive = Firebase.RTDB.updateNode(&fbLive, "/live_data", &live);
  return okDbg && okLive;
}

bool FirebaseNetwork::publishControlAck(const String& kind, const String& token) {
  if (!isReady()) return false;

  FirebaseJson json;
  json.set("last_control_ack_kind", kind);
  json.set("last_control_ack_token", token);
  json.set("last_control_ack_server_ts/.sv", "timestamp");
  json.set("last_control_ack_uptime_ms", (int)millis());
  return Firebase.RTDB.updateNode(&fbLive, "/live_data", &json);
}


bool FirebaseNetwork::enqueueHistory_(const HistoryJob& job) {
  if (_historyCount >= HISTORY_QUEUE_MAX) {
    HistoryJob dropped;
    (void)dequeueHistory_(dropped);
  }
  _historyQueue[_historyTail] = job;
  _historyTail = (uint8_t)((_historyTail + 1U) % HISTORY_QUEUE_MAX);
  if (_historyCount < HISTORY_QUEUE_MAX) _historyCount++;
  return true;
}

bool FirebaseNetwork::dequeueHistory_(HistoryJob& job) {
  if (_historyCount == 0) return false;
  job = _historyQueue[_historyHead];
  _historyHead = (uint8_t)((_historyHead + 1U) % HISTORY_QUEUE_MAX);
  _historyCount--;
  return true;
}

void FirebaseNetwork::clearControlToken_(const char* path, String& cache, bool& pendingFlag) {
  pendingFlag = false;
  cache = "";
  if (!isReady() || !path || !*path) return;
  Firebase.RTDB.setString(&fbLog, path, "");
}

bool FirebaseNetwork::consumePortalRequest() {
  const bool v = _portalRequestPending;
  if (v) {
    _portalTokenHandled = _portalToken;
    clearControlToken_("/controls/open_portal_token", _portalToken, _portalRequestPending);
  } else _portalRequestPending = false;
  return v;
}

bool FirebaseNetwork::consumeRelayOnRequest(String* tokenOut) {
  const bool v = _relayOnPending;
  if (v) {
    const String token = _relayOnToken;
    if (tokenOut) *tokenOut = token;
    _relayOnTokenHandled = token;
    clearControlToken_("/controls/relay_on_token", _relayOnToken, _relayOnPending);
  } else {
    _relayOnPending = false;
    if (tokenOut) tokenOut->clear();
  }
  return v;
}

bool FirebaseNetwork::consumeRelayOffRequest(String* tokenOut) {
  const bool v = _relayOffPending;
  if (v) {
    const String token = _relayOffToken;
    if (tokenOut) *tokenOut = token;
    _relayOffTokenHandled = token;
    clearControlToken_("/controls/relay_off_token", _relayOffToken, _relayOffPending);
  } else {
    _relayOffPending = false;
    if (tokenOut) tokenOut->clear();
  }
  return v;
}

bool FirebaseNetwork::consumeFaultClearRequest() {
  const bool v = _faultClearPending;
  if (v) {
    _faultClearTokenHandled = _faultClearToken;
    clearControlToken_("/controls/fault_clear_token", _faultClearToken, _faultClearPending);
  } else _faultClearPending = false;
  return v;
}

bool FirebaseNetwork::consumeRevertFirmwareRequest() {
  const bool v = _revertFwPending;
  if (v) {
    _revertFwTokenHandled = _revertFwToken;
    clearControlToken_("/controls/revert_fw_token", _revertFwToken, _revertFwPending);
  } else _revertFwPending = false;
  return v;
}

bool FirebaseNetwork::consumeOtaCheckRequest() {
  const bool v = _otaCheckPending;
  if (v) {
    _otaCheckTokenHandled = _otaCheckToken;
    clearControlToken_("/controls/ota_check_token", _otaCheckToken, _otaCheckPending);
  } else _otaCheckPending = false;
  return v;
}

bool FirebaseNetwork::fetchMlControl(bool& enabled, int& dur, int& labelOv, String& sid, String& load) const {
  enabled = _mlEnabledCache;
  dur = _mlDurationCache;
  labelOv = _mlLabelOverrideCache;
  sid = _mlSessionIdCache;
  load = _mlLoadTypeCache;
  return true;
}

void FirebaseNetwork::pollControls(bool allowNet, bool portalActive) {
  if (!allowNet || portalActive || !isReady()) return;
  const uint32_t now = millis();
  if ((now - _lastControlPollMs) < CLOUD_CTRL_READ_GAP_MS) return;
  _lastControlPollMs = now;

  switch (_controlPollSlot) {
    case 0: {
      String token;
      if (Firebase.RTDB.getString(&fbRead, "/controls/open_portal_token")) {
        token = fbRead.stringData();
        updateControlToken_(token, _portalTokenPrimed, _portalToken, _portalTokenHandled, _portalRequestPending);
      }
    } break;
    case 1: {
      String token;
      if (Firebase.RTDB.getString(&fbRead, "/controls/relay_on_token")) {
        token = fbRead.stringData();
        updateControlToken_(token, _relayOnTokenPrimed, _relayOnToken, _relayOnTokenHandled, _relayOnPending);
      }
    } break;
    case 2: {
      String token;
      if (Firebase.RTDB.getString(&fbRead, "/controls/relay_off_token")) {
        token = fbRead.stringData();
        updateControlToken_(token, _relayOffTokenPrimed, _relayOffToken, _relayOffTokenHandled, _relayOffPending);
      }
    } break;
    case 3: {
      String token;
      if (Firebase.RTDB.getString(&fbRead, "/controls/fault_clear_token")) {
        token = fbRead.stringData();
        updateControlToken_(token, _faultClearTokenPrimed, _faultClearToken, _faultClearTokenHandled, _faultClearPending);
      }
    } break;
    case 4: {
      String token;
      if (Firebase.RTDB.getString(&fbRead, "/controls/revert_fw_token")) {
        token = fbRead.stringData();
        updateControlToken_(token, _revertFwTokenPrimed, _revertFwToken, _revertFwTokenHandled, _revertFwPending);
      }
    } break;
    case 5: {
      String token;
      if (Firebase.RTDB.getString(&fbRead, "/controls/ota_check_token")) {
        token = fbRead.stringData();
        updateControlToken_(token, _otaCheckTokenPrimed, _otaCheckToken, _otaCheckTokenHandled, _otaCheckPending);
      }
    } break;
    case 6:
      if (Firebase.RTDB.getBool(&fbRead, "/ml_log/enabled")) _mlEnabledCache = fbRead.boolData();
      break;
    case 7:
      if (Firebase.RTDB.getInt(&fbRead, "/ml_log/duration_s")) _mlDurationCache = fbRead.intData();
      break;
    case 8:
      if (Firebase.RTDB.getInt(&fbRead, "/ml_log/label_override")) _mlLabelOverrideCache = fbRead.intData();
      break;
    case 9:
      if (Firebase.RTDB.getString(&fbRead, "/ml_log/session_id")) { _mlSessionIdCache = fbRead.stringData(); _mlSessionIdCache.trim(); }
      break;
    case 10:
      if (Firebase.RTDB.getString(&fbRead, "/ml_log/load_type")) { _mlLoadTypeCache = fbRead.stringData(); _mlLoadTypeCache.trim(); }
      break;
    default:
      break;
  }
  _controlPollSlot = (uint8_t)((_controlPollSlot + 1U) % 11U);
}

void FirebaseNetwork::requestLiveUpdate(float v, float c, float apparentPower, float t,
                                        float spectral_flux_midhf, float residual_crest_factor,
                                        float edge_spike_ratio, float midband_residual_ratio,
                                        float cycle_nmse, float peak_fluct_cv,
                                        float thd_i, float hf_energy_delta,
                                        float zcv, float abs_irms_zscore_vs_baseline,
                                        uint8_t model_pred,
                                        const String& state,
                                        bool faultLatched, bool webControlsLocked, bool relayLatchedOn) {
  const bool isNormal = (state == "NORMAL") || (state == "UNPLUGGED");
  const bool stateChanged = (state != _lastSentLiveState);
  const unsigned long now = millis();
  const uint32_t interval = isNormal ? _normalIntervalMs : _faultIntervalMs;
  const bool shouldSend = stateChanged || (now - _lastLiveSend >= interval) || (now - _lastLiveSend >= CLOUD_REFRESH_KEEPALIVE_MS && !isNormal);
  if (!shouldSend && !_pendingLive) return;

  _live.v = v;
  _live.c = c;
  _live.apparentPower = apparentPower;
  _live.t = t;
  _live.spectral_flux_midhf = spectral_flux_midhf;
  _live.residual_crest_factor = residual_crest_factor;
  _live.edge_spike_ratio = edge_spike_ratio;
  _live.midband_residual_ratio = midband_residual_ratio;
  _live.cycle_nmse = cycle_nmse;
  _live.peak_fluct_cv = peak_fluct_cv;
  _live.thd_i = thd_i;
  _live.hf_energy_delta = hf_energy_delta;
  _live.zcv = zcv;
  _live.abs_irms_zscore_vs_baseline = abs_irms_zscore_vs_baseline;
  _live.model_pred = model_pred;
  _live.state = state;
  _live.faultLatched = faultLatched;
  _live.webControlsLocked = webControlsLocked;
  _live.relayLatchedOn = relayLatchedOn;
  _pendingLive = true;
}

bool FirebaseNetwork::pushHistoryRecord_(const HistoryJob& job) {
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
  json.set("status", job.status);
  json.set("server_ts/.sv", "timestamp");
  json.set("ts_epoch_ms", (double)epochMs);
  json.set("ts_iso", iso);
  json.set("uptime_ms", (int)millis());
  json.set("feature_space_version", ARC_RUNTIME_FEATURE_SPACE_VERSION);

  if (job.useFeaturePayload) {
    json.set("voltage", job.f.vrms);
    json.set("current", job.f.irms);
    json.set("apparent_power", job.apparentPower);
    json.set("temp", job.f.temp_c);
    json.set("spectral_flux_midhf", job.f.spectral_flux_midhf);
    json.set("residual_crest_factor", job.f.residual_crest_factor);
    json.set("edge_spike_ratio", job.f.edge_spike_ratio);
    json.set("midband_residual_ratio", job.f.midband_residual_ratio);
    json.set("cycle_nmse", job.f.cycle_nmse);
    json.set("peak_fluct_cv", job.f.peak_fluct_cv);
    json.set("thd_i", job.f.thd_i);
    json.set("hf_energy_delta", job.f.hf_energy_delta);
    json.set("zcv", job.f.zcv);
    json.set("abs_irms_zscore_vs_baseline", job.f.abs_irms_zscore_vs_baseline);
    json.set("adc_fs_hz", job.f.adc_fs_hz);
    json.set("feat_valid", (int)job.f.feat_valid);
    json.set("current_valid", (int)job.f.current_valid);
    json.set("model_pred", (int)job.f.model_pred);
    json.set("relay_trip", job.relayTrip);
    json.set("mains_present", job.f.vrms >= MAINS_PRESENT_ON_V);
    json.set("power_condition", powerConditionForState(job.status, job.f.vrms));
  } else {
    json.set("voltage", job.f.vrms);
    json.set("current", job.f.irms);
    json.set("apparent_power", job.apparentPower);
    json.set("temp", job.f.temp_c);
    json.set("spectral_flux_midhf", 0.0f);
    json.set("residual_crest_factor", 0.0f);
    json.set("edge_spike_ratio", 0.0f);
    json.set("midband_residual_ratio", 0.0f);
    json.set("cycle_nmse", 0.0f);
    json.set("peak_fluct_cv", 0.0f);
    json.set("thd_i", 0.0f);
    json.set("hf_energy_delta", 0.0f);
    json.set("zcv", 0.0f);
    json.set("abs_irms_zscore_vs_baseline", 0.0f);
    json.set("model_pred", 0);
    json.set("mains_present", job.f.vrms >= MAINS_PRESENT_ON_V);
    json.set("power_condition", powerConditionForState(job.status, job.f.vrms));
  }
  return Firebase.RTDB.pushJSON(&fbHistory, "/history", &json);
}

bool FirebaseNetwork::logStatusEvent(const String& status, float v, float c, float apparentPower, float t) {
  HistoryJob job;
  job.status = status;
  job.f.vrms = v;
  job.f.irms = c;
  job.f.temp_c = t;
  job.apparentPower = apparentPower;
  job.useFeaturePayload = false;
  return enqueueHistory_(job);
}

bool FirebaseNetwork::logFeatureEvent(const String& status, const FeatureFrame& f, float apparentPower, bool relayTrip) {
  HistoryJob job;
  job.status = status;
  job.f = f;
  job.apparentPower = apparentPower;
  job.relayTrip = relayTrip;
  job.useFeaturePayload = true;
  return enqueueHistory_(job);
}

bool FirebaseNetwork::serviceHistory_() {
  if (_historyCount == 0 || !isReady()) return false;
  HistoryJob job;
  if (!dequeueHistory_(job)) return false;
  if (!pushHistoryRecord_(job)) {
    enqueueHistory_(job);
    stopAllClients();
    _txBackoffUntilMs = millis() + CLOUD_TX_RETRY_MS;
    return false;
  }
  _lastTxMs = millis();
  return true;
}

bool FirebaseNetwork::serviceLive_() {
  if (!_pendingLive || !isReady()) return false;

  const uint64_t epochMs = nowEpochMs();
  const String iso = nowISO8601Ms();
  const bool mainsPresent = (_live.v >= MAINS_PRESENT_ON_V);
  const String powerCondition = powerConditionForState(_live.state, _live.v);
  const String loadState = (_live.c >= LOAD_ON_DETECT_A) ? String("LOAD ON") : String("LOAD OFF");
  String devicePhase = loadState;
  if (isTransitionState(_live.state)) devicePhase = _live.state;
  else if (_live.state == "UNPLUGGED") devicePhase = "UNPLUGGED";

  FirebaseJson json;
  json.set("wifi_connected", WiFi.status() == WL_CONNECTED);
  json.set("wifi_rssi", (int)WiFi.RSSI());
  json.set("ip", WiFi.localIP().toString());
  json.set("mdns", "tinyml-smart-plug.local");
  json.set("ota_ready", WiFi.status() == WL_CONNECTED);
  json.set("fw_version", _fwVersion);
  json.set("voltage", _live.v);
  json.set("current", _live.c);
  json.set("apparent_power", _live.apparentPower);
  json.set("temp", _live.t);
  json.set("spectral_flux_midhf", _live.spectral_flux_midhf);
  json.set("residual_crest_factor", _live.residual_crest_factor);
  json.set("edge_spike_ratio", _live.edge_spike_ratio);
  json.set("midband_residual_ratio", _live.midband_residual_ratio);
  json.set("cycle_nmse", _live.cycle_nmse);
  json.set("peak_fluct_cv", _live.peak_fluct_cv);
  json.set("thd_i", _live.thd_i);
  json.set("hf_energy_delta", _live.hf_energy_delta);
  json.set("zcv", _live.zcv);
  json.set("abs_irms_zscore_vs_baseline", _live.abs_irms_zscore_vs_baseline);
  json.set("model_pred", (int)_live.model_pred);
  json.set("status", _live.state);
  json.set("device_online", true);
  json.set("mains_present", mainsPresent);
  json.set("power_condition", powerCondition);
  json.set("device_phase", devicePhase);
  json.set("load_state", loadState);
  json.set("fault_latched", _live.faultLatched);
  json.set("web_controls_locked", _live.webControlsLocked);
  json.set("relay_latched_on", _live.relayLatchedOn);
  json.set("last_transition", _lastTransitionEvent);
  json.set("last_transition_epoch_ms", (double)_lastTransitionEpochMs);
  json.set("ts_epoch_ms", (double)epochMs);
  json.set("ts_iso", iso);
  json.set("uptime_ms", (int)millis());
  json.set("feature_space_version", ARC_RUNTIME_FEATURE_SPACE_VERSION);
  json.set("server_ts/.sv", "timestamp");

  if (!Firebase.RTDB.updateNode(&fbLive, "/live_data", &json)) {
    stopAllClients();
    _txBackoffUntilMs = millis() + CLOUD_TX_RETRY_MS;
    return false;
  }

  _lastLiveSend = millis();
  _lastSentLiveState = _live.state;
  _pendingLive = false;
  _lastTxMs = _lastLiveSend;

  String historyStatus = "";
  bool pushHistory = false;

  if (!_bootEventLogged && mainsPresent && !isTransitionState(_live.state) && _live.state != "UNPLUGGED") {
    _bootEventLogged = true;
    _lastTransitionEvent = "DEVICE ON";
    _lastTransitionEpochMs = epochMs;
    (void)logStatusEvent(_lastTransitionEvent, _live.v, _live.c, _live.apparentPower, _live.t);
  } else if (_haveLastMains && !_lastMainsPresent && mainsPresent && !isTransitionState(_live.state) && _live.state != "UNPLUGGED") {
    _lastTransitionEvent = "DEVICE PLUGGED IN";
    _lastTransitionEpochMs = epochMs;
    (void)logStatusEvent(_lastTransitionEvent, _live.v, _live.c, _live.apparentPower, _live.t);
  }

  if (_live.state == "ARCING" || _live.state == "HEATING" || _live.state == "OVERLOAD" ||
      _live.state == "SUSTAINED OVERLOAD" || _live.state == "UNDERVOLTAGE" || _live.state == "OVERVOLTAGE" ||
      _live.state == "SAFE_MODE") {
    historyStatus = _live.state;
    pushHistory = (historyStatus != _lastLoggedFaultState);
    if (pushHistory) _lastLoggedFaultState = historyStatus;
  } else if (_live.state == "UNPLUGGED") {
    historyStatus = "DEVICE UNPLUGGED";
    pushHistory = (historyStatus != _lastLoggedFaultState);
    if (pushHistory) _lastLoggedFaultState = historyStatus;
  } else if (_live.state == "NORMAL") {
    _lastLoggedFaultState = "";
    if (epochMs > 0 && _live.v >= MAINS_PRESENT_ON_V) {
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
    (void)logStatusEvent(historyStatus, _live.v, _live.c, _live.apparentPower, _live.t);
  }

  _haveLastMains = true;
  _lastMainsPresent = mainsPresent;
  return true;
}

const FirebaseNetwork::SessionSpec& FirebaseNetwork::activeSpec() const {
  return _manual;
}

void FirebaseNetwork::setLogSession(const String& sessionId, const String& loadType, int labelOverride) {
  _manual.sessionId = sanitizeToken(sessionId);
  _manual.loadType  = sanitizeToken(loadType);
  _manual.labelOverride = (int8_t)labelOverride;
}

void FirebaseNetwork::setLogEnabled(bool en) {
  if (en && !_manualEnabled) resetLoggerRuntime_();
  _manualEnabled = en;
}

void FirebaseNetwork::setLogDurationSeconds(uint16_t sec) {
  if (sec < ML_LOG_MIN_DURATION_S) sec = ML_LOG_MIN_DURATION_S;
  if (sec > ML_LOG_MAX_DURATION_S) sec = ML_LOG_MAX_DURATION_S;
  _manual.durationS = sec;
}

bool FirebaseNetwork::startAutoCapture(const String& reason, uint16_t sec) {
  (void)reason;
  (void)sec;
  return false;
}

void FirebaseNetwork::stopAutoCapture() {
  _autoEnabled = false;
}

void FirebaseNetwork::ingestLog(const FeatureFrame& f, FaultState st, int arcCounter) {
  if (!logEnabled()) return;
  const SessionSpec& spec = activeSpec();
  if (spec.sessionId.length() < 3 || !_buf || _maxRec == 0 || _count >= _maxRec) return;

  const bool manualExpired =
      _manualEnabled &&
      (_sessionStartMs != 0) &&
      ((millis() - _sessionStartMs) >= ((uint32_t)spec.durationS * 1000UL));
  if (manualExpired) return;

  const bool usefulCurrent = (f.current_valid != 0) || (f.feat_valid != 0) || (f.irms >= CURRENT_DISPLAY_OFF_A);
  const bool mainsPresent = (f.vrms >= MAINS_PRESENT_ON_V);
  const bool keepFault = (st != STATE_NORMAL);
  if (!keepFault && (!mainsPresent || !usefulCurrent)) return;

  if (_sessionStartMs == 0) _sessionStartMs = millis();
  if (_count == 0) _chunkStartMs = millis();
  if (_count >= _maxRec) return;

  Rec& r = _buf[_count++];
  r.epoch_ms = f.epoch_ms;
  r.uptime_ms = f.uptime_ms;
  r.spectral_flux_midhf = f.spectral_flux_midhf;
  r.residual_crest_factor = f.residual_crest_factor;
  r.edge_spike_ratio = f.edge_spike_ratio;
  r.midband_residual_ratio = f.midband_residual_ratio;
  r.cycle_nmse = f.cycle_nmse;
  r.peak_fluct_cv = f.peak_fluct_cv;
  r.thd_i = f.thd_i;
  r.hf_energy_delta = f.hf_energy_delta;
  r.zcv = f.zcv;
  r.abs_irms_zscore_vs_baseline = f.abs_irms_zscore_vs_baseline;
  r.v_rms = f.vrms;
  r.i_rms = f.irms;
  r.temp_c = f.temp_c;
  r.label_arc = (spec.labelOverride == 0 || spec.labelOverride == 1) ? spec.labelOverride : ML_UNKNOWN_LABEL;
  r.model_pred = f.model_pred;
  r.feat_valid = f.feat_valid;
  r.current_valid = f.current_valid;
  r.fault_state = (uint8_t)st;
  r.arc_counter = (int16_t)arcCounter;
  r.adc_fs_hz = f.adc_fs_hz;
  r.auto_capture = activeIsAuto() ? 1 : 0;
  r.feature_space_version = ARC_RUNTIME_FEATURE_SPACE_VERSION;
}

void FirebaseNetwork::resetLoggerRuntime_() {
  _sessionStartMs = 0;
  _chunkStartMs = 0;
  _count = 0;
  _mlUploadActive = false;
  _uploadTotalCount = 0;
  _uploadNextIndex = 0;
  _uploadChunkCount = 0;
  _uploadChunkIndex = 0;
  _uploadFinalFlush = false;
  _uploadAuto = false;
  _suspendMlUpload = false;
  _sessionChunkSerial = 0;
}

bool FirebaseNetwork::closeManualSession_(const String& finishedSessionId) {
  _manualEnabled = false;
  if (!isReady()) return false;

  FirebaseJson mlState;
  mlState.set("enabled", false);
  mlState.set("last_completed_session_id", finishedSessionId);
  mlState.set("last_completed_at/.sv", "timestamp");
  if (!Firebase.RTDB.updateNode(&fbLog, "/ml_log", &mlState)) return false;

  String sessPath = "/ml_sessions/";
  sessPath += finishedSessionId;
  FirebaseJson sessMeta;
  sessMeta.set("end_ms/.sv", "timestamp");
  sessMeta.set("closed_by_device", true);
  if (!Firebase.RTDB.updateNode(&fbLog, sessPath.c_str(), &sessMeta)) return false;
  return true;
}

void FirebaseNetwork::serviceMlState_() {
  if (_wasEnabled && !logEnabled()) {
    if (_count > 0 && !_mlUploadActive) {
      _uploadSpec = activeSpec();
      _uploadTotalCount = _count;
      _uploadNextIndex = 0;
      _uploadChunkCount = (_uploadTotalCount + ROWS_PER_CHUNK - 1U) / ROWS_PER_CHUNK;
      _uploadChunkIndex = _sessionChunkSerial;
      _uploadFinalFlush = true;
      _uploadAuto = activeIsAuto();
      _mlUploadActive = (_uploadTotalCount > 0);
    } else if (_count == 0) {
      resetLoggerRuntime_();
    }
  }
  _wasEnabled = logEnabled();
  if (!logEnabled()) return;

  const SessionSpec& spec = activeSpec();
  if (spec.sessionId.length() < 3) return;

  const uint32_t now = millis();
  if (_sessionStartMs == 0) _sessionStartMs = now;

  const bool manualTimeUp = _manualEnabled && ((now - _sessionStartMs) >= ((uint32_t)spec.durationS * 1000UL));
  const bool autoTimeUp   = false;
  const bool chunkTimeUp  = (_count > 0) && (_chunkStartMs != 0) && ((now - _chunkStartMs) >= ((uint32_t)ML_LOG_CHUNK_DURATION_S * 1000UL));
  const bool full         = (_maxRec > 0) && (_count >= _maxRec);

  if (_mlUploadActive) return;

  if (_count == 0) {
    if (manualTimeUp) {
      (void)closeManualSession_(spec.sessionId);
      resetLoggerRuntime_();
    } else if (autoTimeUp) {
      _autoEnabled = false;
      resetLoggerRuntime_();
    }
    return;
  }

  if (!manualTimeUp && !autoTimeUp && !chunkTimeUp && !full) return;

  _uploadSpec = spec;
  _uploadTotalCount = _count;
  _uploadNextIndex = 0;
  _uploadChunkCount = (_uploadTotalCount + ROWS_PER_CHUNK - 1U) / ROWS_PER_CHUNK;
  _uploadChunkIndex = _sessionChunkSerial;
  _uploadFinalFlush = manualTimeUp || autoTimeUp;
  _uploadAuto = activeIsAuto();
  _mlUploadActive = (_uploadTotalCount > 0);
}

bool FirebaseNetwork::serviceMlUpload_() {
  if (!_mlUploadActive || !isReady()) return false;
#if defined(ARDUINO_ARCH_ESP32)
  if (ESP.getFreeHeap() < 32000 || heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) < 24000) {
    _txBackoffUntilMs = millis() + 900UL;
    return false;
  }
#endif
  if (_uploadNextIndex >= _uploadTotalCount) {
    const String finishedSessionId = _uploadSpec.sessionId;
    const uint16_t tailCount = (_count > _uploadTotalCount) ? (uint16_t)(_count - _uploadTotalCount) : 0;
    if (tailCount > 0) {
      memmove(_buf, _buf + _uploadTotalCount, sizeof(Rec) * tailCount);
    }
    _count = tailCount;
    _chunkStartMs = (tailCount > 0) ? millis() : 0;
    _mlUploadActive = false;
    _uploadTotalCount = 0;
    _uploadNextIndex = 0;
    _uploadChunkCount = 0;
    if (_uploadFinalFlush && tailCount == 0) {
      if (_uploadAuto) _autoEnabled = false;
      else (void)closeManualSession_(finishedSessionId);
      _sessionStartMs = 0;
    }
    return false;
  }

  const uint16_t i0 = _uploadNextIndex;
  const uint16_t i1 = ((uint16_t)(i0 + ROWS_PER_CHUNK) < _uploadTotalCount) ? (uint16_t)(i0 + ROWS_PER_CHUNK) : _uploadTotalCount;
  const Rec& firstRec = _buf[i0];
  const Rec& lastRec  = _buf[i1 - 1];
  const char* header = "spectral_flux_midhf,residual_crest_factor,edge_spike_ratio,midband_residual_ratio,cycle_nmse,peak_fluct_cv,thd_i,hf_energy_delta,zcv,abs_irms_zscore_vs_baseline,v_rms,i_rms,temp_c,label_arc,load_type,session_id,epoch_ms,uptime_ms,model_pred,feat_valid,current_valid,fault_state,arc_counter,adc_fs_hz,auto_capture,feature_space_version\n";

  String csv;
  csv.reserve((i1 - i0) * 220 + 320);
  csv += header;
  for (uint16_t i = i0; i < i1; ++i) {
    const Rec& r = _buf[i];
    csv += String(r.spectral_flux_midhf, 6);         csv += ",";
    csv += String(r.residual_crest_factor, 6);       csv += ",";
    csv += String(r.edge_spike_ratio, 6);            csv += ",";
    csv += String(r.midband_residual_ratio, 6);      csv += ",";
    csv += String(r.cycle_nmse, 6);                  csv += ",";
    csv += String(r.peak_fluct_cv, 6);               csv += ",";
    csv += String(r.thd_i, 4);                       csv += ",";
    csv += String(r.hf_energy_delta, 6);             csv += ",";
    csv += String(r.zcv, 6);                         csv += ",";
    csv += String(r.abs_irms_zscore_vs_baseline, 6); csv += ",";
    csv += String(r.v_rms, 3);                 csv += ",";
    csv += String(r.i_rms, 6);                 csv += ",";
    csv += String(r.temp_c, 3);                csv += ",";
    csv += String((int)r.label_arc);           csv += ",";
    csv += _uploadSpec.loadType;               csv += ",";
    csv += _uploadSpec.sessionId;              csv += ",";
    csv += String((unsigned long long)r.epoch_ms); csv += ",";
    csv += String((unsigned long)r.uptime_ms); csv += ",";
    csv += String((int)r.model_pred);          csv += ",";
    csv += String((int)r.feat_valid);          csv += ",";
    csv += String((int)r.current_valid);       csv += ",";
    csv += String((int)r.fault_state);         csv += ",";
    csv += String((int)r.arc_counter);         csv += ",";
    csv += String(r.adc_fs_hz, 2);             csv += ",";
    csv += String((int)r.auto_capture);        csv += ",";
    csv += String((int)r.feature_space_version); csv += "\n";
  }

  String path = "/ml_logs/";
  path += _uploadSpec.sessionId;
  FirebaseJson json;
  const uint16_t localChunkIndex = (uint16_t)(i0 / ROWS_PER_CHUNK);

  json.set("created_at/.sv", "timestamp");
  json.set("count", (int)(i1 - i0));
  json.set("total_count", (int)_uploadTotalCount);
  json.set("chunk_index", (int)localChunkIndex);
  json.set("chunk_seq", (int)_uploadChunkIndex);
  json.set("session_chunk_seq", (int)_uploadChunkIndex);
  json.set("chunk_count", (int)_uploadChunkCount);
  json.set("first_epoch_ms", (double)firstRec.epoch_ms);
  json.set("last_epoch_ms", (double)lastRec.epoch_ms);
  json.set("first_uptime_ms", (int)firstRec.uptime_ms);
  json.set("last_uptime_ms", (int)lastRec.uptime_ms);
  json.set("final", _uploadFinalFlush && (i1 >= _uploadTotalCount));
  json.set("csv", csv);
  json.set("meta/session_id", _uploadSpec.sessionId);
  json.set("meta/load_type", _uploadSpec.loadType);
  json.set("meta/label_override", (int)_uploadSpec.labelOverride);
  json.set("meta/duration_s", (int)_uploadSpec.durationS);
  json.set("meta/auto_capture", _uploadAuto);
  json.set("meta/feature_order", "spectral_flux_midhf,residual_crest_factor,edge_spike_ratio,midband_residual_ratio,cycle_nmse,peak_fluct_cv,thd_i,hf_energy_delta,zcv,abs_irms_zscore_vs_baseline");

  if (!Firebase.RTDB.pushJSON(&fbLog, path.c_str(), &json)) return false;

  _uploadNextIndex = i1;
  _uploadChunkIndex++;
  _sessionChunkSerial = _uploadChunkIndex;
  _lastTxMs = millis();
  return true;
}

void FirebaseNetwork::loop() {
  if (!isReady()) return;
  serviceMlState_();

  const uint32_t now = millis();
  if ((int32_t)(now - _txBackoffUntilMs) < 0) return;
  if ((now - _lastTxMs) < CLOUD_TX_MIN_GAP_MS) return;

  if (serviceHistory_()) return;
  if (serviceLive_()) return;
  if (!_suspendMlUpload && serviceMlUpload_()) return;
}
