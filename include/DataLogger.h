#pragma once
#include <Arduino.h>
#include "SmartPlugTypes.h"
#include "SmartPlugConfig.h"

class CloudHandler;

class DataLogger {
public:
  void begin(CloudHandler* cloud);

  void setEnabled(bool en);
  bool enabled() const { return _enabled; }

  void setDurationSeconds(uint16_t sec);   // 5..60
  void setSession(const String& sessionId, const String& loadType, int labelOverride);

  void ingest(const FeatureFrame& f, FaultState st, int arcCounter);
  void loop();

private:
#if ENABLE_ML_LOGGER
  struct Rec {
    uint64_t epoch_ms;

    // Training fields
    float spectral_entropy;
    float thd_pct;
    float zcv;
    float hf_ratio;
    float hf_var;
    float v_rms;
    float i_rms;
    float temp_c;
    uint8_t label_arc;

    // Debug
    uint8_t model_pred;
    uint8_t state;
    uint8_t arc_cnt;
  };

  CloudHandler* _cloud = nullptr;

  bool _enabled = false;
  bool _wasEnabled = false;

  String _sessionId = "";
  String _loadType = "unknown";
  int8_t _labelOverride = -1; // -1 auto, 0 normal, 1 arc

  uint16_t _durationS = ML_LOG_DURATION_S;

  uint32_t _chunkStartMs = 0;
  uint32_t _lastFlushAttemptMs = 0;

  uint16_t _count = 0;
  static constexpr uint16_t MAX_REC = 600; // safe upper cap
  Rec _buf[MAX_REC];

  bool flushToFirebase(bool finalFlush);
  static String sanitizeToken(const String& s);

#else
  CloudHandler* _cloud = nullptr;
  bool _enabled = false;
  String _sessionId = "";
  String _loadType = "unknown";
  int8_t _labelOverride = -1;
  uint16_t _durationS = ML_LOG_DURATION_S;
#endif
};