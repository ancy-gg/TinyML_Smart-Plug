#include "BootGuard.h"

#include <esp_system.h>     
#include <esp_attr.h>     
#include <esp_ota_ops.h>
#include <esp_partition.h>

// RTC retained across warm resets (WDT/panic/soft reset). Cleared on power loss.
RTC_DATA_ATTR static uint32_t s_magic = 0;
RTC_DATA_ATTR static uint32_t s_lastAppAddr = 0;
RTC_DATA_ATTR static uint8_t  s_crashBoots = 0;
RTC_DATA_ATTR static uint8_t  s_safeMode = 0;

static uint32_t s_stableWindowMs = 45000;
static uint8_t  s_maxCrashBoots  = 3;
static uint32_t s_bootMs0        = 0;
static bool     s_pendingVerify  = false;

static constexpr uint32_t MAGIC = 0xC0FFEE42;

static bool isCrashReset(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_PANIC:
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
    case ESP_RST_BROWNOUT:
      return true;
    default:
      return false;
  }
}

static bool getPendingVerifyState() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) return false;

  esp_ota_img_states_t st;
  const esp_err_t e = esp_ota_get_state_partition(running, &st);
  if (e != ESP_OK) return false; // if unsupported, treat as "not pending"
  return (st == ESP_OTA_IMG_PENDING_VERIFY);
}

void BootGuard::begin(uint32_t stableWindowMs, uint8_t maxCrashBoots) {
  s_stableWindowMs = (stableWindowMs < 5000) ? 5000 : stableWindowMs;
  s_maxCrashBoots  = (maxCrashBoots < 1) ? 1 : maxCrashBoots;
  s_bootMs0 = millis();

  if (s_magic != MAGIC) {
    s_magic = MAGIC;
    s_lastAppAddr = 0;
    s_crashBoots = 0;
    s_safeMode = 0;
  }

  const esp_partition_t* running = esp_ota_get_running_partition();
  const uint32_t runAddr = running ? running->address : 0;

  // If we switched partitions (successful OTA or rollback), forget old crash history.
  if (runAddr != 0 && s_lastAppAddr != 0 && runAddr != s_lastAppAddr) {
    s_crashBoots = 0;
    s_safeMode = 0;
  }
  if (runAddr != 0) s_lastAppAddr = runAddr;

  const esp_reset_reason_t rr = esp_reset_reason();
  const bool crash = isCrashReset(rr);

  if (crash) {
    if (s_crashBoots < 255) s_crashBoots++;
  } else {
    // Non-crash reset (power on, manual reset, software restart after OTA) clears the streak.
    s_crashBoots = 0;
  }

  s_pendingVerify = getPendingVerifyState();

  // Fresh OTA boot + repeated crash => force rollback ASAP.
  if (s_pendingVerify && crash && s_crashBoots >= s_maxCrashBoots) {
    (void)esp_ota_mark_app_invalid_rollback_and_reboot();
    // If it fails, we fall through to SAFE MODE.
  }

  // Enter SAFE MODE if the SAME partition keeps crashing.
  if (crash && s_crashBoots >= s_maxCrashBoots) {
    s_safeMode = 1;
  }
}

void BootGuard::loop() {
  // Never mark valid while in safe mode (safe mode might avoid the buggy code path).
  if (s_safeMode) return;

  const uint32_t up = millis() - s_bootMs0;
  if (up < s_stableWindowMs) return;

  // Stable: clear crash streak.
  s_crashBoots = 0;

  // Pending verify => mark valid after surviving stable window.
  if (s_pendingVerify) {
    (void)esp_ota_mark_app_valid_cancel_rollback();
    s_pendingVerify = false;
  }
}

bool BootGuard::safeMode() { return s_safeMode != 0; }
bool BootGuard::pendingVerify() { return s_pendingVerify; }
uint8_t BootGuard::crashBoots() { return s_crashBoots; }