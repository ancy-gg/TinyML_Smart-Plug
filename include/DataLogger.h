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

  void setDurationSeconds(uint16_t sec); // 5..12 sec supported

  // Call around ~30Hz (your ML_LOG_RATE_HZ)
  void ingest(const FeatureFrame& f, FaultState st, int arcCounter);

  // Call periodically in loop()
  void loop();

private:
#if ENABLE_ML_LOGGER
  struct Rec {
    uint64_t epoch_ms;

    // CSV fields (training compatible)
    float spectral_entropy;
    float thd_pct;
    float zcv;     // we store your zcv_ms here, but column name must be 'zcv'
    float v_rms;
    float i_rms;
    float temp_c;
    uint8_t label_arc;

    // Extra debug (not in CSV)
    uint8_t model_pred;
    uint8_t state;
    uint8_t arc_cnt;
  };

  CloudHandler* _cloud = nullptr;
  bool _enabled = false;

  uint16_t _durationS = ML_LOG_DURATION_S;
  uint16_t _targetCount = (uint16_t)(ML_LOG_DURATION_S * ML_LOG_RATE_HZ);
  uint16_t _count = 0;

  static constexpr uint16_t MAX_REC = 420; // up to ~14 seconds @ 30 Hz
  Rec _buf[MAX_REC];

  bool flushToFirebase();
#else
  CloudHandler* _cloud = nullptr;
  bool _enabled = false;
  uint16_t _durationS = ML_LOG_DURATION_S;
  uint16_t _targetCount = (uint16_t)(ML_LOG_DURATION_S * ML_LOG_RATE_HZ);
#endif
};