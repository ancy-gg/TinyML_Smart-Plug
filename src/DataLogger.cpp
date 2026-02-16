#include "DataLogger.h"
#include "CloudHandler.h"

void DataLogger::begin(CloudHandler* cloud) {
  _cloud = cloud;
}

void DataLogger::setEnabled(bool en) {
  _enabled = en;
#if ENABLE_ML_LOGGER
  if (!en) _count = 0;
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
  if (_count >= _targetCount) return;

  Rec& r = _buf[_count++];

  r.epoch_ms = f.epoch_ms;

  // ---- CSV fields to match training script exactly ----
  r.spectral_entropy = f.entropy;
  r.thd_pct          = f.thd_pct;
  r.zcv              = f.zcv_ms;     // stored as ms, but column header MUST be "zcv"
  r.v_rms            = f.vrms;
  r.i_rms            = f.irms;
  r.temp_c           = f.temp_c;

  // Label strategy:
  // Use system-level decision (latched arc state) as your dataset label.
  // This is better than using model_pred as "label" (circular training).
  r.label_arc        = (st == STATE_ARCING) ? 1 : 0;

  // ---- extra debug (not used by training script) ----
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

  if (_count >= _targetCount) {
    flushToFirebase();
    _count = 0;
  }
#endif
}

#if ENABLE_ML_LOGGER
bool DataLogger::flushToFirebase() {
  if (!_cloud || !_cloud->isReady()) return false;

  // Training-compatible CSV blob
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

  // Put meta separately so CSV stays exactly like your Python expects
  FirebaseJson json;
  json.set("count", (int)_count);
  json.set("created_at/.sv", "timestamp");

  // CSV for training
  json.set("csv", csv);

  // Helpful meta for debugging / later filtering
  // (kept outside CSV so it won't break your training script)
  json.set("meta/rate_hz", (int)ML_LOG_RATE_HZ);
  json.set("meta/duration_s", (int)_durationS);

  // Optional: include a compact meta CSV for tracing
  // epoch_ms, state, arc_cnt, model_pred
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

  return _cloud->pushJSON("/ml_logs", json);
}
#endif