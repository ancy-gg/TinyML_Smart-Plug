#pragma once
#include <Arduino.h>
#include "MainConfiguration.h"

class FirebaseHandler;

class DataLogger {
public:
  void begin(FirebaseHandler* cloud);
  void setEnabled(bool en);
  bool enabled() const { return _manualEnabled || _autoEnabled; }
  bool manualEnabled() const { return _manualEnabled; }
  bool autoCaptureActive() const { return _autoEnabled; }
  void setDurationSeconds(uint16_t sec);
  void setSession(const String& sessionId, const String& loadType, int labelOverride);
  bool startAutoCapture(const String& reason, uint16_t sec = AUTO_ARC_CAPTURE_DURATION_S);
  void stopAutoCapture();
  void ingest(const FeatureFrame& f, FaultState st, int arcCounter);
  void loop();

private:
  struct Rec {
    uint64_t epoch_ms;
    float cycle_nmse, zcv, zc_dwell_ratio, pulse_count_per_cycle, peak_fluct_cv;
    float midband_residual_rms, hf_band_energy_ratio, wpe_entropy, spec_entropy, thd_i;
    float v_rms, i_rms, temp_c;
    int8_t  label_arc;
    uint8_t model_pred, feat_valid, current_valid, fault_state;
    int16_t arc_counter;
    float   adc_fs_hz;
    uint8_t auto_capture;
  };
  struct SessionSpec {
    String sessionId = "";
    String loadType = "unknown";
    int8_t labelOverride = ML_UNKNOWN_LABEL;
    uint16_t durationS = ML_LOG_DURATION_S;
  };

  FirebaseHandler* _cloud = nullptr;
  bool _manualEnabled = false;
  bool _autoEnabled = false;
  bool _wasEnabled = false;
  SessionSpec _manual;
  SessionSpec _auto;
  uint32_t _sessionStartMs = 0;
  uint32_t _chunkStartMs = 0;
  uint32_t _lastFlushAttemptMs = 0;
  uint16_t _count = 0;
  static constexpr uint16_t MAX_REC = 600;
  Rec _buf[MAX_REC];

  const SessionSpec& activeSpec() const;
  bool activeIsAuto() const { return (!_manualEnabled && _autoEnabled); }
  bool flushToFirebase(bool finalFlush);
  void resetRuntimeState_();
  void closeManualSession_(const String& finishedSessionId);
  static String sanitizeToken(const String& s);
};
