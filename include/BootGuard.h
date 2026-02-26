#pragma once
#include <Arduino.h>

// BootGuard: prevents "bad" OTA builds from locking you out.
// - Uses ESP-IDF OTA rollback (pending-verify -> mark valid) when available.
// - Detects repeated crash resets on the SAME partition and enters SAFE MODE.
//   SAFE MODE runs OTA + WiFi only (you decide what to skip in main.cpp).

class BootGuard {
public:
  // stableWindowMs: how long the firmware must run (in FULL mode) before being marked valid.
  // maxCrashBoots:  consecutive crash resets (panic/WDT/brownout) on same partition before safe mode.
  static void begin(uint32_t stableWindowMs = 45000, uint8_t maxCrashBoots = 3);

  // Call very early in loop(). When eligible, it will:
  // - clear crash counter after stableWindowMs
  // - if the running image is "pending verify", mark it VALID (cancel rollback)
  static void loop();

  // True when BootGuard decided this boot should run in SAFE MODE (OTA-only).
  static bool safeMode();

  // True when the running partition is in "pending verify" state (fresh OTA boot).
  static bool pendingVerify();

  // For optional UI/debug.
  static uint8_t crashBoots();

private:
  BootGuard() = delete;
};