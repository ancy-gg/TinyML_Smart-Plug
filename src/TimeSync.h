#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <Arduino.h>

// Non-blocking SNTP time sync for ESP32 (Arduino).
// No physical RTC required.
//
// Usage:
//   TimeSync time;
//   time.begin("Asia/Manila");
//   // in loop:
//   time.update();
//   uint64_t ms = time.nowEpochMs();
//   String iso = time.nowISO8601Ms();

class TimeSync {
public:
  // tz can be "Asia/Manila" (often works) or fallback "CST-8"
  void begin(const char* tz = "Asia/Manila",
             const char* ntp1 = "pool.ntp.org",
             const char* ntp2 = "time.nist.gov");

  // Cheap poll to update internal state. Safe to call every loop.
  void update();

  bool isSynced() const { return _synced; }

  // Epoch time in milliseconds (0 if not synced yet)
  uint64_t nowEpochMs() const;

  // Local time string with milliseconds. Returns "" if not synced.
  String nowISO8601Ms() const;

private:
  bool _started = false;
  bool _synced = false;
};

#endif
