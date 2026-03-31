#include "FirebaseNetwork.h"
#include <time.h>
#include <sys/time.h>

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

void FirebaseNetwork::begin(const char* apiKey, const char* dbUrl, const char* tz, const char* ntp1, const char* ntp2) {
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

bool FirebaseNetwork::enqueueHistory_(const HistoryJob& job) {
  if (_historyCount >= HISTORY_QUEUE_MAX) return false;
  _historyQueue[_historyTail] = job;
  _historyTail = (uint8_t)((_historyTail + 1U) % HISTORY_QUEUE_MAX);
  _historyCount++;
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
  if (v) clearControlToken_("/controls/open_portal_token", _portalToken, _portalRequestPending);
  else _portalRequestPending = false;
  return v;
}

bool FirebaseNetwork::consumeRelayOnRequest() {
  const bool v = _relayOnPending;
  if (v) clearControlToken_("/controls/relay_on_token", _relayOnToken, _relayOnPending);
  else _relayOnPending = false;
  return v;
}

bool FirebaseNetwork::consumeRelayOffRequest() {
  const bool v = _relayOffPending;
  if (v) clearControlToken_("/controls/relay_off_token", _relayOffToken, _relayOffPending);
  else _relayOffPending = false;
  return v;
}

bool FirebaseNetwork::consumeFaultClearRequest() {
  const bool v = _faultClearPending;
  if (v) clearControlToken_("/controls/fault_clear_token", _faultClearToken, _faultClearPending);
  else _faultClearPending = false;
  return v;
}

bool FirebaseNetwork::consumeRevertFirmwareRequest() {
  const bool v = _revertFwPending;
  if (v) clearControlToken_("/controls/revert_fw_token", _revertFwToken, _revertFwPending);
  else _revertFwPending = false;
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
        token = fbRead.stringData(); token.trim();
        if (_portalTokenPrimed && token.length() && token != _portalToken) _portalRequestPending = true;
        _portalToken = token; _portalTokenPrimed = true;
      }
    } break;
    case 1: {
      String token;
      if (Firebase.RTDB.getString(&fbRead, "/controls/relay_on_token")) {
        token = fbRead.stringData(); token.trim();
        if (_relayOnTokenPrimed && token.length() && token != _relayOnToken) _relayOnPending = true;
        _relayOnToken = token; _relayOnTokenPrimed = true;
      }
    } break;
    case 2: {
      String token;
      if (Firebase.RTDB.getString(&fbRead, "/controls/relay_off_token")) {
        token = fbRead.stringData(); token.trim();
        if (_relayOffTokenPrimed && token.length() && token != _relayOffToken) _relayOffPending = true;
        _relayOffToken = token; _relayOffTokenPrimed = true;
      }
    } break;
    case 3: {
      String token;
      if (Firebase.RTDB.getString(&fbRead, "/controls/fault_clear_token")) {
        token = fbRead.stringData(); token.trim();
        if (_faultClearTokenPrimed && token.length() && token != _faultClearToken) _faultClearPending = true;
        _faultClearToken = token; _faultClearTokenPrimed = true;
      }
    } break;
    case 4: {
      String token;
      if (Firebase.RTDB.getString(&fbRead, "/controls/revert_fw_token")) {
        token = fbRead.stringData(); token.trim();
        if (_revertFwTokenPrimed && token.length() && token != _revertFwToken) _revertFwPending = true;
        _revertFwToken = token; _revertFwTokenPrimed = true;
      }
    } break;
    case 5:
      if (Firebase.RTDB.getBool(&fbRead, "/ml_log/enabled")) _mlEnabledCache = fbRead.boolData();
      break;
    case 6:
      if (Firebase.RTDB.getInt(&fbRead, "/ml_log/duration_s")) _mlDurationCache = fbRead.intData();
      break;
    case 7:
      if (Firebase.RTDB.getInt(&fbRead, "/ml_log/label_override")) _mlLabelOverrideCache = fbRead.intData();
      break;
    case 8:
      if (Firebase.RTDB.getString(&fbRead, "/ml_log/session_id")) { _mlSessionIdCache = fbRead.stringData(); _mlSessionIdCache.trim(); }
      break;
    case 9:
      if (Firebase.RTDB.getString(&fbRead, "/ml_log/load_type")) { _mlLoadTypeCache = fbRead.stringData(); _mlLoadTypeCache.trim(); }
      break;
    default:
      break;
  }
  _controlPollSlot = (uint8_t)((_controlPollSlot + 1U) % 10U);
}

void FirebaseNetwork::requestLiveUpdate(float v, float c, float apparentPower, float t,
                                        float cycle_nmse, float zcv, float zc_dwell_ratio,
                                        float pulse_count_per_cycle, float peak_fluct_cv,
                                        float midband_residual_rms, float hf_band_energy_ratio,
                                        float wpe_entropy, float spec_entropy, float thd_i,
                                        uint8_t model_pred,
                                        const String& state) {
  const bool isNormal = (state == "NORMAL");
  const bool stateChanged = (state != _lastSentLiveState);
  const unsigned long now = millis();
  const uint32_t interval = isNormal ? _normalIntervalMs : _faultIntervalMs;
  const bool shouldSend = stateChanged || (now - _lastLiveSend >= interval) || (now - _lastLiveSend >= CLOUD_REFRESH_KEEPALIVE_MS && !isNormal);
  if (!shouldSend && !_pendingLive) return;

  _live.v = v;
  _live.c = c;
  _live.apparentPower = apparentPower;
  _live.t = t;
  _live.cycle_nmse = cycle_nmse;
  _live.zcv = zcv;
  _live.zc_dwell_ratio = zc_dwell_ratio;
  _live.pulse_count_per_cycle = pulse_count_per_cycle;
  _live.peak_fluct_cv = peak_fluct_cv;
  _live.midband_residual_rms = midband_residual_rms;
  _live.hf_band_energy_ratio = hf_band_energy_ratio;
  _live.wpe_entropy = wpe_entropy;
  _live.spec_entropy = spec_entropy;
  _live.thd_i = thd_i;
  _live.model_pred = model_pred;
  _live.state = state;
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

  if (job.useFeaturePayload) {
    json.set("voltage", job.f.vrms);
    json.set("current", job.f.irms);
    json.set("apparent_power", job.apparentPower);
    json.set("temp", job.f.temp_c);
    json.set("cycle_nmse", job.f.cycle_nmse);
    json.set("zcv", job.f.zcv);
    json.set("zc_dwell_ratio", job.f.zc_dwell_ratio);
    json.set("pulse_count_per_cycle", job.f.pulse_count_per_cycle);
    json.set("peak_fluct_cv", job.f.peak_fluct_cv);
    json.set("midband_residual_rms", job.f.midband_residual_rms);
    json.set("hf_band_energy_ratio", job.f.hf_band_energy_ratio);
    json.set("wpe_entropy", job.f.wpe_entropy);
    json.set("spec_entropy", job.f.spec_entropy);
    json.set("thd_i", job.f.thd_i);
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
    json.set("cycle_nmse", 0.0f);
    json.set("zcv", 0.0f);
    json.set("zc_dwell_ratio", 0.0f);
    json.set("pulse_count_per_cycle", 0.0f);
    json.set("peak_fluct_cv", 0.0f);
    json.set("midband_residual_rms", 0.0f);
    json.set("hf_band_energy_ratio", 0.0f);
    json.set("wpe_entropy", 0.0f);
    json.set("spec_entropy", 0.0f);
    json.set("thd_i", 0.0f);
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
  if (_historyCount == 0) return false;
  HistoryJob job;
  if (!dequeueHistory_(job)) return false;
  if (!pushHistoryRecord_(job)) {
    enqueueHistory_(job);
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
  json.set("cycle_nmse", _live.cycle_nmse);
  json.set("zcv", _live.zcv);
  json.set("zc_dwell_ratio", _live.zc_dwell_ratio);
  json.set("pulse_count_per_cycle", _live.pulse_count_per_cycle);
  json.set("peak_fluct_cv", _live.peak_fluct_cv);
  json.set("midband_residual_rms", _live.midband_residual_rms);
  json.set("hf_band_energy_ratio", _live.hf_band_energy_ratio);
  json.set("wpe_entropy", _live.wpe_entropy);
  json.set("spec_entropy", _live.spec_entropy);
  json.set("thd_i", _live.thd_i);
  json.set("model_pred", (int)_live.model_pred);
  json.set("status", _live.state);
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

  if (!Firebase.RTDB.updateNode(&fbLive, "/live_data", &json)) return false;

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
  return _manualEnabled ? _manual : _auto;
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
  if (_manualEnabled || _autoEnabled || _mlUploadActive) return false;
  if (sec < ML_LOG_AUTO_MIN_DURATION_S) sec = ML_LOG_AUTO_MIN_DURATION_S;
  if (sec > ML_LOG_AUTO_MAX_DURATION_S) sec = ML_LOG_AUTO_MAX_DURATION_S;
  String autoSession = String("auto_");
  autoSession += reason;
  autoSession += "_";
  autoSession += String((unsigned long)millis());
  _auto.sessionId = sanitizeToken(autoSession);

  String autoLoadType = String("auto_");
  autoLoadType += reason;
  _auto.loadType = sanitizeToken(autoLoadType);
  _auto.labelOverride = ML_UNKNOWN_LABEL;
  _auto.durationS = sec;
  _autoEnabled = true;
  resetLoggerRuntime_();
  return true;
}

void FirebaseNetwork::stopAutoCapture() {
  _autoEnabled = false;
  resetLoggerRuntime_();
}

void FirebaseNetwork::ingestLog(const FeatureFrame& f, FaultState st, int arcCounter) {
  if (!logEnabled() || _mlUploadActive) return;
  const SessionSpec& spec = activeSpec();
  if (spec.sessionId.length() < 3 || _count >= MAX_REC) return;

  const bool usefulCurrent = (f.current_valid != 0) || (f.feat_valid != 0) || (f.irms >= CURRENT_DISPLAY_OFF_A);
  const bool mainsPresent = (f.vrms >= MAINS_PRESENT_ON_V);
  const bool keepFault = (st != STATE_NORMAL);
  if (!keepFault && (!mainsPresent || !usefulCurrent)) return;

  if (_sessionStartMs == 0) _sessionStartMs = millis();
  if (_count == 0) _chunkStartMs = millis();

  Rec& r = _buf[_count++];
  r.epoch_ms = f.epoch_ms;
  r.cycle_nmse = f.cycle_nmse;
  r.zcv = f.zcv;
  r.zc_dwell_ratio = f.zc_dwell_ratio;
  r.pulse_count_per_cycle = f.pulse_count_per_cycle;
  r.peak_fluct_cv = f.peak_fluct_cv;
  r.midband_residual_rms = f.midband_residual_rms;
  r.hf_band_energy_ratio = f.hf_band_energy_ratio;
  r.wpe_entropy = f.wpe_entropy;
  r.spec_entropy = f.spec_entropy;
  r.thd_i = f.thd_i;
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
      _uploadChunkIndex = 0;
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

  const bool manualTimeUp = (_manualEnabled && !_autoEnabled) && ((now - _sessionStartMs) >= ((uint32_t)spec.durationS * 1000UL));
  const bool autoTimeUp   = activeIsAuto() && ((now - _sessionStartMs) >= ((uint32_t)spec.durationS * 1000UL));
  const bool chunkTimeUp  = (_count > 0) && (_chunkStartMs != 0) && ((now - _chunkStartMs) >= ((uint32_t)ML_LOG_CHUNK_DURATION_S * 1000UL));
  const bool full         = (_count >= MAX_REC);

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
  _uploadChunkIndex = 0;
  _uploadFinalFlush = manualTimeUp || autoTimeUp;
  _uploadAuto = activeIsAuto();
  _mlUploadActive = (_uploadTotalCount > 0);
}

bool FirebaseNetwork::serviceMlUpload_() {
  if (!_mlUploadActive || !isReady()) return false;
  if (_uploadNextIndex >= _uploadTotalCount) {
    const String finishedSessionId = _uploadSpec.sessionId;
    _count = 0;
    _chunkStartMs = 0;
    _mlUploadActive = false;
    _uploadTotalCount = 0;
    _uploadNextIndex = 0;
    _uploadChunkIndex = 0;
    _uploadChunkCount = 0;
    if (_uploadFinalFlush) {
      if (_uploadAuto) _autoEnabled = false;
      else (void)closeManualSession_(finishedSessionId);
      _sessionStartMs = 0;
    }
    return false;
  }

  const uint16_t i0 = _uploadNextIndex;
  const uint16_t i1 = ((uint16_t)(i0 + ROWS_PER_CHUNK) < _uploadTotalCount) ? (uint16_t)(i0 + ROWS_PER_CHUNK) : _uploadTotalCount;
  const char* header = "cycle_nmse,zcv,zc_dwell_ratio,pulse_count_per_cycle,peak_fluct_cv,midband_residual_rms,hf_band_energy_ratio,wpe_entropy,spec_entropy,thd_i,v_rms,i_rms,temp_c,label_arc,load_type,session_id,epoch_ms,model_pred,feat_valid,current_valid,fault_state,arc_counter,adc_fs_hz,auto_capture\n";

  String csv;
  csv.reserve((i1 - i0) * 220 + 320);
  csv += header;
  for (uint16_t i = i0; i < i1; ++i) {
    const Rec& r = _buf[i];
    csv += String(r.cycle_nmse, 6);            csv += ",";
    csv += String(r.zcv, 6);                   csv += ",";
    csv += String(r.zc_dwell_ratio, 6);        csv += ",";
    csv += String(r.pulse_count_per_cycle, 6); csv += ",";
    csv += String(r.peak_fluct_cv, 6);         csv += ",";
    csv += String(r.midband_residual_rms, 6);  csv += ",";
    csv += String(r.hf_band_energy_ratio, 6);  csv += ",";
    csv += String(r.wpe_entropy, 6);           csv += ",";
    csv += String(r.spec_entropy, 6);          csv += ",";
    csv += String(r.thd_i, 4);                 csv += ",";
    csv += String(r.v_rms, 3);                 csv += ",";
    csv += String(r.i_rms, 6);                 csv += ",";
    csv += String(r.temp_c, 3);                csv += ",";
    csv += String((int)r.label_arc);           csv += ",";
    csv += _uploadSpec.loadType;               csv += ",";
    csv += _uploadSpec.sessionId;              csv += ",";
    csv += String((unsigned long long)r.epoch_ms); csv += ",";
    csv += String((int)r.model_pred);          csv += ",";
    csv += String((int)r.feat_valid);          csv += ",";
    csv += String((int)r.current_valid);       csv += ",";
    csv += String((int)r.fault_state);         csv += ",";
    csv += String((int)r.arc_counter);         csv += ",";
    csv += String(r.adc_fs_hz, 2);             csv += ",";
    csv += String((int)r.auto_capture);        csv += "\n";
  }

  String path = "/ml_logs/";
  path += _uploadSpec.sessionId;
  FirebaseJson json;
  json.set("created_at/.sv", "timestamp");
  json.set("count", (int)(i1 - i0));
  json.set("total_count", (int)_uploadTotalCount);
  json.set("chunk_index", (int)_uploadChunkIndex);
  json.set("chunk_count", (int)_uploadChunkCount);
  json.set("final", _uploadFinalFlush && (i1 >= _uploadTotalCount));
  json.set("csv", csv);
  json.set("meta/session_id", _uploadSpec.sessionId);
  json.set("meta/load_type", _uploadSpec.loadType);
  json.set("meta/label_override", (int)_uploadSpec.labelOverride);
  json.set("meta/duration_s", (int)_uploadSpec.durationS);
  json.set("meta/auto_capture", _uploadAuto);
  json.set("meta/feature_order", "cycle_nmse,zcv,zc_dwell_ratio,pulse_count_per_cycle,peak_fluct_cv,midband_residual_rms,hf_band_energy_ratio,wpe_entropy,spec_entropy,thd_i");

  if (!Firebase.RTDB.pushJSON(&fbLog, path.c_str(), &json)) return false;

  _uploadNextIndex = i1;
  _uploadChunkIndex++;
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
  if (serviceMlUpload_()) return;
}
