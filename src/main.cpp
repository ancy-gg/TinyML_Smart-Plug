#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <math.h>

#include "SmartPlugConfig.h"
#include "SmartPlugTypes.h"
#include "NetworkManager.h"
#include "CloudHandler.h"
#include "NotificationOLED.h"
#include "TimeSync.h"
#include "PullOTA.h"
#include "BootGuard.h"

#include "CurrentSensor.h"
#include "VoltageSensor.h"
#include "TempSensor.h"
#include "FaultLogic.h"
#include "Actuators.h"
#include "ArcModel.h"
#include "ArcFeatures.h"

#if ENABLE_ML_LOGGER
  #include "DataLogger.h"
#endif

#define API_KEY "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw"
#define DATABASE_URL "tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app"
static const char* FW_VERSION = "TSP-v3.7.6-p-mcp";   //m - set calib to 0.0 (1.0 for first-order var)
                                                      //p - change 0 relay, with calibration
                                                      //c - change 1 relay, with calibration
                                                      
static const char* OTA_DESIRED_VERSION_PATH = "/ota/desired_version";
static const char* OTA_FIRMWARE_URL_PATH    = "/ota/firmware_url";
static const uint32_t OTA_CHECK_INTERVAL_MS = 60000;
static constexpr uint32_t SENSOR_BOOT_SETTLE_MS = CURRENT_BOOT_SETTLE_MS;
static constexpr uint32_t PROTECTION_INHIBIT_MS = 5000UL;
static constexpr uint32_t ML_CONTROL_FAST_POLL_MS = 1000UL;

NotificationOLED       oled(0x3C);
NetworkManager   net;
CloudHandler     cloud;
TimeSync         timeSync;
PullOTA          ota;

VoltageSensor    voltSensor(PIN_VOLT_ADC);
TempSensor       tempSensor(PIN_TEMP_ADC);

FaultLogic       faultLogic;
Actuators        actuators;

#if ENABLE_ML_LOGGER
DataLogger       logger;
#endif

CurrentSensor    curSensor;
ArcFeatures      arcFeat;
CurrentCalib     curCalib;

static uint16_t s_raw[N_SAMP];
static QueueHandle_t qFeat = nullptr;

static bool gSafeMode = false;
static volatile bool gPauseByOta = false;

static bool arcInputStable(bool currentValid, bool featValid, float irms) {
  static uint32_t stableSince = 0;
  static float refIrms = 0.0f;
  const uint32_t now = millis();

  if (!currentValid || !featValid || irms < ARC_MIN_IRMS_A) {
    stableSince = 0;
    refIrms = irms;
    return false;
  }

  if (stableSince == 0) {
    stableSince = now;
    refIrms = irms;
    return false;
  }

  const float tol = fmaxf(0.08f, 0.25f * fmaxf(refIrms, 0.10f));
  if (fabsf(irms - refIrms) > tol) {
    stableSince = now;
    refIrms = irms;
    return false;
  }

  refIrms += 0.08f * (irms - refIrms);
  return (now - stableSince) >= 1500UL;
}

static bool debouncedMainsPresentForState(float vProtect) {
  static bool init = false;
  static bool stable = false;
  static int8_t pending = -1;
  static uint32_t pendingSince = 0;

  const uint32_t now = millis();
  const bool rawOn = (vProtect >= MAINS_PRESENT_ON_V);
  const bool rawOff = (vProtect <= MAINS_PRESENT_OFF_V);

  if (!init) {
    stable = rawOn;
    init = true;
  }

  int8_t target = -1;
  if (rawOn) target = 1;
  else if (rawOff) target = 0;

  if (target >= 0 && target != (stable ? 1 : 0)) {
    if (pending != target) {
      pending = target;
      pendingSince = now;
    } else if ((now - pendingSince) >= MAINS_EDGE_DEBOUNCE_MS) {
      stable = (target != 0);
      pending = -1;
      pendingSince = 0;
    }
  } else if (target == (stable ? 1 : 0)) {
    pending = -1;
    pendingSince = 0;
  }

  return stable;
}


static bool classifyUnpluggedSocket(float vRaw, float vProtect, float irmsA, bool currentValid, FaultState st, uint32_t* sinceMs) {
  if (!sinceMs) return false;

  const bool rawGone  = (vRaw <= MAINS_PRESENT_OFF_V);
  const bool protGone = (vProtect <= MAINS_PRESENT_OFF_V);
  const bool zeroLike = rawGone || protGone;
  const bool noLoad = (!currentValid) || (irmsA <= LOAD_OFF_DETECT_A);
  const bool arcLike = (st == STATE_ARCING);
  const bool heatLike = (st == STATE_HEATING);
  const bool faultKeep = arcLike || heatLike;

  if (zeroLike && noLoad && !faultKeep) {
    if (*sinceMs == 0) *sinceMs = millis();
  } else if (vRaw >= MAINS_PRESENT_ON_V || vProtect >= MAINS_PRESENT_ON_V || !noLoad || faultKeep) {
    *sinceMs = 0;
  }

  return (*sinceMs != 0) && ((millis() - *sinceMs) >= UNPLUGGED_STATE_DELAY_MS);
}

static float cleanDisplayCurrent(float irmsRaw, bool currentValid, bool featValid, float vRaw, float vProtect) {
  static bool learnStarted = false;
  static bool floorLocked = false;
  static uint32_t learnStartMs = 0;
  static double floorSqAcc = 0.0;
  static uint32_t floorCount = 0;
  static float learnedFloorA = 0.0f;

  static bool displayGateOn = false;
  static float dispIrms = 0.0f;
  static uint32_t lastUpdateMs = 0;

  const uint32_t now = millis();
  const bool mainsOn = (vProtect >= MAINS_PRESENT_ON_V);

  if (vRaw <= MAINS_PRESENT_OFF_V) {
    dispIrms = 0.0f;
    displayGateOn = false;
    lastUpdateMs = now;
    return 0.0f;
  }

  if (!learnStarted && mainsOn) {
    learnStarted = true;
    learnStartMs = now;
  }

  const bool canLearnIdle =
      CURRENT_IDLE_LEARN_ENABLE &&
      learnStarted &&
      !floorLocked &&
      mainsOn &&
      currentValid &&
      !featValid &&
      irmsRaw > 0.005f &&
      irmsRaw < CURRENT_IDLE_LEARN_MAX_A &&
      ((now - learnStartMs) <= CURRENT_IDLE_LEARN_WINDOW_MS);

  if (canLearnIdle) {
    floorSqAcc += (double)irmsRaw * (double)irmsRaw;
    floorCount++;
    learnedFloorA = sqrtf((float)(floorSqAcc / (double)floorCount));
  }

  if (CURRENT_IDLE_LEARN_ENABLE &&
      learnStarted &&
      !floorLocked &&
      ((now - learnStartMs) > CURRENT_IDLE_LEARN_WINDOW_MS)) {
    floorLocked = (floorCount >= CURRENT_IDLE_LEARN_MIN_FRAMES);
  }

  float irms = irmsRaw;
  if (floorLocked && learnedFloorA > 0.005f) {
    const float floorUsed = fmaxf(0.0f, learnedFloorA - CURRENT_IDLE_FLOOR_MARGIN_A);
    irms = sqrtf(fmaxf(0.0f, (irms * irms) - (floorUsed * floorUsed)));
  }

  if (!currentValid) irms = 0.0f;

  const uint32_t dtMs = (lastUpdateMs == 0) ? 0 : (now - lastUpdateMs);
  lastUpdateMs = now;
  const float dtS = (dtMs > 0) ? (dtMs / 1000.0f) : 0.0f;
  const float tau = (irms <= CURRENT_DISPLAY_SMOOTH_SPLIT_A)
                      ? CURRENT_DISPLAY_SMOOTH_TAU_LOW_S
                      : CURRENT_DISPLAY_SMOOTH_TAU_HIGH_S;
  const float alpha = (dtS <= 0.0f) ? 1.0f : fminf(1.0f, dtS / fmaxf(0.02f, tau));

  dispIrms += alpha * (irms - dispIrms);

  if (displayGateOn) {
    if (dispIrms < CURRENT_DISPLAY_GATE_OFF_A) displayGateOn = false;
  } else {
    if (dispIrms > CURRENT_DISPLAY_GATE_ON_A) displayGateOn = true;
  }

  if (!displayGateOn) dispIrms = 0.0f;
  return dispIrms;
}

static float cleanLogicCurrent(float irmsRaw, bool currentValid, float vRaw, float vProtect) {
  static uint32_t mainsGoneSinceMs = 0;
  const uint32_t now = millis();

  const bool rawGone = (vRaw <= MAINS_PRESENT_OFF_V);
  const bool protGone = (vProtect <= MAINS_PRESENT_OFF_V);
  if (rawGone || protGone) {
    if (mainsGoneSinceMs == 0) mainsGoneSinceMs = now;
    if ((now - mainsGoneSinceMs) >= 450UL) return 0.0f;
  } else {
    mainsGoneSinceMs = 0;
  }

  if (!currentValid) return 0.0f;
  return irmsRaw;
}

static void handleCueEvents(float vRaw, float vProtect, float irms, bool mainsPresent, bool paused, FaultState st) {
  static uint32_t lowSinceMs = 0;
  static uint32_t highSinceMs = 0;
  static uint32_t lastVoltWarnMs = 0;

  static float loadBaseA = 0.0f;
  static float prevIrms = 0.0f;
  static uint32_t stableSinceMs = 0;
  static uint32_t lastLoadCueMs = 0;

  const uint32_t now = millis();
  if (paused) return;

  const bool stateNormal = (st == STATE_NORMAL);

  const float vVote = 0.65f * vProtect + 0.35f * vRaw;
  const bool lowWarn  = (vVote > VOLT_UNDERVOLT_MIN_V && vVote < VOLT_UNDERVOLT_MAX_V) &&
                        (vRaw < (VOLT_UNDERVOLT_MAX_V + 8.0f));
  const bool highWarn = (vVote >= VOLT_OVERVOLT_TRIP_V) &&
                        (vRaw >= (VOLT_OVERVOLT_TRIP_V - 8.0f));

  if (lowWarn) {
    if (lowSinceMs == 0) lowSinceMs = now;
  } else lowSinceMs = 0;

  if (highWarn) {
    if (highSinceMs == 0) highSinceMs = now;
  } else highSinceMs = 0;

  if (lowSinceMs && (now - lowSinceMs >= 1500U) && (now - lastVoltWarnMs >= 8000U)) {
    lastVoltWarnMs = now;
    actuators.notify(SND_VOLT_LOW);
  }
  if (highSinceMs && (now - highSinceMs >= 1000U) && (now - lastVoltWarnMs >= 8000U)) {
    lastVoltWarnMs = now;
    actuators.notify(SND_VOLT_HIGH);
  }

  static uint32_t mainsStableSinceMs = 0;

  if (!mainsPresent || vVote < 200.0f || !stateNormal) {
    loadBaseA = 0.0f;
    prevIrms = irms;
    stableSinceMs = 0;
    mainsStableSinceMs = 0;
    return;
  }

  if (mainsStableSinceMs == 0) mainsStableSinceMs = now;

  const float dI = fabsf(irms - prevIrms);
  prevIrms = irms;
  if (dI < 0.08f) {
    if (stableSinceMs == 0) stableSinceMs = now;
  } else stableSinceMs = 0;

  if (stableSinceMs && (now - stableSinceMs >= DEVICE_PLUG_STABLE_MS)) {
    loadBaseA += 0.02f * (irms - loadBaseA);
  }

  const bool cueAllowed = ((now - mainsStableSinceMs) >= DEVICE_PLUG_CUE_INHIBIT_MS);
  const bool firstLoad = (loadBaseA < 0.15f && irms >= 0.35f);
  const bool addedLoad = (loadBaseA >= 0.15f && (irms - loadBaseA) >= 0.45f && dI < 0.15f);
  if (cueAllowed && (firstLoad || addedLoad) && (now - lastLoadCueMs >= DEVICE_PLUG_CUE_COOLDOWN_MS)) {
    actuators.notify(SND_DEVICE_PLUG);
    lastLoadCueMs = now;
    loadBaseA = irms;
    stableSinceMs = 0;
  }
}


static void pollPortalControl(CloudHandler& cloud, NetworkManager& net, bool paused, bool portalActive) {
  static uint32_t lastPoll = 0;
  static String lastToken = "";
  static bool tokenPrimed = false;

  if (paused || portalActive || !net.isConnected() || !cloud.isReady()) return;
  if (millis() - lastPoll < 1500UL) return;
  lastPoll = millis();

  String token;
  if (!cloud.getString("/controls/open_portal_token", token)) return;
  token.trim();
  if (!token.length()) return;

  // Prime from the first cloud read after boot so an old token does not
  // reopen the AP every restart.
  if (!tokenPrimed) {
    lastToken = token;
    tokenPrimed = true;
    return;
  }

  if (token == lastToken) return;

  lastToken = token;
  net.requestPortal(false);
}

static void pollRelayControl(CloudHandler& cloud, NetworkManager& net, Actuators& actuators, bool paused, bool portalActive, bool controlsLocked) {
  static uint32_t lastPoll = 0;
  static String lastOnToken = "";
  static String lastOffToken = "";
  static bool primed = false;

  if (paused || portalActive || !net.isConnected() || !cloud.isReady()) return;
  if ((millis() - lastPoll) < 700UL) return;
  lastPoll = millis();

  String onToken;
  String offToken;
  cloud.getString("/controls/relay_on_token", onToken);
  cloud.getString("/controls/relay_off_token", offToken);
  onToken.trim();
  offToken.trim();

  if (!primed) {
    lastOnToken = onToken;
    lastOffToken = offToken;
    primed = true;
    return;
  }

  if (!controlsLocked && onToken.length() && onToken != lastOnToken) {
    lastOnToken = onToken;
    actuators.pulseRelayOn();
    actuators.notify(SND_RESET_ACK);
  } else if (onToken.length()) {
    lastOnToken = onToken;
  }

  if (!controlsLocked && offToken.length() && offToken != lastOffToken) {
    lastOffToken = offToken;
    actuators.pulseRelayOff();
    actuators.notify(SND_RESET_ACK);
  } else if (offToken.length()) {
    lastOffToken = offToken;
  }
}


static bool pollFaultClearControl(CloudHandler& cloud, NetworkManager& net, bool paused, bool portalActive) {
  static uint32_t lastPoll = 0;
  static String lastToken = "";
  static bool primed = false;

  if (paused || portalActive || !net.isConnected() || !cloud.isReady()) return false;
  if ((millis() - lastPoll) < 700UL) return false;
  lastPoll = millis();

  String token;
  if (!cloud.getString("/controls/fault_clear_token", token)) return false;
  token.trim();

  if (!primed) {
    lastToken = token;
    primed = true;
    return false;
  }

  if (!token.length() || token == lastToken) return false;
  lastToken = token;
  return true;
}
static void onOtaEvent(OtaEvent ev, int progress) {
  if (ev == OtaEvent::START) {
    gPauseByOta = true;
    oled.setOta(true, 0);
    actuators.notify(SND_OTA_START);
  } else if (ev == OtaEvent::PROGRESS) {
    oled.setOta(true, (uint8_t)constrain(progress, 0, 100));
  } else if (ev == OtaEvent::SUCCESS) {
    oled.setOta(true, 100);
    actuators.notify(SND_OTA_OK);
    cloud.logStatusEvent("FIRMWARE UPDATED", 0.0f, 0.0f, 0.0f, 0.0f, &timeSync);
  } else if (ev == OtaEvent::FAIL) {
    gPauseByOta = false;
    oled.setOta(false, 0);
    actuators.notify(SND_OTA_FAIL);
  }
}

static void Core0Task(void* pv) {
  (void)pv;
  ArcFeatOut out;

  while (millis() < SENSOR_BOOT_SETTLE_MS) vTaskDelay(20 / portTICK_PERIOD_MS);

  while (true) {
    FeatureFrame f = {};
    f.uptime_ms = millis();

    float fs_hz = FS_TARGET_HZ;
    size_t got = curSensor.capture(s_raw, N_SAMP, &fs_hz);
    if (got != N_SAMP || fs_hz < CURRENT_FRAME_MIN_FS_HZ) {
      vTaskDelay(1);
      fs_hz = FS_TARGET_HZ;
      got = curSensor.capture(s_raw, N_SAMP, &fs_hz);
    }

    bool ok = false;
    if (got == N_SAMP && fs_hz >= CURRENT_FRAME_MIN_FS_HZ) {
      ok = arcFeat.compute(s_raw, N_SAMP, fs_hz, curCalib, MAINS_F0_HZ, out);
    }

    if (ok && out.current_valid) {
      f.adc_fs_hz = out.fs_hz;
      f.irms = out.irms_a;
      f.current_valid = 1;
      f.feat_valid = out.feat_valid ? 1 : 0;

      if (out.feat_valid) {
        f.cycle_nmse            = out.cycle_nmse;
        f.zcv                   = out.zcv;
        f.zc_dwell_ratio        = out.zc_dwell_ratio;
        f.pulse_count_per_cycle = out.pulse_count_per_cycle;
        f.peak_fluct_cv         = out.peak_fluct_cv;
        f.midband_residual_rms  = out.midband_residual_rms;
        f.hf_band_energy_ratio  = out.hf_band_energy_ratio;
        f.wpe_entropy           = out.wpe_entropy;
        f.spec_entropy          = out.spec_entropy;
        f.thd_i                 = out.thd_i;
      }
    }

    if (qFeat) xQueueOverwrite(qFeat, &f);
    vTaskDelay(1);
  }
}

#if ENABLE_ML_LOGGER
static void pollMlControl(CloudHandler& cloud, DataLogger& logger, NotificationOLED& oledUi) {
  static uint32_t lastPoll = 0;
  static bool lastEnabled = false;

  if (millis() - lastPoll < ML_CONTROL_FAST_POLL_MS) return;
  lastPoll = millis();

  bool enabled = false;
  int dur = 10;
  int labelOv = -1;
  String sid = "";
  String load = "unknown";

  cloud.getBool("/ml_log/enabled", enabled);
  cloud.getInt("/ml_log/duration_s", dur);
  cloud.getInt("/ml_log/label_override", labelOv);
  cloud.getString("/ml_log/session_id", sid);
  cloud.getString("/ml_log/load_type", load);

  if (dur < 5) dur = 5;
  if (dur > 60) dur = 60;

  if (enabled && sid.length() < 3) {
    logger.setEnabled(false);
    lastEnabled = false;
    return;
  }

  logger.setDurationSeconds((uint16_t)dur);
  logger.setSession(sid, load, labelOv);
  logger.setEnabled(enabled);

  if (enabled && !lastEnabled) {
    actuators.notify(SND_LOGGER_ON);
    oledUi.triggerCollecting(1000);
  }
  lastEnabled = enabled;
}
#endif

#if ENABLE_ML_LOGGER
static void maybeStartAutoArcCapture(DataLogger& logger, NotificationOLED& oledUi, bool paused, bool safeMode, bool modelPositive) {
  static bool lastPositive = false;
  static uint32_t lastAutoStartMs = 0;

  const bool rising = (modelPositive && !lastPositive);
  lastPositive = modelPositive;

#if ENABLE_AUTO_ARC_CAPTURE
  if (!paused && !safeMode && !logger.manualEnabled() && rising) {
    const uint32_t now = millis();
    if ((now - lastAutoStartMs) >= AUTO_ARC_CAPTURE_COOLDOWN_MS) {
      if (logger.startAutoCapture("model_arc", AUTO_ARC_CAPTURE_DURATION_S)) {
        lastAutoStartMs = now;
        actuators.notify(SND_LOGGER_ON);
        oledUi.triggerCollecting(1000);
      }
    }
  }
#else
  (void)logger; (void)oledUi; (void)paused; (void)safeMode; (void)modelPositive;
#endif
}
#endif

void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);

  BootGuard::begin(12000, 3);
  gSafeMode = BootGuard::safeMode();

  oled.begin();
  oled.startBootSequence(3600);
  actuators.begin(PIN_LATCH_ON, PIN_LATCH_OFF, PIN_BUZZER_PWM, &oled);
  actuators.notify(SND_BOOT);

  voltSensor.begin();
  voltSensor.setWindowMs(220);
  voltSensor.setClampHysteresis(15.0f, 25.0f);
  voltSensor.setLongAverage(18.0f, 25.0f);
  voltSensor.setCubicCalib(VOLTAGE_CAL_C3, VOLTAGE_CAL_C2, VOLTAGE_CAL_C1, VOLTAGE_CAL_C0);

  curCalib.cubic3 = CURRENT_CAL_C3;
  curCalib.cubic2 = CURRENT_CAL_C2;
  curCalib.cubic1 = CURRENT_CAL_C1;
  curCalib.cubic0 = CURRENT_CAL_C0;
  curSensor.setCalib(curCalib);

  tempSensor.begin();
  tempSensor.setLongAverage(8.0f, 1.0f);

  if (!gSafeMode) {
    if (curSensor.begin()) {
      qFeat = xQueueCreate(1, sizeof(FeatureFrame));
      xTaskCreatePinnedToCore(Core0Task, "Core0Sense", 16384, nullptr, 3, nullptr, 0);

      char msg[24];
      snprintf(msg, sizeof(msg), "%s ready", currentBackendName());
      oled.showStatus("Sensors", msg);
    } else {
      char msg[24];
      snprintf(msg, sizeof(msg), "No %s", currentBackendName());
      oled.showStatus("WARN", msg);
      delay(700);
    }
  } else {
    oled.showStatus("SAFE MODE", "OTA only");
  }

  net.begin([](WiFiManager* wm) {
    (void)wm;
  });

  cloud.begin(API_KEY, DATABASE_URL);
  cloud.setFirmwareVersion(FW_VERSION);
  cloud.setNormalIntervalMs(6000);
  cloud.setFaultIntervalMs(1000);
  timeSync.begin();

#if ENABLE_ML_LOGGER
  logger.begin(&cloud);
  logger.setEnabled(false);
  logger.setDurationSeconds(10);
#endif

  ota.begin(FW_VERSION, &cloud);
  ota.setEventCallback(onOtaEvent);
  ota.setPaths(OTA_DESIRED_VERSION_PATH, OTA_FIRMWARE_URL_PATH);
  ota.setCheckInterval(OTA_CHECK_INTERVAL_MS);

  if (!gSafeMode) (void)BootGuard::confirmNow();

  if (gSafeMode) oled.showStatus("SAFE MODE", "OTA only");
}

void loop() {
  BootGuard::loop();
  net.update();
  ota.loop();
  timeSync.update();

  const bool portalActive = net.inConfigPortal();
  const bool paused = gPauseByOta;
  const bool bootSettling = (millis() < SENSOR_BOOT_SETTLE_MS);
  const bool protectionInhibit = (millis() < (SENSOR_BOOT_SETTLE_MS + PROTECTION_INHIBIT_MS));

  static bool lastWiFiConnected = false;
  static NetworkManager::Phase lastWiFiPhase = NetworkManager::PHASE_BOOT_BLOCK;
  static uint32_t wifiBannerUntilMs = 0;
  static bool wifiTimedOutUi = false;

  const NetworkManager::Phase wifiPhase = net.phase();
  const bool wifiConnected = net.isConnected();

  if (wifiPhase != lastWiFiPhase) {
    if (wifiPhase == NetworkManager::PHASE_CONNECTING) {
      wifiBannerUntilMs = millis() + 2500UL;
      wifiTimedOutUi = false;
    } else if (wifiPhase == NetworkManager::PHASE_TIMEOUT) {
      wifiBannerUntilMs = millis() + 2500UL;
      wifiTimedOutUi = true;
    } else if (wifiPhase == NetworkManager::PHASE_CONNECTED) {
      wifiBannerUntilMs = 0;
      wifiTimedOutUi = false;
      oled.triggerConnected(1500UL);
    } else if (wifiPhase == NetworkManager::PHASE_AP_WAIT_CLIENT) {
      wifiBannerUntilMs = 0;
      wifiTimedOutUi = false;
      actuators.notify(SND_WIFI_PORTAL);
    } else if (wifiPhase == NetworkManager::PHASE_PORTAL_ACTIVE) {
      wifiBannerUntilMs = 0;
      wifiTimedOutUi = false;
    }
    lastWiFiPhase = wifiPhase;
  }

  if (wifiConnected && !lastWiFiConnected) actuators.notify(SND_WIFI_OK);
  lastWiFiConnected = wifiConnected;


  static FeatureFrame lastF = {};
  static bool hasLast = false;
  static uint32_t lastFeatRxMs = 0;
  FeatureFrame f;
  bool gotNewFeat = false;
  if (qFeat && xQueueReceive(qFeat, &f, 0) == pdTRUE) {
    lastF = f;
    hasLast = true;
    lastFeatRxMs = millis();
    gotNewFeat = true;
  } else if (hasLast) {
    f = lastF;
    if ((millis() - lastFeatRxMs) > FEAT_STALE_MS) {
      f.irms = 0.0f;
      f.current_valid = 0;
      f.feat_valid = 0;
      f.model_pred = 0;
      f.adc_fs_hz = 0.0f;
      f.cycle_nmse = 0.0f;
      f.zcv = 0.0f;
      f.zc_dwell_ratio = 0.0f;
      f.pulse_count_per_cycle = 0.0f;
      f.peak_fluct_cv = 0.0f;
      f.midband_residual_rms = 0.0f;
      f.hf_band_energy_ratio = 0.0f;
      f.wpe_entropy = 0.0f;
      f.spec_entropy = 0.0f;
      f.thd_i = 0.0f;
      lastF = f;
    }
  } else {
    memset(&f, 0, sizeof(f));
    f.uptime_ms = millis();
  }

  static float vRms = 0.0f;
  static float vFast = 0.0f;
  static float vRaw = 0.0f;
  static float tC = 0.0f;
  static uint32_t tTemp = 0;

  float newV = voltSensor.update();
  if (newV >= 0.0f) {
    vRms = newV;
    vFast = voltSensor.protectVrms();
    vRaw = voltSensor.rawVrms();
  }

  if (millis() - tTemp >= 500) {
    tTemp = millis();
    float newT = tempSensor.readTempC();
    if (newT > -50.0f && newT < 150.0f) tC = newT;
  }


  f.vrms = vRms;
  f.temp_c = tC;

  const float irmsRawMeasured = f.irms;
  const float irmsRawForLogic = cleanLogicCurrent(irmsRawMeasured, f.current_valid != 0, vRaw, vFast);
  f.irms = cleanDisplayCurrent(irmsRawForLogic, f.current_valid != 0, f.feat_valid != 0, vRaw, vFast);

  if (irmsRawForLogic < FEATURE_MIN_IRMS_A) {
    f.feat_valid = 0;
    f.cycle_nmse = 0.0f;
    f.zcv = 0.0f;
    f.zc_dwell_ratio = 0.0f;
    f.pulse_count_per_cycle = 0.0f;
    f.peak_fluct_cv = 0.0f;
    f.midband_residual_rms = 0.0f;
    f.hf_band_energy_ratio = 0.0f;
    f.wpe_entropy = 0.0f;
    f.spec_entropy = 0.0f;
    f.thd_i = 0.0f;
  }

  float apparentPowerVa = 0.0f;
  if (vRms > 0.10f && f.irms > 0.001f) apparentPowerVa = vRms * f.irms;

  const bool arcEligible = (!gSafeMode && !paused && !bootSettling && !protectionInhibit &&
                            arcInputStable(f.current_valid, f.feat_valid, irmsRawForLogic) &&
                            vFast >= VOLT_UNDERVOLT_MAX_V &&
                            vFast < VOLT_OVERVOLT_TRIP_V);

  const bool faultClearRequested = (!gSafeMode) ? pollFaultClearControl(cloud, net, paused, portalActive) : false;
  if (faultClearRequested) {
    faultLogic.resetLatch();
    actuators.notify(SND_RESET_ACK);
  }

  int pred = 0;
  if (arcEligible) {
    pred = ArcPredict(
      f.cycle_nmse, f.zcv, f.zc_dwell_ratio,
      f.pulse_count_per_cycle, f.peak_fluct_cv,
      f.midband_residual_rms, f.hf_band_energy_ratio,
      f.wpe_entropy, f.spec_entropy, f.thd_i,
      f.vrms, irmsRawForLogic, f.temp_c
    );
  }
  f.model_pred = (uint8_t)pred;

  FaultState st = STATE_NORMAL;
  if (!bootSettling) {
    st = faultLogic.update(vFast, vRaw, tC, irmsRawForLogic, f.model_pred, arcEligible);
  }

  if (gSafeMode) st = STATE_NORMAL;

  static uint32_t noPowerSinceMsCtl = 0;
  const bool unpluggedLiveCtl = classifyUnpluggedSocket(vRaw, vFast, irmsRawForLogic, f.current_valid != 0, st, &noPowerSinceMsCtl);
  const bool controlsLocked = gSafeMode || paused || bootSettling || protectionInhibit || unpluggedLiveCtl || faultLogic.webControlLocked() || faultLogic.voltageLockoutActive();

  if (!gSafeMode) {
    pollPortalControl(cloud, net, paused, portalActive);
    pollRelayControl(cloud, net, actuators, paused, portalActive, controlsLocked);
  }

  const bool tripOffEdge = faultLogic.consumeTripOffEdge();
  const bool autoOnEdge  = faultLogic.consumeAutoOnEdge();
  if (tripOffEdge) actuators.pulseRelayOff();
  if (autoOnEdge && !controlsLocked) actuators.pulseRelayOn();

  actuators.apply(st, vRms, vFast, irmsRawForLogic, tC);
  const bool mainsPresentStable = debouncedMainsPresentForState(vFast);
  handleCueEvents(vRaw, vFast, irmsRawForLogic, mainsPresentStable, paused || gSafeMode || protectionInhibit, st);

#if ENABLE_ML_LOGGER
  maybeStartAutoArcCapture(logger, oled, paused || protectionInhibit, gSafeMode, (arcEligible && f.model_pred == 1));
  if (!paused && !gSafeMode) pollMlControl(cloud, logger, oled);
  else logger.setEnabled(false);
#endif

  if (!paused && !gSafeMode) {
#if ENABLE_ML_LOGGER
    static uint32_t lastMl = 0;
    const uint32_t period = 1000UL / ML_LOG_RATE_HZ;
    if (millis() - lastMl >= period) {
      lastMl = millis();
      f.epoch_ms = timeSync.isSynced() ? timeSync.nowEpochMs() : 0;
      FeatureFrame fLog = f;
      fLog.irms = irmsRawForLogic;
      logger.ingest(fLog, st, faultLogic.arcCounter());
    }
#endif
  }

  static FaultState displayFaultState = STATE_NORMAL;
  static uint32_t displayFaultUntil = 0;
  if (faultClearRequested) {
    displayFaultState = STATE_NORMAL;
    displayFaultUntil = 0;
  }
  if (st != STATE_NORMAL) {
    displayFaultState = st;
    displayFaultUntil = millis() + FAULT_ALERT_MIN_MS;
  } else if (displayFaultState != STATE_NORMAL && (int32_t)(millis() - displayFaultUntil) >= 0) {
    displayFaultState = STATE_NORMAL;
    displayFaultUntil = 0;
  }
  const bool displayFaultActive = (displayFaultState != STATE_NORMAL) && ((int32_t)(displayFaultUntil - millis()) > 0);

  static uint32_t noPowerSinceMsOled = 0;
  const bool unpluggedLiveOled = classifyUnpluggedSocket(vRaw, vFast, irmsRawForLogic, f.current_valid != 0, st, &noPowerSinceMsOled);

  const FaultState oledState = displayFaultActive ? displayFaultState : st;
  OledOverlay ov = OledOverlay::NONE;
  if (!gPauseByOta && !bootSettling) {
    if (unpluggedLiveOled && oledState != STATE_ARCING && oledState != STATE_HEATING) {
      ov = OledOverlay::UNPLUGGED;
    } else if (displayFaultActive) {
      if (oledState == STATE_HEATING) ov = OledOverlay::FAULT_HEAT;
      else if (oledState == STATE_ARCING) ov = OledOverlay::FAULT_ARC;
      else if (oledState == STATE_OVERVOLTAGE) ov = OledOverlay::FAULT_OVERVOLT;
      else if (oledState == STATE_UNDERVOLTAGE) ov = OledOverlay::FAULT_UNDERVOLT;
      else if (oledState == STATE_OVERLOAD || oledState == STATE_SUSTAINED_OVERLOAD) ov = OledOverlay::FAULT_OVERLOAD;
    }
  }

  oled.setOverlay(ov);
  oled.setState(oledState);
  oled.setMeasurements(vRms, f.irms, apparentPowerVa, tC);
  const bool apWindowActive = (wifiPhase == NetworkManager::PHASE_AP_WAIT_CLIENT);
  const bool showWifiBanner = portalActive || apWindowActive || ((int32_t)(wifiBannerUntilMs - millis()) > 0);
  oled.setWiFi(wifiConnected, net.rssi(), showWifiBanner, portalActive, wifiTimedOutUi, apWindowActive);

  static uint32_t lastOled = 0;
  if (millis() - lastOled >= 80) {
    lastOled = millis();
    oled.render();
  }

  static FaultState lastImmediateFaultState = STATE_NORMAL;
  if (!gSafeMode && !paused && !bootSettling && cloud.isReady()) {
    if (st != STATE_NORMAL && (st != lastImmediateFaultState || tripOffEdge)) {
      FeatureFrame fHist = f;
      fHist.irms = irmsRawForLogic;
      (void)cloud.logFeatureEvent(String(stateToCstr(st)), fHist, apparentPowerVa, tripOffEdge, &timeSync);
    }
    lastImmediateFaultState = st;
  }

  static uint32_t lastLive = 0;
  if (millis() - lastLive > 1000) {
    lastLive = millis();

    static uint32_t noPowerSinceMs = 0;
    const bool unpluggedLive = classifyUnpluggedSocket(vRaw, vFast, irmsRawForLogic, f.current_valid != 0, st, &noPowerSinceMs);

    String stateStr;
    if (gPauseByOta) stateStr = "OTA_UPDATING";
    else if (portalActive) stateStr = "CONFIG_PORTAL";
    else if (bootSettling || protectionInhibit) stateStr = "STARTUP_STABILIZING";
    else if (gSafeMode) stateStr = "SAFE_MODE";
    else if (unpluggedLive && st != STATE_ARCING && st != STATE_HEATING) stateStr = "UNPLUGGED";
    else stateStr = String(stateToCstr(st));

    cloud.update(vRms, f.irms, apparentPowerVa, tC,
                 f.cycle_nmse, f.zcv, f.zc_dwell_ratio,
                 f.pulse_count_per_cycle, f.peak_fluct_cv,
                 f.midband_residual_rms, f.hf_band_energy_ratio,
                 f.wpe_entropy, f.spec_entropy, f.thd_i,
                 f.model_pred,
                 stateStr, &timeSync);
  }

#if ENABLE_ML_LOGGER
  if (!paused && !gSafeMode) {
    logger.loop();
  }
#endif

  delay(2);
}