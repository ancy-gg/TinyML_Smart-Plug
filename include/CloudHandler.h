#ifndef CLOUD_HANDLER_H
#define CLOUD_HANDLER_H

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include "TimeSync.h"

class CloudHandler {
public:
  void begin(const char* apiKey, const char* dbUrl);

  void setFirmwareVersion(const char* fw);
  void setNormalIntervalMs(uint32_t ms);
  void setFaultIntervalMs(uint32_t ms);

  void update(float v, float c, float apparentPower, float t,
              float cycle_nmse, float zcv, float zc_dwell_ratio,
              float pulse_count_per_cycle, float peak_fluct_cv,
              float midband_residual_rms, float hf_band_energy_ratio,
              float wpe_entropy, float spec_entropy, float thd_i,
              uint8_t model_pred,
              const String& state, TimeSync* time);

  bool isReady() const;
  bool getString(const char* path, String& out);
  bool getBool(const char* path, bool& out);
  bool getInt(const char* path, int& out);

  bool pushJSON(const char* path, FirebaseJson& json);
  bool logStatusEvent(const String& status, float v, float c, float apparentPower, float t, TimeSync* time);

private:
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;

  unsigned long _lastLiveSend = 0;
  uint32_t _normalIntervalMs = 6000;
  uint32_t _faultIntervalMs  = 1500;

  String _lastSentLiveState = "";
  String _lastLoggedFaultState = "";
  String _lastHourlyNormalKey = "";
  String _fwVersion = "—";

  bool pushHistoryRecord(const String& status, float v, float c, float apparentPower, float t,
                         float cycle_nmse, float zcv, float zc_dwell_ratio,
                         float pulse_count_per_cycle, float peak_fluct_cv,
                         float midband_residual_rms, float hf_band_energy_ratio,
                         float wpe_entropy, float spec_entropy, float thd_i,
                         uint8_t model_pred,
                         TimeSync* time);
};

#endif
