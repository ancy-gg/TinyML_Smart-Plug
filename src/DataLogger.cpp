#include "DataLogger.h"
#include "CloudHandler.h"

void DataLogger::begin(CloudHandler* cloud) { _cloud = cloud; }

String DataLogger::sanitizeToken(const String& s) {
  String o = s;
  o.trim();
  if (o.length() == 0) return "unknown";
  o.replace(",", "_");
  o.replace(" ", "_");
  o.replace("/", "_");
  o.replace("\\", "_");
  if (o.length() > 32) o = o.substring(0, 32);
  return o;
}

void DataLogger::setSession(const String& sessionId, const String& loadType, int labelOverride) {
#if ENABLE_ML_LOGGER
  _sessionId = sanitizeToken(sessionId);
  _loadType  = sanitizeToken(loadType);
  _labelOverride = (int8_t)labelOverride;
#else
  (void)sessionId; (void)loadType; (void)labelOverride;
#endif
}

void DataLogger::setEnabled(bool en) { _enabled = en; }

void DataLogger::setDurationSeconds(uint16_t sec) {
  if (sec < 5) sec = 5;
  if (sec > 60) sec = 60;
  _durationS = sec;
}

void DataLogger::ingest(const FeatureFrame& f, FaultState st, int arcCounter) {
#if ENABLE_ML_LOGGER
  if (!_enabled) return;
  if (_sessionId.length() < 3) return;
  if (_count >= MAX_REC) return;

  if (_count == 0) _chunkStartMs = millis();

  Rec& r = _buf[_count++];
  r.epoch_ms = f.epoch_ms;

  r.spectral_entropy = f.entropy;
  r.thd_pct          = f.thd_pct;
  r.zcv              = f.zcv_ms;
  r.v_rms            = f.vrms;
  r.i_rms            = f.irms;
  r.temp_c           = f.temp_c;

  // labeling: allow website override for clean datasets
  uint8_t lab = 0;
  if (_labelOverride == 0) lab = 0;
  else if (_labelOverride == 1) lab = 1;
  else lab = (st == STATE_ARCING) ? 1 : 0;
  r.label_arc = lab;

  r.model_pred = f.model_pred;
  r.state      = (uint8_t)st;
  r.arc_cnt    = (uint8_t)arcCounter;
#else
  (void)f; (void)st; (void)arcCounter;
#endif
}

void DataLogger::loop() {
#if ENABLE_ML_LOGGER
  if (!_cloud || !_cloud->isReady()) return;

  // flush remaining on disable
  if (_wasEnabled && !_enabled) {
    if (_count > 0) {
      flushToFirebase(true);
      _count = 0;
      _chunkStartMs = 0;
    }
  }
  _wasEnabled = _enabled;

  if (!_enabled) return;
  if (_sessionId.length() < 3) return;
  if (_count == 0) return;

  const uint32_t now = millis();
  const uint32_t elapsed = now - _chunkStartMs;

  const bool timeUp = (elapsed >= (uint32_t)_durationS * 1000UL);
  const bool full   = (_count >= MAX_REC);

  if (!timeUp && !full) return;

  if (_lastFlushAttemptMs && (now - _lastFlushAttemptMs) < 2000) return;
  _lastFlushAttemptMs = now;

  if (flushToFirebase(false)) {
    _count = 0;
    _chunkStartMs = 0;
    _lastFlushAttemptMs = 0;
  }
#endif
}

#if ENABLE_ML_LOGGER
bool DataLogger::flushToFirebase(bool finalFlush) {
  if (!_cloud || !_cloud->isReady()) return false;
  if (_sessionId.length() < 3) return false;

  // CSV matches python + adds load_type/session_id/epoch_ms
  String csv;
  csv.reserve(_count * 90 + 200);
  csv += "spectral_entropy,thd_pct,zcv,v_rms,i_rms,temp_c,label_arc,load_type,session_id,epoch_ms\n";

  for (uint16_t i = 0; i < _count; i++) {
    const Rec& r = _buf[i];
    csv += String(r.spectral_entropy, 6); csv += ",";
    csv += String(r.thd_pct, 3);          csv += ",";
    csv += String(r.zcv, 6);              csv += ",";
    csv += String(r.v_rms, 3);            csv += ",";
    csv += String(r.i_rms, 6);            csv += ",";
    csv += String(r.temp_c, 3);           csv += ",";
    csv += String((int)r.label_arc);      csv += ",";
    csv += _loadType;                     csv += ",";
    csv += _sessionId;                    csv += ",";
    csv += String((unsigned long long)r.epoch_ms);
    csv += "\n";
  }

  FirebaseJson json;
  json.set("created_at/.sv", "timestamp");
  json.set("count", (int)_count);
  json.set("final", finalFlush);
  json.set("csv", csv);

  json.set("meta/session_id", _sessionId);
  json.set("meta/load_type", _loadType);
  json.set("meta/label_override", (int)_labelOverride);
  json.set("meta/duration_s", (int)_durationS);

  // IMPORTANT: write under the session node
  String path = "/ml_logs/"; 
  path += _sessionId;
  const bool ok = _cloud->pushJSON(path.c_str(), json);

  if (!ok) Serial.println("[ML_LOG] pushJSON failed");
  else     Serial.printf("[ML_LOG] Uploaded %u rows to %s\n", _count, path.c_str());

  return ok;
}
#endif