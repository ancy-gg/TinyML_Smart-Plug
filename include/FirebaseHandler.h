#ifndef FIREBASE_HANDLER_H
#define FIREBASE_HANDLER_H

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include "MainConfiguration.h"

class FirebaseHandler {
public:
  void begin(const char* apiKey, const char* dbUrl, const char* tz = "Asia/Manila",
             const char* ntp1 = "pool.ntp.org", const char* ntp2 = "time.nist.gov");

  void updateClock();
  bool isSynced() const { return _synced; }
  uint64_t nowEpochMs() const;
  String nowISO8601Ms() const;

  void setFirmwareVersion(const char* fw);
  void setNormalIntervalMs(uint32_t ms);
  void setFaultIntervalMs(uint32_t ms);

  bool isReady() const;
  bool getString(const char* path, String& out);
  bool getBool(const char* path, bool& out);
  bool getInt(const char* path, int& out);

  bool pushJSON(const char* path, FirebaseJson& json);
  bool updateJSON(const char* path, FirebaseJson& json);
  bool logStatusEvent(const String& status, float v, float c, float apparentPower, float t);
  bool logFeatureEvent(const String& status, const FeatureFrame& f, float apparentPower, bool relayTrip);

  void update(float v, float c, float apparentPower, float t,
              float cycle_nmse, float zcv, float zc_dwell_ratio,
              float pulse_count_per_cycle, float peak_fluct_cv,
              float midband_residual_rms, float hf_band_energy_ratio,
              float wpe_entropy, float spec_entropy, float thd_i,
              uint8_t model_pred,
              const String& state);

private:
  FirebaseData fbLive;
  FirebaseData fbRead;
  FirebaseData fbHistory;
  FirebaseData fbLog;
  FirebaseAuth auth;
  FirebaseConfig config;

  unsigned long _lastLiveSend = 0;
  uint32_t _normalIntervalMs = CLOUD_LIVE_NORMAL_INTERVAL_MS;
  uint32_t _faultIntervalMs  = CLOUD_LIVE_FAULT_INTERVAL_MS;

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

  bool pushHistoryRecord(const String& status, float v, float c, float apparentPower, float t,
                         float cycle_nmse, float zcv, float zc_dwell_ratio,
                         float pulse_count_per_cycle, float peak_fluct_cv,
                         float midband_residual_rms, float hf_band_energy_ratio,
                         float wpe_entropy, float spec_entropy, float thd_i,
                         uint8_t model_pred);
};

#endif
