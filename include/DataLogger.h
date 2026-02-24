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

  void setDurationSeconds(uint16_t sec);

  void ingest(const FeatureFrame& f, FaultState st, int arcCounter);
  void loop();

private:
#if ENABLE_ML_LOGGER
  struct Rec {
    uint64_t epoch_ms;

    float spectral_entropy;
    float thd_pct;
    float zcv;
    float v_rms;
    float i_rms;
    float temp_c;
    uint8_t label_arc;

    uint8_t model_pred;
    uint8_t state;
    uint8_t arc_cnt;
  };

  CloudHandler* _cloud = nullptr;
  bool _enabled = false;

  uint16_t _durationS = ML_LOG_DURATION_S;
  uint16_t _targetCount = (uint16_t)(ML_LOG_DURATION_S * ML_LOG_RATE_HZ);
  uint16_t _count = 0;

  uint32_t _startMs = 0;
  uint32_t _lastFlushAttemptMs = 0;

  static constexpr uint16_t MAX_REC = 420;
  Rec _buf[MAX_REC];

  bool flushToFirebase(uint32_t elapsedMs);
#else
  CloudHandler* _cloud = nullptr;
  bool _enabled = false;
  uint16_t _durationS = ML_LOG_DURATION_S;
  uint16_t _targetCount = (uint16_t)(ML_LOG_DURATION_S * ML_LOG_RATE_HZ);
#endif
};