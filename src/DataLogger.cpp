#include "DataLogger.h"
#include "FirebaseHandler.h"

void DataLogger::begin(FirebaseHandler* cloud) {
  _cloud = cloud;
  resetRuntimeState_();
}

String DataLogger::sanitizeToken(const String& s) {
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

const DataLogger::SessionSpec& DataLogger::activeSpec() const {
  return _manualEnabled ? _manual : _auto;
}

void DataLogger::setSession(const String& sessionId, const String& loadType, int labelOverride) {
  _manual.sessionId = sanitizeToken(sessionId);
  _manual.loadType  = sanitizeToken(loadType);
  _manual.labelOverride = (int8_t)labelOverride;
}

void DataLogger::setEnabled(bool en) {
  if (en && !_manualEnabled) resetRuntimeState_();
  _manualEnabled = en;
}

void DataLogger::setDurationSeconds(uint16_t sec) {
  if (sec < ML_LOG_MIN_DURATION_S) sec = ML_LOG_MIN_DURATION_S;
  if (sec > ML_LOG_MAX_DURATION_S) sec = ML_LOG_MAX_DURATION_S;
  _manual.durationS = sec;
}

bool DataLogger::startAutoCapture(const String& reason, uint16_t sec) {
  if (_manualEnabled || _autoEnabled) return false;
  if (sec < ML_LOG_AUTO_MIN_DURATION_S) sec = ML_LOG_AUTO_MIN_DURATION_S;
  if (sec > ML_LOG_AUTO_MAX_DURATION_S) sec = ML_LOG_AUTO_MAX_DURATION_S;
  _auto.sessionId = sanitizeToken(String("auto_") + reason + String("_") + String((unsigned long)millis()));
  _auto.loadType = sanitizeToken(String("auto_") + reason);
  _auto.labelOverride = ML_UNKNOWN_LABEL;
  _auto.durationS = sec;
  _autoEnabled = true;
  resetRuntimeState_();
  return true;
}

void DataLogger::stopAutoCapture() {
  _autoEnabled = false;
  resetRuntimeState_();
}

void DataLogger::ingest(const FeatureFrame& f, FaultState st, int arcCounter) {
  if (!enabled()) return;
  const SessionSpec& spec = activeSpec();
  if (spec.sessionId.length() < 3 || _count >= MAX_REC) return;

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

void DataLogger::loop() {
  if (!_cloud || !_cloud->isReady()) return;

  if (_wasEnabled && !enabled()) {
    if (_count > 0) (void)flushToFirebase(true);
    resetRuntimeState_();
  }
  _wasEnabled = enabled();
  if (!enabled()) return;

  const SessionSpec& spec = activeSpec();
  if (spec.sessionId.length() < 3) return;

  const uint32_t now = millis();
  if (_sessionStartMs == 0) _sessionStartMs = now;

  const bool manualTimeUp = (_manualEnabled && !_autoEnabled) && ((now - _sessionStartMs) >= ((uint32_t)spec.durationS * 1000UL));
  const bool autoTimeUp   = activeIsAuto() && ((now - _sessionStartMs) >= ((uint32_t)spec.durationS * 1000UL));
  const bool chunkTimeUp  = (_count > 0) && (_chunkStartMs != 0) && ((now - _chunkStartMs) >= ((uint32_t)ML_LOG_CHUNK_DURATION_S * 1000UL));
  const bool full         = (_count >= MAX_REC);

  if (_count == 0) {
    if (manualTimeUp) {
      closeManualSession_(spec.sessionId);
      resetRuntimeState_();
    } else if (autoTimeUp) {
      _autoEnabled = false;
      resetRuntimeState_();
    }
    return;
  }

  if (!manualTimeUp && !autoTimeUp && !chunkTimeUp && !full) return;
  if (_lastFlushAttemptMs && (now - _lastFlushAttemptMs) < 2000UL) return;
  _lastFlushAttemptMs = now;

  const bool finalFlush = manualTimeUp || autoTimeUp;
  if (flushToFirebase(finalFlush)) {
    const String finishedSessionId = spec.sessionId;
    _count = 0;
    _chunkStartMs = 0;
    _lastFlushAttemptMs = 0;
    if (autoTimeUp) {
      _autoEnabled = false;
      _sessionStartMs = 0;
    } else if (manualTimeUp) {
      closeManualSession_(finishedSessionId);
      _sessionStartMs = 0;
    }
  }
}

void DataLogger::resetRuntimeState_() {
  _sessionStartMs = 0;
  _chunkStartMs = 0;
  _lastFlushAttemptMs = 0;
  _count = 0;
}

void DataLogger::closeManualSession_(const String& finishedSessionId) {
  _manualEnabled = false;
  FirebaseJson mlState;
  mlState.set("enabled", false);
  mlState.set("last_completed_session_id", finishedSessionId);
  mlState.set("last_completed_at/.sv", "timestamp");
  (void)_cloud->updateJSON("/ml_log", mlState);

  String sessPath = "/ml_sessions/";
  sessPath += finishedSessionId;
  FirebaseJson sessMeta;
  sessMeta.set("end_ms/.sv", "timestamp");
  sessMeta.set("closed_by_device", true);
  (void)_cloud->updateJSON(sessPath.c_str(), sessMeta);
}

bool DataLogger::flushToFirebase(bool finalFlush) {
  if (!_cloud || !_cloud->isReady()) return false;
  const SessionSpec& spec = activeSpec();
  if (spec.sessionId.length() < 3) return false;

  static constexpr uint16_t ROWS_PER_CHUNK = 80;
  const uint16_t totalCount = _count;
  const uint16_t chunkCount = (totalCount + ROWS_PER_CHUNK - 1U) / ROWS_PER_CHUNK;
  const char* header = "cycle_nmse,zcv,zc_dwell_ratio,pulse_count_per_cycle,peak_fluct_cv,midband_residual_rms,hf_band_energy_ratio,wpe_entropy,spec_entropy,thd_i,v_rms,i_rms,temp_c,label_arc,load_type,session_id,epoch_ms,model_pred,feat_valid,current_valid,fault_state,arc_counter,adc_fs_hz,auto_capture\n";

  String path = "/ml_logs/";
  path += spec.sessionId;

  for (uint16_t chunk = 0; chunk < chunkCount; ++chunk) {
    const uint16_t i0 = chunk * ROWS_PER_CHUNK;
    const uint16_t i1 = ((uint16_t)(i0 + ROWS_PER_CHUNK) < totalCount) ? (uint16_t)(i0 + ROWS_PER_CHUNK) : totalCount;

    String csv;
    csv.reserve((i1 - i0) * 300 + 360);
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
      csv += spec.loadType;                      csv += ",";
      csv += spec.sessionId;                     csv += ",";
      csv += String((unsigned long long)r.epoch_ms); csv += ",";
      csv += String((int)r.model_pred);          csv += ",";
      csv += String((int)r.feat_valid);          csv += ",";
      csv += String((int)r.current_valid);       csv += ",";
      csv += String((int)r.fault_state);         csv += ",";
      csv += String((int)r.arc_counter);         csv += ",";
      csv += String(r.adc_fs_hz, 2);             csv += ",";
      csv += String((int)r.auto_capture);        csv += "\n";
    }

    FirebaseJson json;
    json.set("created_at/.sv", "timestamp");
    json.set("count", (int)(i1 - i0));
    json.set("total_count", (int)totalCount);
    json.set("chunk_index", (int)chunk);
    json.set("chunk_count", (int)chunkCount);
    json.set("final", finalFlush && (chunk == (chunkCount - 1U)));
    json.set("csv", csv);
    json.set("meta/session_id", spec.sessionId);
    json.set("meta/load_type", spec.loadType);
    json.set("meta/label_override", (int)spec.labelOverride);
    json.set("meta/duration_s", (int)spec.durationS);
    json.set("meta/auto_capture", activeIsAuto());
    json.set("meta/feature_order", "cycle_nmse,zcv,zc_dwell_ratio,pulse_count_per_cycle,peak_fluct_cv,midband_residual_rms,hf_band_energy_ratio,wpe_entropy,spec_entropy,thd_i");
    if (!_cloud->pushJSON(path.c_str(), json)) return false;
    delay(2);
  }
  return true;
}
