#include "FirebaseNetwork.h"
#include <time.h>
#include <sys/time.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <stdlib.h>

static inline bool cloudNetReady_() {
  return WiFi.status() == WL_CONNECTED;
}

static inline bool cloudHeapHealthy_() {
#if defined(ARDUINO_ARCH_ESP32)
  return (ESP.getFreeHeap() >= 48000) &&
         (heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) >= 28000);
#else
  return true;
#endif
}

void FirebaseNetwork::configureClient_(FirebaseData& client, uint16_t responseSize) {
  client.setBSSLBufferSize(CLOUD_TLS_RX_BUFFER_BYTES, CLOUD_TLS_TX_BUFFER_BYTES);
  client.setResponseSize(responseSize);
  client.keepAlive(5, 5, 1);
  Firebase.RTDB.setReadTimeout(&client, CLOUD_SERVER_RESPONSE_TIMEOUT_MS);
}

void FirebaseNetwork::recoverClient_(FirebaseData& client, uint32_t backoffMs) {
  client.stopWiFiClient();
  _txBackoffUntilMs = millis() + backoffMs;
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

static int8_t deviceFamilyCodeFromToken_(String token) {
  token.trim();
  token.toLowerCase();
  token.replace("-", "_");
  token.replace(" ", "_");
  while (token.indexOf("__") >= 0) token.replace("__", "_");

  if (token == "resistive" || token == "resistive_linear" || token == "heater" || token == "heating") {
    return CONTEXT_FAMILY_RESISTIVE_LINEAR;
  }
  if (token == "inductive" || token == "motor" || token == "fan" || token == "inductive_motor") {
    return CONTEXT_FAMILY_INDUCTIVE_MOTOR;
  }
  if (token == "smps" || token == "rectifier" || token == "rectifier_smps" || token == "charger" || token == "adapter") {
    return CONTEXT_FAMILY_RECTIFIER_SMPS;
  }
  if (token == "dimmer" || token == "phase" || token == "dimmer_phase" || token == "phase_angle" || token == "phase_angle_controlled") {
    return CONTEXT_FAMILY_PHASE_ANGLE_CONTROLLED;
  }
  if (token == "universal" || token == "universal_motor" || token == "brush" || token == "brush_universal_motor" || token == "vacuum") {
    return CONTEXT_FAMILY_BRUSH_UNIVERSAL_MOTOR;
  }
  if (token == "mixed" || token == "mixed_unknown" || token == "other" || token == "other_mixed") {
    return CONTEXT_FAMILY_OTHER_MIXED;
  }
  return CONTEXT_FAMILY_UNKNOWN;
}

const char* FirebaseNetwork::loadFamilyLabel_(int8_t code) {
  switch (code) {
    case CONTEXT_FAMILY_RESISTIVE_LINEAR: return "RESISTIVE";
    case CONTEXT_FAMILY_INDUCTIVE_MOTOR: return "INDUCTIVE";
    case CONTEXT_FAMILY_RECTIFIER_SMPS: return "SMPS";
    case CONTEXT_FAMILY_PHASE_ANGLE_CONTROLLED: return "PHASE CONTROL";
    case CONTEXT_FAMILY_BRUSH_UNIVERSAL_MOTOR: return "BRUSHED MOTOR";
    case CONTEXT_FAMILY_OTHER_MIXED: return "OTHER";
    default: return "UNKNOWN";
  }
}

String FirebaseNetwork::formatDurationHms_(uint64_t durationMs) {
  const uint64_t totalSec = durationMs / 1000ULL;
  const uint32_t hours = (uint32_t)(totalSec / 3600ULL);
  const uint32_t minutes = (uint32_t)((totalSec % 3600ULL) / 60ULL);
  const uint32_t seconds = (uint32_t)(totalSec % 60ULL);

  char buf[16];
  snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu",
           (unsigned long)hours,
           (unsigned long)minutes,
           (unsigned long)seconds);
  return String(buf);
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

static inline int jsonKeyPos_(const String& raw, const char* key) {
  if (!key || !*key) return -1;
  const String pat = String("\"") + key + String("\"");
  return raw.indexOf(pat);
}

static bool jsonStringField_(const String& raw, const char* key, String& out) {
  const int kp = jsonKeyPos_(raw, key);
  if (kp < 0) return false;
  int p = raw.indexOf(':', kp);
  if (p < 0) return false;
  p++;
  while (p < (int)raw.length() && (raw[p] == ' ' || raw[p] == '\t' || raw[p] == '\n' || raw[p] == '\r')) p++;
  if (p >= (int)raw.length()) return false;
  if (raw[p] == 'n') { out = ""; return true; }
  if (raw[p] != '"') return false;
  p++;
  String v;
  bool esc = false;
  while (p < (int)raw.length()) {
    const char ch = raw[p++];
    if (esc) { v += ch; esc = false; continue; }
    if (ch == '\\') { esc = true; continue; }
    if (ch == '"') break;
    v += ch;
  }
  out = v;
  return true;
}

static bool jsonBoolField_(const String& raw, const char* key, bool& out) {
  const int kp = jsonKeyPos_(raw, key);
  if (kp < 0) return false;
  int p = raw.indexOf(':', kp);
  if (p < 0) return false;
  p++;
  while (p < (int)raw.length() && (raw[p] == ' ' || raw[p] == '\t' || raw[p] == '\n' || raw[p] == '\r')) p++;
  if (raw.startsWith("true", p)) { out = true; return true; }
  if (raw.startsWith("false", p)) { out = false; return true; }
  return false;
}

static bool jsonIntField_(const String& raw, const char* key, int& out) {
  const int kp = jsonKeyPos_(raw, key);
  if (kp < 0) return false;
  int p = raw.indexOf(':', kp);
  if (p < 0) return false;
  p++;
  while (p < (int)raw.length() && (raw[p] == ' ' || raw[p] == '\t' || raw[p] == '\n' || raw[p] == '\r')) p++;
  int s = p;
  if (p < (int)raw.length() && raw[p] == '-') p++;
  while (p < (int)raw.length() && isdigit((unsigned char)raw[p])) p++;
  if (p <= s) return false;
  out = raw.substring(s, p).toInt();
  return true;
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
  config.timeout.socketConnection = CLOUD_SOCKET_TIMEOUT_MS;
  config.timeout.serverResponse = CLOUD_SERVER_RESPONSE_TIMEOUT_MS;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  configureClient_(fbLive);
  configureClient_(fbRead);
  configureClient_(fbHistory);
  configureClient_(fbLog);
  configureClient_(fbCtrl);
  ensureBuffersAllocated_();

  if (!_started) {
    _started = true;
    setenv("TZ", tz, 1);
    tzset();
    configTime(0, 0, ntp1, ntp2);
  }
}

void FirebaseNetwork::updateClock() {
  if (!_started) return;
  const uint64_t epochMs = nowEpochMs();
  if (epochMs == 0) return;

  const uint32_t uptimeMs = millis();
  bool shouldRefreshAnchor = false;

  portENTER_CRITICAL(&_timeAnchorMux);
  const bool hadAnchor = _epochAnchorValid;
  const uint64_t anchoredEpoch = hadAnchor
      ? (_epochAnchorMs + (uint64_t)(uint32_t)(uptimeMs - _uptimeAnchorMs))
      : 0ULL;
  if (!hadAnchor) {
    shouldRefreshAnchor = true;
  } else {
    const int64_t driftMs = (int64_t)epochMs - (int64_t)anchoredEpoch;
    shouldRefreshAnchor = llabs(driftMs) > 2500LL;
  }
  if (shouldRefreshAnchor) {
    _epochAnchorMs = epochMs;
    _uptimeAnchorMs = uptimeMs;
    _epochAnchorValid = true;
  }
  portEXIT_CRITICAL(&_timeAnchorMux);

  _synced = true;
}

uint64_t FirebaseNetwork::nowEpochMs() const {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  if (!timeLooksValid(tv.tv_sec)) return 0;
  return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000ULL);
}

uint64_t FirebaseNetwork::epochForUptimeMs(uint32_t uptimeMs) const {
  uint64_t epochMs = 0;
  portENTER_CRITICAL(&_timeAnchorMux);
  if (_epochAnchorValid) {
    const int32_t deltaMs = (int32_t)(uptimeMs - _uptimeAnchorMs);
    if (deltaMs >= 0) epochMs = _epochAnchorMs + (uint64_t)(uint32_t)deltaMs;
  }
  portEXIT_CRITICAL(&_timeAnchorMux);
  return epochMs;
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
  fbCtrl.stopWiFiClient();
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
  ControlEvent ev;
  ev.kind = kind;
  ev.token = token;
  ev.source = "device";
  ev.relayPulse = false;
  return enqueueControlEvent_(ev);
}

bool FirebaseNetwork::publishRelayPulseEvent(const String& kind, const String& token, const String& source) {
  ControlEvent ev;
  ev.kind = kind;
  ev.token = token;
  ev.source = source;
  ev.relayPulse = true;
  return enqueueControlEvent_(ev);
}

bool FirebaseNetwork::controlWorkPending_() const {
  return _portalRequestPending ||
         _relayOnPending ||
         _relayOffPending ||
         _faultClearPending ||
         _revertFwPending ||
         _otaCheckPending;
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

bool FirebaseNetwork::enqueueControlEvent_(const ControlEvent& ev) {
  if (_controlEventCount >= CONTROL_EVENT_QUEUE_MAX) {
    ControlEvent dropped;
    (void)dequeueControlEvent_(dropped);
  }
  _controlEventQueue[_controlEventTail] = ev;
  _controlEventTail = (uint8_t)((_controlEventTail + 1U) % CONTROL_EVENT_QUEUE_MAX);
  if (_controlEventCount < CONTROL_EVENT_QUEUE_MAX) _controlEventCount++;
  return true;
}

bool FirebaseNetwork::dequeueControlEvent_(ControlEvent& ev) {
  if (_controlEventCount == 0) return false;
  ev = _controlEventQueue[_controlEventHead];
  _controlEventHead = (uint8_t)((_controlEventHead + 1U) % CONTROL_EVENT_QUEUE_MAX);
  _controlEventCount--;
  return true;
}

void FirebaseNetwork::clearControlToken_(const char* path, String& cache, bool& pendingFlag) {
  (void)path;
  pendingFlag = false;
  cache = "";
  // Avoid synchronous token-clear writes from the main loop. The handled-token
  // cache already prevents replaying the same command, and keeping consume()
  // fully local prevents relay/control actions from stalling UI, buzzer, or
  // logging when Firebase is slow.
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

bool FirebaseNetwork::fetchMlControl(bool& enabled, int& dur, int& labelOv, String& sid, String& load, String& deviceFamily, String& deviceName, int& trialNumber, String& divisionTag, String& notes, bool& trustedNormal, String& requestToken) const {
  enabled = _mlEnabledCache;
  dur = _mlDurationCache;
  labelOv = _mlLabelOverrideCache;
  sid = _mlSessionIdCache;
  load = _mlLoadTypeCache;
  deviceFamily = _mlDeviceFamilyCache;
  deviceName = _mlDeviceNameCache;
  trialNumber = _mlTrialNumberCache;
  divisionTag = _mlDivisionTagCache;
  notes = _mlNotesCache;
  trustedNormal = _mlTrustedNormalCache;
  requestToken = _mlRequestTokenCache;
  return true;
}

void FirebaseNetwork::pollControls(bool allowNet, bool portalActive) {
  if (!allowNet || portalActive || !isReady()) return;
  const uint32_t now = millis();
  if ((int32_t)(now - _txBackoffUntilMs) < 0) return;
  const bool heavyCloudLoad = _manualEnabled || _mlUploadActive || _uploadFinalFlush;
  if (heavyCloudLoad && (_pendingLive || _historyCount > 0 || _controlEventCount > 0)) return;

  const uint32_t controlsGap = heavyCloudLoad ? CLOUD_CTRL_READ_GAP_LOGGING_MS : CLOUD_CTRL_READ_GAP_MS;
  const uint32_t mlGap = heavyCloudLoad ? CLOUD_ML_READ_GAP_LOGGING_MS : CLOUD_ML_READ_GAP_MS;
  const bool controlsDue = ((now - _lastControlsReadMs) >= controlsGap);
  const bool mlDue = ((now - _lastMlLogReadMs) >= mlGap);
  if (!controlsDue && !mlDue) return;
  if (!cloudHeapHealthy_()) {
    _txBackoffUntilMs = now + CLOUD_TX_RETRY_MS;
    return;
  }

  const uint32_t controlsAge = now - _lastControlsReadMs;
  const uint32_t mlAge = now - _lastMlLogReadMs;
  const bool preferControls = controlsDue && (!mlDue || controlsAge >= mlAge);

  if (preferControls) {
    _lastControlsReadMs = now;
    if (Firebase.RTDB.getJSON(&fbRead, "/controls")) {
      const String raw = fbRead.payload();
      String token;
      if (jsonStringField_(raw, "open_portal_token", token)) updateControlToken_(token, _portalTokenPrimed, _portalToken, _portalTokenHandled, _portalRequestPending);
      if (jsonStringField_(raw, "relay_on_token", token)) updateControlToken_(token, _relayOnTokenPrimed, _relayOnToken, _relayOnTokenHandled, _relayOnPending);
      if (jsonStringField_(raw, "relay_off_token", token)) updateControlToken_(token, _relayOffTokenPrimed, _relayOffToken, _relayOffTokenHandled, _relayOffPending);
      if (jsonStringField_(raw, "fault_clear_token", token)) updateControlToken_(token, _faultClearTokenPrimed, _faultClearToken, _faultClearTokenHandled, _faultClearPending);
      if (jsonStringField_(raw, "revert_fw_token", token)) updateControlToken_(token, _revertFwTokenPrimed, _revertFwToken, _revertFwTokenHandled, _revertFwPending);
      if (jsonStringField_(raw, "ota_check_token", token)) updateControlToken_(token, _otaCheckTokenPrimed, _otaCheckToken, _otaCheckTokenHandled, _otaCheckPending);
    } else {
      recoverClient_(fbRead, CLOUD_CTRL_FAIL_RETRY_MS);
    }
  } else {
    _lastMlLogReadMs = now;
    if (Firebase.RTDB.getJSON(&fbRead, "/ml_log")) {
      const String raw = fbRead.payload();
      bool b = false;
      int v = 0;
      String s;
      if (jsonBoolField_(raw, "enabled", b)) _mlEnabledCache = b;
      if (jsonIntField_(raw, "duration_s", v)) _mlDurationCache = v;
      if (jsonIntField_(raw, "label_override", v)) _mlLabelOverrideCache = v;
      if (jsonStringField_(raw, "session_id", s)) { _mlSessionIdCache = s; _mlSessionIdCache.trim(); }
      if (jsonStringField_(raw, "load_type", s)) { _mlLoadTypeCache = s; _mlLoadTypeCache.trim(); }
      if (jsonStringField_(raw, "device_family", s)) { _mlDeviceFamilyCache = s; _mlDeviceFamilyCache.trim(); }
      if (jsonStringField_(raw, "device_name", s)) { _mlDeviceNameCache = s; _mlDeviceNameCache.trim(); }
      if (jsonIntField_(raw, "trial_number", v)) _mlTrialNumberCache = v;
      if (jsonStringField_(raw, "division_tag", s)) { _mlDivisionTagCache = s; _mlDivisionTagCache.trim(); }
      if (jsonStringField_(raw, "notes", s)) { _mlNotesCache = s; _mlNotesCache.trim(); }
      if (jsonBoolField_(raw, "trusted_normal_session", b)) _mlTrustedNormalCache = b;
      if (jsonStringField_(raw, "request_token", s)) { _mlRequestTokenCache = s; _mlRequestTokenCache.trim(); }
    } else {
      recoverClient_(fbRead, CLOUD_CTRL_FAIL_RETRY_MS);
    }
  }
}

void FirebaseNetwork::requestLiveUpdate(float v, float c, float apparentPower, float t,
                                        float abs_irms_zscore_vs_baseline, float delta_irms_abs,
                                        float halfcycle_asymmetry, float suspicious_run_energy,
                                        float pulse_count_per_cycle, float zero_dwell_ratio,
                                        float low_current_ratio, float max_low_current_run_ms,
                                        float delta_hf_energy, float delta_flux, float v_sag_pct,
                                        float midband_residual_ratio, float zcv,
                                        float spectral_flux_midhf, float peak_fluct_cv,
                                        float residual_crest_factor, float thd_i,
                                        float hf_energy_delta, float edge_spike_ratio,
                                        uint8_t model_pred,
                                        int8_t contextFamilyCodeRuntime,
                                        float contextFamilyConfidence,
                                        int8_t provisionalContextFamilyCode,
                                        float provisionalContextFamilyConfidence,
                                        bool contextAcquiring,
                                        bool contextLatched,
                                        const String& state,
                                        bool faultLatched, bool webControlsLocked, bool relayLatchedOn, bool relayPulseActive) {
  const bool isNormal = (state == "NORMAL") || (state == "UNPLUGGED");
  const bool stateChanged = (state != _lastSentLiveState);
  const unsigned long now = millis();
  const uint32_t interval = isNormal ? _normalIntervalMs : _faultIntervalMs;
  const bool shouldSend = stateChanged || (now - _lastLiveSend >= interval) || (now - _lastLiveSend >= CLOUD_REFRESH_KEEPALIVE_MS && !isNormal);
  if (!shouldSend && !_pendingLive) return;

  _live.v = (isfinite(v) && v > 0.0f) ? v : 0.0f;
  _live.c = (isfinite(c) && c > 0.0f) ? c : 0.0f;
  _live.apparentPower = (isfinite(apparentPower) && apparentPower > 0.0f) ? apparentPower : 0.0f;
  _live.t = isfinite(t) ? t : 0.0f;
  _live.abs_irms_zscore_vs_baseline =
      tinymlClampFeatureValue(TINYML_FEATURE_ABS_IRMS_ZSCORE_VS_BASELINE, abs_irms_zscore_vs_baseline);
  _live.delta_irms_abs =
      tinymlClampFeatureValue(TINYML_FEATURE_DELTA_IRMS_ABS, delta_irms_abs);
  _live.halfcycle_asymmetry =
      tinymlClampFeatureValue(TINYML_FEATURE_HALFCYCLE_ASYMMETRY, halfcycle_asymmetry);
  _live.suspicious_run_energy =
      tinymlClampFeatureValue(TINYML_FEATURE_SUSPICIOUS_RUN_ENERGY, suspicious_run_energy);
  _live.pulse_count_per_cycle =
      tinymlClampFeatureValue(TINYML_FEATURE_PULSE_COUNT_PER_CYCLE, pulse_count_per_cycle);
  _live.zero_dwell_ratio =
      tinymlClampFeatureValue(TINYML_FEATURE_ZERO_DWELL_RATIO, zero_dwell_ratio);
  _live.low_current_ratio =
      tinymlClampFeatureValue(TINYML_FEATURE_LOW_CURRENT_RATIO, low_current_ratio);
  _live.max_low_current_run_ms =
      tinymlClampFeatureValue(TINYML_FEATURE_MAX_LOW_CURRENT_RUN_MS, max_low_current_run_ms);
  _live.delta_hf_energy =
      tinymlClampFeatureValue(TINYML_FEATURE_DELTA_HF_ENERGY, delta_hf_energy);
  _live.delta_flux =
      tinymlClampFeatureValue(TINYML_FEATURE_DELTA_FLUX, delta_flux);
  _live.v_sag_pct =
      tinymlClampFeatureValue(TINYML_FEATURE_V_SAG_PCT, v_sag_pct);
  _live.midband_residual_ratio =
      tinymlClampFeatureValue(TINYML_FEATURE_MIDBAND_RESIDUAL_RATIO, midband_residual_ratio);
  _live.zcv =
      tinymlClampFeatureValue(TINYML_FEATURE_ZCV, zcv);
  _live.spectral_flux_midhf =
      tinymlClampFeatureValue(TINYML_FEATURE_SPECTRAL_FLUX_MIDHF, spectral_flux_midhf);
  _live.peak_fluct_cv =
      tinymlClampFeatureValue(TINYML_FEATURE_PEAK_FLUCT_CV, peak_fluct_cv);
  _live.residual_crest_factor =
      tinymlClampFeatureValue(TINYML_FEATURE_RESIDUAL_CREST_FACTOR, residual_crest_factor);
  _live.thd_i =
      tinymlClampFeatureValue(TINYML_FEATURE_THD_I, thd_i);
  _live.hf_energy_delta =
      tinymlClampFeatureValue(TINYML_FEATURE_HF_ENERGY_DELTA, hf_energy_delta);
  _live.edge_spike_ratio =
      tinymlClampFeatureValue(TINYML_FEATURE_EDGE_SPIKE_RATIO, edge_spike_ratio);
  _live.model_pred = model_pred;
  _live.contextFamilyCodeRuntime = contextFamilyCodeRuntime;
  _live.contextFamilyConfidence =
      tinymlClampFeatureValue(TINYML_FEATURE_CONTEXT_FAMILY_CONFIDENCE, contextFamilyConfidence);
  _live.provisionalContextFamilyCode = provisionalContextFamilyCode;
  _live.provisionalContextFamilyConfidence =
      tinymlClampFeatureValue(TINYML_FEATURE_CONTEXT_FAMILY_CONFIDENCE, provisionalContextFamilyConfidence);
  _live.contextAcquiring = contextAcquiring;
  _live.contextLatched = contextLatched;
  _live.state = state;
  _live.faultLatched = faultLatched;
  _live.webControlsLocked = webControlsLocked;
  _live.relayLatchedOn = relayLatchedOn;
  _live.relayPulseActive = relayPulseActive;
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
  if (!cloudHeapHealthy_()) {
    _txBackoffUntilMs = millis() + CLOUD_TX_RETRY_MS;
    return false;
  }
  HistoryJob job;
  if (!dequeueHistory_(job)) return false;
  if (!pushHistoryRecord_(job)) {
    enqueueHistory_(job);
    recoverClient_(fbHistory, CLOUD_TX_RETRY_MS);
    return false;
  }
  _lastTxMs = millis();
  return true;
}

bool FirebaseNetwork::serviceControlEvent_() {
  if (_controlEventCount == 0 || !isReady()) return false;
  if (!cloudHeapHealthy_()) {
    _txBackoffUntilMs = millis() + CLOUD_TX_RETRY_MS;
    return false;
  }

  ControlEvent ev;
  if (!dequeueControlEvent_(ev)) return false;

  FirebaseJson json;
  json.set("last_control_ack_kind", ev.kind);
  json.set("last_control_ack_token", ev.token);
  json.set("last_control_ack_server_ts/.sv", "timestamp");
  json.set("last_control_ack_uptime_ms", (int)millis());

  if (ev.relayPulse) {
    json.set("last_relay_pulse_kind", ev.kind);
    json.set("last_relay_pulse_token", ev.token);
    json.set("last_relay_pulse_source", ev.source);
    json.set("last_relay_pulse_server_ts/.sv", "timestamp");
    json.set("last_relay_pulse_uptime_ms", (int)millis());
    if (ev.kind == "relay_on") {
      json.set("relay_latched_on", true);
      json.set("relay_pulse_active", true);
      json.set("load_state", "LOAD ON");
      json.set("device_phase", "LOAD ON");
    } else if (ev.kind == "relay_off") {
      json.set("relay_latched_on", false);
      json.set("relay_pulse_active", true);
      json.set("load_state", "LOAD OFF");
      json.set("device_phase", "LOAD OFF");
    }
  }

  if (!Firebase.RTDB.updateNode(&fbCtrl, "/live_data", &json)) {
    enqueueControlEvent_(ev);
    recoverClient_(fbCtrl, CLOUD_CTRL_FAIL_RETRY_MS);
    return false;
  }

  _lastTxMs = millis();
  return true;
}

bool FirebaseNetwork::serviceLive_() {
  if (!_pendingLive || !isReady()) return false;
  if (!cloudHeapHealthy_()) {
    _txBackoffUntilMs = millis() + CLOUD_TX_RETRY_MS;
    return false;
  }

  const uint64_t epochMs = nowEpochMs();
  const String iso = nowISO8601Ms();
  const bool mainsPresent = (_live.v >= MAINS_PRESENT_ON_V);
  const bool loadDetected = (_live.c >= LOAD_ON_DETECT_A);
  const bool relayClosed = _live.relayLatchedOn;
  const bool compactLive = _manualEnabled || _mlUploadActive || _uploadFinalFlush;
  const String powerCondition = powerConditionForState(_live.state, _live.v);
  const String loadState = relayClosed ? String("LOAD ON") : String("LOAD OFF");
  String devicePhase = loadState;
  if (isTransitionState(_live.state)) devicePhase = _live.state;
  else if (_live.state == "UNPLUGGED") devicePhase = "UNPLUGGED";

  int8_t bestLoadFamilyCode = _live.contextFamilyCodeRuntime;
  if (bestLoadFamilyCode == CONTEXT_FAMILY_UNKNOWN) {
    bestLoadFamilyCode = _live.provisionalContextFamilyCode;
  }
  if (loadDetected) {
    if (!_loadRunActive) {
      _loadRunActive = true;
      _loadRunStartEpochMs = epochMs;
      _loadRunFamilyCode = bestLoadFamilyCode;
    } else if (bestLoadFamilyCode != CONTEXT_FAMILY_UNKNOWN) {
      _loadRunFamilyCode = bestLoadFamilyCode;
    }
  }

  const uint64_t loadOnSinceEpochMs = _loadRunActive ? _loadRunStartEpochMs : 0ULL;
  const uint64_t loadOnDurationMs =
      (_loadRunActive && epochMs > _loadRunStartEpochMs)
          ? (epochMs - _loadRunStartEpochMs)
          : 0ULL;

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
  json.set("abs_irms_zscore_vs_baseline", _live.abs_irms_zscore_vs_baseline);
  json.set("delta_irms_abs", _live.delta_irms_abs);
  json.set("halfcycle_asymmetry", _live.halfcycle_asymmetry);
  json.set("suspicious_run_energy", _live.suspicious_run_energy);
  json.set("pulse_count_per_cycle", _live.pulse_count_per_cycle);
  json.set("zero_dwell_ratio", _live.zero_dwell_ratio);
  json.set("low_current_ratio", _live.low_current_ratio);
  json.set("max_low_current_run_ms", _live.max_low_current_run_ms);
  json.set("delta_hf_energy", _live.delta_hf_energy);
  json.set("delta_flux", _live.delta_flux);
  json.set("midband_residual_ratio", _live.midband_residual_ratio);
  json.set("spectral_flux_midhf", _live.spectral_flux_midhf);
  json.set("residual_crest_factor", _live.residual_crest_factor);
  json.set("thd_i", _live.thd_i);
  json.set("hf_energy_delta", _live.hf_energy_delta);
  json.set("edge_spike_ratio", _live.edge_spike_ratio);
  if (!compactLive) {
    json.set("v_sag_pct", _live.v_sag_pct);
    json.set("zcv", _live.zcv);
    json.set("peak_fluct_cv", _live.peak_fluct_cv);
    json.set("cycle_nmse", _live.cycle_nmse);
  }
  json.set("model_pred", (int)_live.model_pred);
  json.set("context_family_code_runtime", (int)_live.contextFamilyCodeRuntime);
  json.set("context_family_confidence", _live.contextFamilyConfidence);
  json.set("context_family_code_provisional", (int)_live.provisionalContextFamilyCode);
  json.set("context_family_confidence_provisional", _live.provisionalContextFamilyConfidence);
  json.set("context_acquiring", _live.contextAcquiring);
  json.set("context_latched", _live.contextLatched);
  json.set("status", _live.state);
  json.set("device_online", true);
  json.set("mains_present", mainsPresent);
  json.set("power_condition", powerCondition);
  json.set("device_phase", devicePhase);
  json.set("load_state", loadState);
  json.set("load_detected", loadDetected);
  json.set("load_on_since_epoch_ms", (double)loadOnSinceEpochMs);
  json.set("load_on_duration_ms", (double)loadOnDurationMs);
  json.set("load_family_code_live", (int)bestLoadFamilyCode);
  json.set("load_family_label_live", loadFamilyLabel_(bestLoadFamilyCode));
  json.set("fault_latched", _live.faultLatched);
  json.set("web_controls_locked", _live.webControlsLocked);
  json.set("relay_latched_on", _live.relayLatchedOn);
  json.set("relay_pulse_active", _live.relayPulseActive);
  json.set("ml_log_enabled", _manualEnabled);
  json.set("ml_log_session_id", _manual.sessionId);
  json.set("last_transition", _lastTransitionEvent);
  json.set("last_transition_epoch_ms", (double)_lastTransitionEpochMs);
  json.set("ts_epoch_ms", (double)epochMs);
  json.set("ts_iso", iso);
  json.set("uptime_ms", (int)millis());
  json.set("feature_space_version", ARC_RUNTIME_FEATURE_SPACE_VERSION);
  json.set("compact_live_mode", compactLive);
  json.set("server_ts/.sv", "timestamp");

  if (!Firebase.RTDB.updateNode(&fbLive, "/live_data", &json)) {
    recoverClient_(fbLive, CLOUD_TX_RETRY_MS);
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

  if (_loadRunActive && !loadDetected) {
    const uint64_t loadEndEpochMs = epochMs;
    const uint64_t durationMs =
        (loadEndEpochMs > _loadRunStartEpochMs)
            ? (loadEndEpochMs - _loadRunStartEpochMs)
            : 0ULL;
    String loadStatus = "LOAD - ";
    loadStatus += loadFamilyLabel_(_loadRunFamilyCode);
    if (durationMs > 0ULL) {
      loadStatus += " | ";
      loadStatus += formatDurationHms_(durationMs);
    }
    (void)logStatusEvent(loadStatus, _live.v, _live.c, _live.apparentPower, _live.t);
    _loadRunActive = false;
    _loadRunStartEpochMs = 0ULL;
    _loadRunFamilyCode = CONTEXT_FAMILY_UNKNOWN;
  } else if (!_loadRunActive && !loadDetected) {
    _loadRunStartEpochMs = 0ULL;
    _loadRunFamilyCode = CONTEXT_FAMILY_UNKNOWN;
  }

  _haveLastMains = true;
  _lastMainsPresent = mainsPresent;
  return true;
}

const FirebaseNetwork::SessionSpec& FirebaseNetwork::activeSpec() const {
  return _manual;
}

bool FirebaseNetwork::manualRequestBusy_() const {
  return _manualEnabled ||
         _mlUploadActive ||
         _uploadFinalFlush ||
         (_count > 0) ||
         (_sessionStartMs != 0);
}

void FirebaseNetwork::setLogSession(const String& sessionId, const String& loadType, int labelOverride, const String& deviceFamily, const String& deviceName, int trialNumber, const String& divisionTag, const String& notes, bool trustedNormal) {
  if (manualRequestBusy_()) return;

  _manual.sessionId = sanitizeToken(sessionId);
  _manual.loadType  = sanitizeToken(loadType);
  _manual.deviceFamily = sanitizeToken(deviceFamily);
  _manual.deviceName = sanitizeToken(deviceName);
  _manual.trialNumber = (trialNumber > 0) ? trialNumber : 1;
  _manual.divisionTag = sanitizeToken(divisionTag);
  _manual.notes = sanitizeToken(notes);
  _manual.trustedNormalSession = trustedNormal ? 1 : 0;
  _manual.labelOverride = (int8_t)labelOverride;
}


bool FirebaseNetwork::captureUsefulForManual_(const FeatureFrame& f, FaultState st) const {
  const bool usefulCurrent =
      (f.current_valid != 0) ||
      (f.feat_valid != 0) ||
      (f.irms >= CURRENT_DISPLAY_OFF_A);
  const bool mainsPresent = (f.vrms >= MAINS_PRESENT_ON_V);
  const bool keepFault = (st != STATE_NORMAL);
  return keepFault || (f.invalid_loaded_flag != 0) || (mainsPresent && usefulCurrent);
}

float FirebaseNetwork::computeContinuousDurationSeconds_(const Rec* recs, uint16_t count) const {
  if (!recs || count < 2) return 0.0f;

  float diffs[64];
  uint16_t diffCount = 0;
  uint32_t prev = recs[0].frame_start_uptime_ms ? recs[0].frame_start_uptime_ms :
                  (recs[0].frame_end_uptime_ms ? recs[0].frame_end_uptime_ms : recs[0].uptime_ms);
  for (uint16_t i = 1; i < count && diffCount < 64; ++i) {
    const uint32_t cur = recs[i].frame_start_uptime_ms ? recs[i].frame_start_uptime_ms :
                         (recs[i].frame_end_uptime_ms ? recs[i].frame_end_uptime_ms : recs[i].uptime_ms);
    if (cur > prev) diffs[diffCount++] = float(cur - prev) * 0.001f;
    prev = cur;
  }

  float medianStep = 0.0f;
  if (diffCount > 0) {
    for (uint16_t i = 1; i < diffCount; ++i) {
      const float key = diffs[i];
      int j = (int)i - 1;
      while (j >= 0 && diffs[j] > key) { diffs[j + 1] = diffs[j]; --j; }
      diffs[j + 1] = key;
    }
    medianStep = diffs[diffCount / 2];
  }

  if (!(medianStep > 0.0f)) {
    double dtSum = 0.0;
    uint16_t dtCount = 0;
    for (uint16_t i = 0; i < count; ++i) {
      const float dt = recs[i].frame_dt_ms * 0.001f;
      if (isfinite(dt) && dt > 0.0f) { dtSum += dt; dtCount++; }
    }
    if (dtCount > 0) medianStep = float(dtSum / double(dtCount));
  }

  if (!(medianStep > 0.0f)) return 0.0f;
  const float gapThreshold = fmaxf(medianStep * 4.0f, 0.75f);

  float total = 0.0f;
  prev = recs[0].frame_start_uptime_ms ? recs[0].frame_start_uptime_ms :
         (recs[0].frame_end_uptime_ms ? recs[0].frame_end_uptime_ms : recs[0].uptime_ms);
  for (uint16_t i = 1; i < count; ++i) {
    const uint32_t cur = recs[i].frame_start_uptime_ms ? recs[i].frame_start_uptime_ms :
                         (recs[i].frame_end_uptime_ms ? recs[i].frame_end_uptime_ms : recs[i].uptime_ms);
    float dt = (cur > prev) ? float(cur - prev) * 0.001f : medianStep;
    if (!isfinite(dt) || dt <= 0.0f) dt = medianStep;
    if (dt > gapThreshold) dt = medianStep;
    total += dt;
    prev = cur;
  }
  return (total > 0.0f) ? total : 0.0f;
}

void FirebaseNetwork::setLogEnabled(bool en) {
  if (en && manualRequestBusy_()) return;
  if (en && !_manualEnabled) resetLoggerRuntime_();
  _manualEnabled = en;
  if (en) _manualArmMs = millis();
}

void FirebaseNetwork::setLogDurationSeconds(uint16_t sec) {
  if (manualRequestBusy_()) return;
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
  if (_manualEnabled && _mlUploadActive && _uploadFinalFlush) return;
  const SessionSpec& spec = activeSpec();
  if (spec.sessionId.length() < 3 || !_buf || _maxRec == 0 || _count >= _maxRec) return;

  const uint32_t now = millis();
  const bool captureUseful = captureUsefulForManual_(f, st);

  if (_manualEnabled) {
    if (_count == 0) {
      if (!captureUseful) return;
      if (_sessionStartMs == 0) _sessionStartMs = now;
      if (_chunkStartMs == 0) _chunkStartMs = now;
    } else if (_sessionStartMs == 0) {
      _sessionStartMs = now;
    }
  } else {
    if (_sessionStartMs == 0) _sessionStartMs = now;
    const bool usefulCurrent =
        (f.current_valid != 0) ||
        (f.feat_valid != 0) ||
        (f.invalid_loaded_flag != 0) ||
        (f.irms >= CURRENT_DISPLAY_OFF_A);
    const bool mainsPresent = (f.vrms >= MAINS_PRESENT_ON_V);
    const bool keepFault = (st != STATE_NORMAL);
    if (!keepFault && (!mainsPresent || !usefulCurrent) && (f.invalid_loaded_flag == 0)) return;
  }

  const bool manualExpired =
      _manualEnabled &&
      (_sessionStartMs != 0) &&
      ((now - _sessionStartMs) >= ((uint32_t)spec.durationS * 1000UL));
  if (manualExpired) return;

  if (_manualEnabled) _collectionEligible = true;

  if (_count == 0 && _chunkStartMs == 0) _chunkStartMs = now;
  if (_count >= _maxRec) return;

  Rec& r = _buf[_count++];
  r.epoch_ms = f.epoch_ms;
  r.uptime_ms = f.uptime_ms;
  r.frame_start_uptime_ms = f.frame_start_uptime_ms;
  r.frame_end_uptime_ms = f.frame_end_uptime_ms;
  r.feature_compute_end_uptime_ms = f.feature_compute_end_uptime_ms;
  r.log_enqueue_uptime_ms = f.log_enqueue_uptime_ms;
  r.frame_dt_ms = f.frame_dt_ms;
  r.compute_time_ms = f.compute_time_ms;
  r.timing_skew_ms = f.timing_skew_ms;
  r.fft_size = f.fft_size;
  r.hop_samples = f.hop_samples;
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
  r.fs_err_hz = f.fs_err_hz;
  r.suspicious_run_energy = f.suspicious_run_energy;
  r.pulse_count_per_cycle = f.pulse_count_per_cycle;
  r.zero_dwell_ratio = f.zero_dwell_ratio;
  r.low_current_ratio = f.low_current_ratio;
  r.max_low_current_run_ms = f.max_low_current_run_ms;
  r.delta_irms_abs = f.delta_irms_abs;
  r.delta_hf_energy = f.delta_hf_energy;
  r.delta_flux = f.delta_flux;
  r.v_sag_pct = f.v_sag_pct;
  r.halfcycle_asymmetry = f.halfcycle_asymmetry;
  r.v_rms = f.vrms;
  r.i_rms = f.irms;
  r.temp_c = f.temp_c;
  r.queue_drop_count = f.queue_drop_count;
  r.suspicious_run_len = f.suspicious_run_len;
  r.invalid_loaded_run_len = f.invalid_loaded_run_len;
  r.label_arc = (spec.labelOverride == 0 || spec.labelOverride == 1) ? spec.labelOverride : ML_UNKNOWN_LABEL;
  r.restrike_count_short = f.restrike_count_short;
  r.model_pred = f.model_pred;
  r.feat_valid = f.feat_valid;
  r.current_valid = f.current_valid;
  r.fault_state = (uint8_t)st;
  r.sampling_quality_bad = f.sampling_quality_bad;
  r.invalid_loaded_flag = f.invalid_loaded_flag;
  r.invalid_off_flag = f.invalid_off_flag;
  r.relay_blank_active = f.relay_blank_active;
  r.turnon_blank_active = f.turnon_blank_active;
  r.transient_blank_active = f.transient_blank_active;
  const int8_t deviceFamilyCode = (f.device_family_code != CONTEXT_FAMILY_UNKNOWN)
      ? f.device_family_code
      : deviceFamilyCodeFromToken_(spec.deviceFamily);
  r.device_family_code = deviceFamilyCode;
  r.context_family_code_runtime = f.context_family_code_runtime;
  r.context_family_code_provisional = f.context_family_code_provisional;
  r.context_family_confidence = f.context_family_confidence;
  r.context_family_confidence_provisional = f.context_family_confidence_provisional;
  r.context_acquiring = f.context_acquiring;
  r.context_latched = f.context_latched;
  r.arc_counter = _manualEnabled ? 0 : (int16_t)arcCounter;
  r.adc_fs_hz = f.adc_fs_hz;
  r.auto_capture = activeIsAuto() ? 1 : 0;
  r.feature_space_version = ARC_RUNTIME_FEATURE_SPACE_VERSION;
}

void FirebaseNetwork::resetLoggerRuntime_() {
  _manualArmMs = 0;
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
  _collectionEligible = false;
  _collectionGoodFrames = 0;
  _sessionChunkSerial = 0;
}

bool FirebaseNetwork::closeManualSession_(const String& finishedSessionId) {
  _manualEnabled = false;
  if (!isReady()) return false;

  const bool newerManualRequestPending =
      _mlEnabledCache &&
      (_mlSessionIdCache.length() >= 3) &&
      (_mlSessionIdCache != finishedSessionId);

  FirebaseJson mlState;
  if (!newerManualRequestPending) mlState.set("enabled", false);
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
  if (_manualEnabled && _manualArmMs == 0) _manualArmMs = now;

  const bool manualCaptureStarted = _manualEnabled && (_count > 0) && (_sessionStartMs != 0);
  const bool manualTimeUp = manualCaptureStarted && ((now - _sessionStartMs) >= ((uint32_t)spec.durationS * 1000UL));
  const bool manualArmTimeout = _manualEnabled && !manualCaptureStarted && (_manualArmMs != 0) &&
                                ((now - _manualArmMs) >= (((uint32_t)spec.durationS * 1000UL) + ML_LOG_IDLE_ARM_GRACE_MS));
  const bool autoTimeUp   = false;
  const bool chunkTimeUp  = (!_manualEnabled) && (_count > 0) && (_chunkStartMs != 0) && ((now - _chunkStartMs) >= ((uint32_t)ML_LOG_CHUNK_DURATION_S * 1000UL));
  const bool full         = (_maxRec > 0) && (_count >= _maxRec);

  if (_mlUploadActive) return;

  if (_count == 0) {
    if (manualTimeUp || manualArmTimeout) {
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
  if (controlWorkPending_() || _pendingLive || _historyCount > 0) return false;
#if defined(ARDUINO_ARCH_ESP32)
  if (ESP.getFreeHeap() < 32000 || heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) < 24000) {
    _txBackoffUntilMs = millis() + 900UL;
    return false;
  }
#endif

  if (_uploadNextIndex >= _uploadTotalCount) {
    const String finishedSessionId = _uploadSpec.sessionId;
    if (_uploadTotalCount > 0) {
      const Rec& firstUploaded = _buf[0];
      const Rec& lastUploaded = _buf[_uploadTotalCount - 1];
      float wallDurationS = 0.0f;
      if (lastUploaded.frame_end_uptime_ms > firstUploaded.frame_start_uptime_ms) {
        wallDurationS = float(lastUploaded.frame_end_uptime_ms - firstUploaded.frame_start_uptime_ms) / 1000.0f;
      } else if (lastUploaded.frame_start_uptime_ms > firstUploaded.frame_start_uptime_ms) {
        wallDurationS = float(lastUploaded.frame_start_uptime_ms - firstUploaded.frame_start_uptime_ms) / 1000.0f;
      } else if (lastUploaded.frame_end_uptime_ms > firstUploaded.frame_end_uptime_ms) {
        wallDurationS = float(lastUploaded.frame_end_uptime_ms - firstUploaded.frame_end_uptime_ms) / 1000.0f;
      } else if (lastUploaded.uptime_ms > firstUploaded.uptime_ms) {
        wallDurationS = float(lastUploaded.uptime_ms - firstUploaded.uptime_ms) / 1000.0f;
      } else if (lastUploaded.epoch_ms > firstUploaded.epoch_ms) {
        wallDurationS = float(lastUploaded.epoch_ms - firstUploaded.epoch_ms) / 1000.0f;
      }
      const float measuredDurationS = computeContinuousDurationSeconds_(_buf, _uploadTotalCount);
      const float preferredDurationS = (wallDurationS > 0.0f) ? wallDurationS : measuredDurationS;
      float fsSum = 0.0f;
      uint32_t fsCount = 0;
      for (uint16_t i = 0; i < _uploadTotalCount; ++i) {
        const float fs = _buf[i].adc_fs_hz;
        if (isfinite(fs) && fs > 0.0f) { fsSum += fs; fsCount++; }
      }
      FirebaseJson sessMeta;
      sessMeta.set("row_count", (int)_uploadTotalCount);
      sessMeta.set("first_epoch_ms", (double)firstUploaded.epoch_ms);
      sessMeta.set("last_epoch_ms", (double)lastUploaded.epoch_ms);
      sessMeta.set("capture_start_ms", (double)firstUploaded.epoch_ms);
      sessMeta.set("capture_end_ms", (double)lastUploaded.epoch_ms);
      sessMeta.set("first_frame_start_uptime_ms", (int)firstUploaded.frame_start_uptime_ms);
      sessMeta.set("last_frame_end_uptime_ms", (int)lastUploaded.frame_end_uptime_ms);
      sessMeta.set("device_family", _uploadSpec.deviceFamily);
      sessMeta.set("device_name", _uploadSpec.deviceName);
      sessMeta.set("trial_number", _uploadSpec.trialNumber);
      sessMeta.set("division_tag", _uploadSpec.divisionTag);
      sessMeta.set("notes", _uploadSpec.notes);
      sessMeta.set("trusted_normal_session", (int)_uploadSpec.trustedNormalSession);
      if (preferredDurationS > 0.0f) sessMeta.set("source_duration_s", preferredDurationS);
      if (measuredDurationS > 0.0f) sessMeta.set("source_continuous_duration_s", measuredDurationS);
      if (wallDurationS > 0.0f) sessMeta.set("source_wall_duration_s", wallDurationS);
      if (fsCount > 0) sessMeta.set("source_sample_rate_hz", fsSum / float(fsCount));
      if (_uploadTotalCount > 1) {
        const float meanFrameHz = (preferredDurationS > 0.0f) ? (float(_uploadTotalCount - 1U) / preferredDurationS) :
                                 ((measuredDurationS > 0.0f) ? (float(_uploadTotalCount - 1U) / measuredDurationS) : 0.0f);
        if (meanFrameHz > 0.0f) sessMeta.set("feature_frame_rate_hz", meanFrameHz);
      }
      String sessPath = "/ml_sessions/";
      sessPath += finishedSessionId;
      (void)Firebase.RTDB.updateNode(&fbLog, sessPath.c_str(), &sessMeta);
    }
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
  const char* header = "abs_irms_zscore_vs_baseline,delta_irms_abs,halfcycle_asymmetry,suspicious_run_energy,pulse_count_per_cycle,zero_dwell_ratio,low_current_ratio,max_low_current_run_ms,delta_hf_energy,delta_flux,midband_residual_ratio,zcv,spectral_flux_midhf,peak_fluct_cv,residual_crest_factor,thd_i,hf_energy_delta,edge_spike_ratio,v_sag_pct,cycle_nmse,fs_err_hz,v_rms,i_rms,temp_c,label_arc,device_family,device_name,trial_number,division_tag,notes,trusted_normal_session,load_type,session_id,epoch_ms,uptime_ms,frame_start_uptime_ms,frame_end_uptime_ms,feature_compute_end_uptime_ms,log_enqueue_uptime_ms,frame_dt_ms,compute_time_ms,timing_skew_ms,fft_size,hop_samples,queue_drop_count,suspicious_run_len,invalid_loaded_run_len,restrike_count_short,model_pred,feat_valid,current_valid,sampling_quality_bad,invalid_loaded_flag,invalid_off_flag,relay_blank_active,turnon_blank_active,transient_blank_active,device_family_code,context_family_code_runtime,context_family_code_provisional,context_family_confidence,context_family_confidence_provisional,context_acquiring,context_latched,fault_state,arc_counter,adc_fs_hz,auto_capture,feature_space_version\n";

  String csv;
  csv.reserve((i1 - i0) * 520 + 512);
  csv += header;
  for (uint16_t i = i0; i < i1; ++i) {
    const Rec& r = _buf[i];
    csv += String(r.abs_irms_zscore_vs_baseline, 6); csv += ",";
    csv += String(r.delta_irms_abs, 6);              csv += ",";
    csv += String(r.halfcycle_asymmetry, 6);         csv += ",";
    csv += String(r.suspicious_run_energy, 6);       csv += ",";
    csv += String(r.pulse_count_per_cycle, 6);       csv += ",";
    csv += String(r.zero_dwell_ratio, 6);            csv += ",";
    csv += String(r.low_current_ratio, 6);           csv += ",";
    csv += String(r.max_low_current_run_ms, 6);      csv += ",";
    csv += String(r.delta_hf_energy, 6);             csv += ",";
    csv += String(r.delta_flux, 6);                  csv += ",";
    csv += String(r.midband_residual_ratio, 6);      csv += ",";
    csv += String(r.zcv, 6);                         csv += ",";
    csv += String(r.spectral_flux_midhf, 6);         csv += ",";
    csv += String(r.peak_fluct_cv, 6);               csv += ",";
    csv += String(r.residual_crest_factor, 6);       csv += ",";
    csv += String(r.thd_i, 4);                       csv += ",";
    csv += String(r.hf_energy_delta, 6);             csv += ",";
    csv += String(r.edge_spike_ratio, 6);            csv += ",";
    csv += String(r.v_sag_pct, 6);                   csv += ",";
    csv += String(r.cycle_nmse, 6);                  csv += ",";
    csv += String(r.fs_err_hz, 3);                   csv += ",";
    csv += String(r.v_rms, 3);                       csv += ",";
    csv += String(r.i_rms, 6);                       csv += ",";
    csv += String(r.temp_c, 3);                      csv += ",";
    csv += String((int)r.label_arc);           csv += ",";
    csv += _uploadSpec.deviceFamily;           csv += ",";
    csv += _uploadSpec.deviceName;             csv += ",";
    csv += String(_uploadSpec.trialNumber);    csv += ",";
    csv += _uploadSpec.divisionTag;            csv += ",";
    csv += _uploadSpec.notes;                  csv += ",";
    csv += String((int)_uploadSpec.trustedNormalSession); csv += ",";
    csv += _uploadSpec.loadType;               csv += ",";
    csv += _uploadSpec.sessionId;              csv += ",";
    csv += String((unsigned long long)r.epoch_ms); csv += ",";
    csv += String((unsigned long)r.uptime_ms); csv += ",";
    csv += String((unsigned long)r.frame_start_uptime_ms); csv += ",";
    csv += String((unsigned long)r.frame_end_uptime_ms); csv += ",";
    csv += String((unsigned long)r.feature_compute_end_uptime_ms); csv += ",";
    csv += String((unsigned long)r.log_enqueue_uptime_ms); csv += ",";
    csv += String(r.frame_dt_ms, 3);           csv += ",";
    csv += String(r.compute_time_ms, 3);       csv += ",";
    csv += String(r.timing_skew_ms, 3);        csv += ",";
    csv += String((unsigned int)r.fft_size);   csv += ",";
    csv += String((unsigned int)r.hop_samples); csv += ",";
    csv += String((unsigned long)r.queue_drop_count); csv += ",";
    csv += String((int)r.suspicious_run_len);  csv += ",";
    csv += String((int)r.invalid_loaded_run_len); csv += ",";
    csv += String((int)r.restrike_count_short); csv += ",";
    csv += String((int)r.model_pred);          csv += ",";
    csv += String((int)r.feat_valid);          csv += ",";
    csv += String((int)r.current_valid);       csv += ",";
    csv += String((int)r.sampling_quality_bad); csv += ",";
    csv += String((int)r.invalid_loaded_flag); csv += ",";
    csv += String((int)r.invalid_off_flag);    csv += ",";
    csv += String((int)r.relay_blank_active);  csv += ",";
    csv += String((int)r.turnon_blank_active); csv += ",";
    csv += String((int)r.transient_blank_active); csv += ",";
    csv += String((int)r.device_family_code);  csv += ",";
    csv += String((int)r.context_family_code_runtime); csv += ",";
    csv += String((int)r.context_family_code_provisional); csv += ",";
    csv += String(r.context_family_confidence, 4); csv += ",";
    csv += String(r.context_family_confidence_provisional, 4); csv += ",";
    csv += String((int)r.context_acquiring);   csv += ",";
    csv += String((int)r.context_latched);     csv += ",";
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
  const Rec& sessionFirstRec = _buf[0];
  const Rec& sessionLastRec = _buf[_uploadTotalCount - 1];
  float sessionWallDurationS = 0.0f;
  if (sessionLastRec.frame_end_uptime_ms > sessionFirstRec.frame_start_uptime_ms) {
    sessionWallDurationS = float(sessionLastRec.frame_end_uptime_ms - sessionFirstRec.frame_start_uptime_ms) / 1000.0f;
  } else if (sessionLastRec.frame_start_uptime_ms > sessionFirstRec.frame_start_uptime_ms) {
    sessionWallDurationS = float(sessionLastRec.frame_start_uptime_ms - sessionFirstRec.frame_start_uptime_ms) / 1000.0f;
  } else if (sessionLastRec.epoch_ms > sessionFirstRec.epoch_ms) {
    sessionWallDurationS = float(sessionLastRec.epoch_ms - sessionFirstRec.epoch_ms) / 1000.0f;
  }
  const float sessionContinuousDurationS = computeContinuousDurationSeconds_(_buf, _uploadTotalCount);
  const float sessionPreferredDurationS = (sessionWallDurationS > 0.0f) ? sessionWallDurationS : sessionContinuousDurationS;

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
  json.set("first_frame_start_uptime_ms", (int)firstRec.frame_start_uptime_ms);
  json.set("last_frame_end_uptime_ms", (int)lastRec.frame_end_uptime_ms);
  if (sessionPreferredDurationS > 0.0f) json.set("source_duration_s", sessionPreferredDurationS);
  if (sessionWallDurationS > 0.0f) json.set("source_wall_duration_s", sessionWallDurationS);
  if (sessionContinuousDurationS > 0.0f) json.set("source_continuous_duration_s", sessionContinuousDurationS);
  json.set("final", _uploadFinalFlush && (i1 >= _uploadTotalCount));
  json.set("csv", csv);
  json.set("meta/session_id", _uploadSpec.sessionId);
  json.set("meta/load_type", _uploadSpec.loadType);
  json.set("meta/device_family", _uploadSpec.deviceFamily);
  json.set("meta/device_name", _uploadSpec.deviceName);
  json.set("meta/trial_number", _uploadSpec.trialNumber);
  json.set("meta/division_tag", _uploadSpec.divisionTag);
  json.set("meta/notes", _uploadSpec.notes);
  json.set("meta/trusted_normal_session", (int)_uploadSpec.trustedNormalSession);
  json.set("meta/label_override", (int)_uploadSpec.labelOverride);
  json.set("meta/duration_s", (int)_uploadSpec.durationS);
  if (sessionPreferredDurationS > 0.0f) json.set("meta/source_duration_s", sessionPreferredDurationS);
  if (sessionWallDurationS > 0.0f) json.set("meta/source_wall_duration_s", sessionWallDurationS);
  if (sessionContinuousDurationS > 0.0f) json.set("meta/source_continuous_duration_s", sessionContinuousDurationS);
  json.set("meta/auto_capture", _uploadAuto);
  json.set("meta/feature_order", "abs_irms_zscore_vs_baseline,delta_irms_abs,halfcycle_asymmetry,suspicious_run_energy,pulse_count_per_cycle,zero_dwell_ratio,low_current_ratio,max_low_current_run_ms,delta_hf_energy,delta_flux,midband_residual_ratio,zcv,spectral_flux_midhf,peak_fluct_cv,residual_crest_factor,thd_i,hf_energy_delta,edge_spike_ratio,v_sag_pct,cycle_nmse");
  json.set("meta/pwa_feature_order", "pulse_count_per_cycle,max_low_current_run_ms,zero_dwell_ratio,low_current_ratio,thd_i,spectral_flux_midhf,hf_energy_delta,residual_crest_factor,peak_fluct_cv,zcv,delta_irms_abs,midband_residual_ratio,edge_spike_ratio");
  json.set("meta/fft_size", (int)ARC_RUNTIME_FRAME_SAMPLES);
  json.set("meta/hop_samples", (int)ARC_RUNTIME_HOP_SAMPLES);
  json.set("meta/feature_emit_every_hops", (int)ARC_RUNTIME_EMIT_EVERY_HOPS);

  if (!Firebase.RTDB.pushJSON(&fbLog, path.c_str(), &json)) {
    recoverClient_(fbLog, CLOUD_TX_RETRY_MS);
    return false;
  }

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

  if (serviceControlEvent_()) return;
  if (serviceLive_()) return;
  if (serviceHistory_()) return;
  if (!_suspendMlUpload) {
    const uint32_t uploadGapMs = (_manualEnabled || _mlUploadActive || _uploadFinalFlush) ? 700UL : 0UL;
    if (uploadGapMs == 0UL || (now - _lastMlUploadAttemptMs) >= uploadGapMs) {
      if (serviceMlUpload_()) {
        _lastMlUploadAttemptMs = millis();
        return;
      }
    }
  }
}
