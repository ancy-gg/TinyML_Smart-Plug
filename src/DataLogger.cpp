#include "DataLogger.h"
#include "CloudHandler.h"

void DataLogger::begin(CloudHandler* cloud) {
  _cloud = cloud;
}

void DataLogger::setEnabled(bool en) {
  _enabled = en;
#if ENABLE_ML_LOGGER
  if (!en) {
    _count = 0;
    _startMs = 0;
    _lastFlushAttemptMs = 0;
  }
#endif
}

void DataLogger::setDurationSeconds(uint16_t sec) {
  if (sec < 5) sec = 5;
  if (sec > 14) sec = 14;

  _durationS = sec;
  _targetCount = (uint16_t)(_durationS * ML_LOG_RATE_HZ);
#if ENABLE_ML_LOGGER
  if (_targetCount > MAX_REC) _targetCount = MAX_REC;
#endif
}

void DataLogger::ingest(const FeatureFrame& f, FaultState st, int arcCounter) {
#if ENABLE_ML_LOGGER
  if (!_enabled) return;
  if (_count >= MAX_REC) return;

  if (_count == 0) {
    _startMs = millis();
  }

  Rec& r = _buf[_count++];
  r.epoch_ms = f.epoch_ms;

  r.spectral_entropy = f.entropy;
  r.thd_pct          = f.thd_pct;
  r.zcv              = f.zcv_ms;
  r.v_rms            = f.vrms;
  r.i_rms            = f.irms;
  r.temp_c           = f.temp_c;

  r.label_arc        = (st == STATE_ARCING) ? 1 : 0;

  r.model_pred = f.model_pred;
  r.state      = (uint8_t)st;
  r.arc_cnt    = (uint8_t)arcCounter;
#else
  (void)f; (void)st; (void)arcCounter;
#endif
}

void DataLogger::loop() {
#if ENABLE_ML_LOGGER
  if (!_enabled) return;
  if (_count == 0) return;

  const uint32_t now = millis();
  const uint32_t elapsedMs = now - _startMs;

  const bool timeUp = (elapsedMs >= (uint32_t)_durationS * 1000UL);
  const bool countUp = (_count >= _targetCount);

  if (!timeUp && !countUp) return;

  // retry flush every 2 seconds if Firebase not ready / push fails
  if (_lastFlushAttemptMs && (now - _lastFlushAttemptMs) < 2000) return;
  _lastFlushAttemptMs = now;

  if (flushToFirebase(elapsedMs)) {
    _count = 0;
    _startMs = 0;
    _lastFlushAttemptMs = 0;
  }
#endif
}

#if ENABLE_ML_LOGGER
bool DataLogger::flushToFirebase(uint32_t elapsedMs) {
  if (!_cloud || !_cloud->isReady()) return false;

  String csv;
  csv.reserve(_count * 64 + 120);
  csv += "spectral_entropy,thd_pct,zcv,v_rms,i_rms,temp_c,label_arc\n";

  for (uint16_t i = 0; i < _count; i++) {
    const Rec& r = _buf[i];
    csv += String(r.spectral_entropy, 6); csv += ",";
    csv += String(r.thd_pct, 3);          csv += ",";
    csv += String(r.zcv, 6);              csv += ",";
    csv += String(r.v_rms, 3);            csv += ",";
    csv += String(r.i_rms, 6);            csv += ",";
    csv += String(r.temp_c, 3);           csv += ",";
    csv += String((int)r.label_arc);
    csv += "\n";
  }

  FirebaseJson json;
  json.set("count", (int)_count);
  json.set("created_at/.sv", "timestamp");
  json.set("csv", csv);

  const float elapsedS = (elapsedMs > 0) ? (elapsedMs / 1000.0f) : 1.0f;
  json.set("meta/elapsed_s", elapsedS);
  json.set("meta/actual_hz", (float)_count / elapsedS);
  json.set("meta/target_rate_hz", (int)ML_LOG_RATE_HZ);
  json.set("meta/duration_s", (int)_durationS);

  String metaCsv;
  metaCsv.reserve(_count * 40 + 80);
  metaCsv += "epoch_ms,state,arc_cnt,model_pred\n";
  for (uint16_t i = 0; i < _count; i++) {
    const Rec& r = _buf[i];
    metaCsv += String((unsigned long long)r.epoch_ms); metaCsv += ",";
    metaCsv += String((int)r.state); metaCsv += ",";
    metaCsv += String((int)r.arc_cnt); metaCsv += ",";
    metaCsv += String((int)r.model_pred);
    metaCsv += "\n";
  }
  json.set("meta/meta_csv", metaCsv);

  const bool ok = _cloud->pushJSON("/ml_logs", json);
  if (!ok) Serial.println("[ML_LOG] pushJSON failed (will retry)");
  else     Serial.printf("[ML_LOG] Uploaded %u rows in %.2fs\n", _count, elapsedS);

  return ok;
}
#endif