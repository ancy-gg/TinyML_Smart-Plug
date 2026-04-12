#ifndef FIREBASE_NETWORK_H
#define FIREBASE_NETWORK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <WiFi.h>
#ifndef WiFI_CONNECTED
#define WiFI_CONNECTED (WiFi.status() == WL_CONNECTED)
#endif
#include <Firebase_ESP_Client.h>
#include "MainConfiguration.h"

class FirebaseNetwork {
public:
  void begin(const char* apiKey, const char* dbUrl, const char* tz = "Asia/Manila",
             const char* ntp1 = "pool.ntp.org", const char* ntp2 = "time.nist.gov");

  void updateClock();
  bool isSynced() const { return _synced; }
  uint64_t nowEpochMs() const;
  uint64_t epochForUptimeMs(uint32_t uptimeMs) const;
  String nowISO8601Ms() const;

  void setFirmwareVersion(const char* fw);
  void setNormalIntervalMs(uint32_t ms);
  void setFaultIntervalMs(uint32_t ms);

  bool isReady() const;
  void loop();
  void pollControls(bool allowNet, bool portalActive);

  bool getString(const char* path, String& out);
  bool getBool(const char* path, bool& out);
  bool getInt(const char* path, int& out);

  bool consumePortalRequest();
  bool consumeRelayOnRequest(String* tokenOut = nullptr);
  bool consumeRelayOffRequest(String* tokenOut = nullptr);
  bool consumeFaultClearRequest();
  bool consumeRevertFirmwareRequest();
  bool consumeOtaCheckRequest();
  bool fetchMlControl(bool& enabled, int& dur, int& labelOv, String& sid, String& load, String& deviceFamily, String& deviceName, int& trialNumber, String& divisionTag, String& notes, bool& trustedNormal, String& requestToken) const;

  void requestLiveUpdate(float v, float c, float apparentPower, float t,
                         float abs_irms_zscore_vs_baseline, float delta_irms_abs,
                         float halfcycle_asymmetry, float suspicious_run_energy,
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
                         bool faultLatched, bool webControlsLocked, bool relayLatchedOn, bool relayPulseActive);

  bool logStatusEvent(const String& status, float v, float c, float apparentPower, float t);
  bool logFeatureEvent(const String& status, const FeatureFrame& f, float apparentPower, bool relayTrip);
  bool publishOtaDebug(const String& phase, const String& detail, int progress = -1);
  bool publishControlAck(const String& kind, const String& token);
  bool publishRelayPulseEvent(const String& kind, const String& token, const String& source = "device");
  void stopAllClients();

  void setLogEnabled(bool en);
  bool logEnabled() const { return _manualEnabled || _autoEnabled; }
  bool manualEnabled() const { return _manualEnabled; }
  bool autoCaptureActive() const { return false; }
  void setLogDurationSeconds(uint16_t sec);
  void setMlUploadSuspended(bool en) { _suspendMlUpload = en; }
  void setLogSession(const String& sessionId, const String& loadType, int labelOverride, const String& deviceFamily = "unknown", const String& deviceName = "unknown_device", int trialNumber = 1, const String& divisionTag = "steady", const String& notes = "", bool trustedNormal = false);
  bool startAutoCapture(const String& reason, uint16_t sec = AUTO_ARC_CAPTURE_DURATION_S);
  void stopAutoCapture();
  void ingestLog(const FeatureFrame& f, FaultState st, int arcCounter);

private:
  struct LiveSnapshot {
    float v = 0.0f, c = 0.0f, apparentPower = 0.0f, t = 0.0f;
    float abs_irms_zscore_vs_baseline = 0.0f, delta_irms_abs = 0.0f;
    float halfcycle_asymmetry = 0.0f, cycle_nmse = 0.0f;
    float delta_hf_energy = 0.0f, v_sag_pct = 0.0f;
    float suspicious_run_energy = 0.0f, delta_flux = 0.0f;
    float midband_residual_ratio = 0.0f, zcv = 0.0f;
    float spectral_flux_midhf = 0.0f, peak_fluct_cv = 0.0f;
    float residual_crest_factor = 0.0f, thd_i = 0.0f;
    float hf_energy_delta = 0.0f, edge_spike_ratio = 0.0f;
    uint8_t model_pred = 0;
    int8_t contextFamilyCodeRuntime = CONTEXT_FAMILY_UNKNOWN;
    float contextFamilyConfidence = 0.0f;
    int8_t provisionalContextFamilyCode = CONTEXT_FAMILY_UNKNOWN;
    float provisionalContextFamilyConfidence = 0.0f;
    bool contextAcquiring = false;
    bool contextLatched = false;
    String state;
    bool faultLatched = false;
    bool webControlsLocked = false;
    bool relayLatchedOn = false;
    bool relayPulseActive = false;
  };

  struct HistoryJob {
    String status;
    FeatureFrame f;
    float apparentPower = 0.0f;
    bool relayTrip = false;
    bool useFeaturePayload = false;
  };

  struct SessionSpec {
    String sessionId = "";
    String loadType = "unknown";
    String deviceFamily = "unknown";
    String deviceName = "unknown_device";
    int trialNumber = 1;
    String divisionTag = "steady";
    String notes = "";
    uint8_t trustedNormalSession = 0;
    int8_t labelOverride = ML_UNKNOWN_LABEL;
    uint16_t durationS = ML_LOG_DURATION_S;
  };

  struct Rec {
    uint64_t epoch_ms;
    uint32_t uptime_ms;
    uint32_t frame_start_uptime_ms;
    uint32_t frame_end_uptime_ms;
    uint32_t feature_compute_end_uptime_ms;
    uint32_t log_enqueue_uptime_ms;
    float frame_dt_ms;
    float compute_time_ms;
    float timing_skew_ms;
    uint16_t fft_size;
    uint16_t hop_samples;
    float spectral_flux_midhf, residual_crest_factor, edge_spike_ratio, midband_residual_ratio, cycle_nmse;
    float peak_fluct_cv, thd_i, hf_energy_delta, zcv, abs_irms_zscore_vs_baseline;
    float fs_err_hz, suspicious_run_energy, delta_irms_abs, delta_hf_energy, delta_flux, v_sag_pct, halfcycle_asymmetry;
    float v_rms, i_rms, temp_c;
    uint32_t queue_drop_count;
    uint16_t suspicious_run_len;
    uint16_t invalid_loaded_run_len;
    int8_t  label_arc;
    uint8_t restrike_count_short;
    uint8_t model_pred, feat_valid, current_valid, fault_state;
    uint8_t sampling_quality_bad, invalid_loaded_flag, invalid_off_flag;
    uint8_t relay_blank_active, turnon_blank_active, transient_blank_active;
    int8_t  device_family_code;
    int8_t  context_family_code_runtime;
    int8_t  context_family_code_provisional;
    float   context_family_confidence;
    float   context_family_confidence_provisional;
    uint8_t context_acquiring;
    uint8_t context_latched;
    int16_t arc_counter;
    float   adc_fs_hz;
    uint8_t auto_capture;
    uint8_t feature_space_version;
  };

  static constexpr uint8_t HISTORY_QUEUE_MAX = 24;
  static constexpr uint32_t CLOUD_TX_MIN_GAP_MS = 220UL;
  static constexpr uint32_t CLOUD_TX_RETRY_MS = 1800UL;
  static constexpr uint32_t CLOUD_CTRL_FAIL_RETRY_MS = 900UL;
  static constexpr uint32_t CLOUD_CTRL_READ_GAP_MS = 500UL;
  static constexpr uint32_t CLOUD_CTRL_READ_GAP_LOGGING_MS = 1200UL;
  static constexpr uint16_t CLOUD_TLS_RX_BUFFER_BYTES = 2048;
  static constexpr uint16_t CLOUD_TLS_TX_BUFFER_BYTES = 1024;
  static constexpr uint16_t CLOUD_RESPONSE_BUFFER_BYTES = 1024;
  static constexpr uint32_t CLOUD_SOCKET_TIMEOUT_MS = 3500UL;
  static constexpr uint32_t CLOUD_SERVER_RESPONSE_TIMEOUT_MS = 3000UL;
  static constexpr uint16_t ROWS_PER_CHUNK = 8;

  FirebaseData fbLive;
  FirebaseData fbRead;
  FirebaseData fbHistory;
  FirebaseData fbLog;
  FirebaseData fbCtrl;
  FirebaseAuth auth;
  FirebaseConfig config;

  unsigned long _lastLiveSend = 0;
  uint32_t _normalIntervalMs = CLOUD_LIVE_NORMAL_INTERVAL_MS;
  uint32_t _faultIntervalMs  = CLOUD_LIVE_FAULT_INTERVAL_MS;
  uint32_t _lastTxMs = 0;
  uint32_t _txBackoffUntilMs = 0;

  String _lastSentLiveState = "";
  String _lastLoggedFaultState = "";
  String _lastHourlyNormalKey = "";
  String _fwVersion = "—";

  bool _bootEventLogged = false;
  bool _haveLastMains = false;
  bool _lastMainsPresent = false;
  String _lastTransitionEvent = "";
  uint64_t _lastTransitionEpochMs = 0;

  bool _started = false;
  bool _synced = false;
  bool _epochAnchorValid = false;
  uint64_t _epochAnchorMs = 0;
  uint32_t _uptimeAnchorMs = 0;
  mutable portMUX_TYPE _timeAnchorMux = portMUX_INITIALIZER_UNLOCKED;

  bool _pendingLive = false;
  LiveSnapshot _live;

  HistoryJob _historyQueue[HISTORY_QUEUE_MAX];
  uint8_t _historyHead = 0;
  uint8_t _historyTail = 0;
  uint8_t _historyCount = 0;

  bool _portalTokenPrimed = false;
  bool _relayOnTokenPrimed = false;
  bool _relayOffTokenPrimed = false;
  bool _faultClearTokenPrimed = false;
  bool _revertFwTokenPrimed = false;
  bool _otaCheckTokenPrimed = false;
  String _portalToken = "";
  String _relayOnToken = "";
  String _relayOffToken = "";
  String _faultClearToken = "";
  String _revertFwToken = "";
  String _otaCheckToken = "";
  String _portalTokenHandled = "";
  String _relayOnTokenHandled = "";
  String _relayOffTokenHandled = "";
  String _faultClearTokenHandled = "";
  String _revertFwTokenHandled = "";
  String _otaCheckTokenHandled = "";
  bool _portalRequestPending = false;
  bool _relayOnPending = false;
  bool _relayOffPending = false;
  bool _faultClearPending = false;
  bool _revertFwPending = false;
  bool _otaCheckPending = false;
  bool _mlEnabledCache = false;
  int  _mlDurationCache = ML_LOG_DURATION_S;
  int  _mlLabelOverrideCache = ML_UNKNOWN_LABEL;
  String _mlSessionIdCache = "";
  String _mlLoadTypeCache = "unknown";
  String _mlDeviceFamilyCache = "unknown";
  String _mlDeviceNameCache = "unknown_device";
  int _mlTrialNumberCache = 1;
  String _mlDivisionTagCache = "steady";
  String _mlNotesCache = "";
  bool _mlTrustedNormalCache = false;
  String _mlRequestTokenCache = "";
  uint8_t _controlPollSlot = 0;
  uint32_t _lastControlPollMs = 0;

  bool _manualEnabled = false;
  bool _autoEnabled = false;
  bool _wasEnabled = false;
  SessionSpec _manual;
  SessionSpec _auto;
  uint32_t _manualArmMs = 0;
  uint32_t _sessionStartMs = 0;
  uint32_t _chunkStartMs = 0;
  uint16_t _count = 0;
  static constexpr uint16_t MAX_REC_LIMIT = 2400;
  uint16_t _maxRec = 0;
  Rec* _buf = nullptr;
  bool _mlUploadActive = false;
  SessionSpec _uploadSpec;
  uint16_t _uploadTotalCount = 0;
  uint16_t _uploadNextIndex = 0;
  uint16_t _uploadChunkCount = 0;
  uint16_t _uploadChunkIndex = 0;
  bool _uploadFinalFlush = false;
  bool _uploadAuto = false;
  bool _suspendMlUpload = false;
  bool _loggerFrozen = false;
  bool _collectionEligible = false;
  uint8_t _collectionGoodFrames = 0;
  uint32_t _sessionChunkSerial = 0;

  static bool timeLooksValid(time_t t);
  static String powerConditionForState(const String& state, float v);
  static bool isTransitionState(const String& state);
  static String sanitizeToken(const String& s);
  static void updateControlToken_(const String& token, bool& primed, String& cache, String& handled, bool& pendingFlag);
  void configureClient_(FirebaseData& client, uint16_t responseSize = CLOUD_RESPONSE_BUFFER_BYTES);
  void recoverClient_(FirebaseData& client, uint32_t backoffMs);

  const SessionSpec& activeSpec() const;
  bool activeIsAuto() const { return (!_manualEnabled && _autoEnabled); }

  bool pushHistoryRecord_(const HistoryJob& job);
  bool controlWorkPending_() const;
  void clearControlToken_(const char* path, String& cache, bool& pendingFlag);
  bool enqueueHistory_(const HistoryJob& job);
  bool dequeueHistory_(HistoryJob& job);
  bool serviceLive_();
  bool serviceHistory_();
  bool serviceMlUpload_();
  void serviceMlState_();
  bool closeManualSession_(const String& finishedSessionId);
  void resetLoggerRuntime_();
  bool captureUsefulForManual_(const FeatureFrame& f, FaultState st) const;
  float computeContinuousDurationSeconds_(const Rec* recs, uint16_t count) const;
  void ensureBuffersAllocated_();
};

#endif
