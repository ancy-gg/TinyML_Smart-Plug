#include "TimeSync.h"

#include <time.h>
#include <sys/time.h>

// Heuristic: if epoch is before 2020-01-01, consider "not synced".
static bool timeLooksValid(time_t t) {
  return t > 1577836800; // 2020-01-01
}

void TimeSync::begin(const char* tz, const char* ntp1, const char* ntp2) {
  if (_started) return;
  _started = true;

  setenv("TZ", tz, 1);
  tzset();

  // Start SNTP (non-blocking)
  configTime(0, 0, ntp1, ntp2);
}

void TimeSync::update() {
  if (!_started || _synced) return;

  time_t now = time(nullptr);
  if (timeLooksValid(now)) _synced = true;
}

uint64_t TimeSync::nowEpochMs() const {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  if (!timeLooksValid(tv.tv_sec)) return 0;
  return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000ULL);
}

String TimeSync::nowISO8601Ms() const {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  if (!timeLooksValid(tv.tv_sec)) return String("");

  struct tm tmLocal;
  localtime_r(&tv.tv_sec, &tmLocal);

  char buf[40];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
           tmLocal.tm_year + 1900,
           tmLocal.tm_mon + 1,
           tmLocal.tm_mday,
           tmLocal.tm_hour,
           tmLocal.tm_min,
           tmLocal.tm_sec,
           (int)(tv.tv_usec / 1000));
  return String(buf);
}
