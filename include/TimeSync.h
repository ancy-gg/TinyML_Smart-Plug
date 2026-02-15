#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <Arduino.h>

class TimeSync {
public:
  void begin(const char* tz = "Asia/Manila",
             const char* ntp1 = "pool.ntp.org",
             const char* ntp2 = "time.nist.gov");

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
