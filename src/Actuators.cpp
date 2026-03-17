#include "Actuators.h"
#include "OLED_NOTIF.h"
#include "SmartPlugConfig.h"

static constexpr uint8_t  BUZZ_CH = 0;

struct ToneStep {
  uint16_t hz;
  uint16_t dur_ms;
  uint8_t duty;
};
struct TonePattern {
  const ToneStep* steps;
  uint8_t count;
  bool repeat;
  uint8_t priority;
};

static constexpr ToneStep P_BOOT[] = {
  {1850, 80, 112}, {0, 40, 0}, {2350, 90, 120}, {0, 40, 0}, {2950, 130, 128}
};
static constexpr ToneStep P_WIFI_PORTAL[] = {
  {880, 120, 112}, {0, 70, 0}, {1175, 120, 116}, {0, 70, 0}, {1568, 150, 120}
};
static constexpr ToneStep P_WIFI_OK[] = {
  {1800, 60, 110}, {0, 25, 0}, {2400, 70, 118}, {0, 25, 0}, {3200, 110, 126}
};
static constexpr ToneStep P_LOGGER_ON[] = {
  {1150, 55, 104}, {0, 35, 0}, {1500, 55, 110}, {0, 35, 0}, {1950, 85, 118}
};
static constexpr ToneStep P_OTA_START[] = {
  {1500, 80, 112}, {0, 30, 0}, {1300, 90, 112}, {0, 30, 0}, {1100, 120, 118}
};
static constexpr ToneStep P_OTA_OK[] = {
  {1300, 60, 110}, {0, 25, 0}, {1800, 60, 118}, {0, 25, 0}, {2500, 140, 126}
};
static constexpr ToneStep P_OTA_FAIL[] = {
  {520, 200, 132}, {0, 80, 0}, {420, 240, 136}, {0, 80, 0}, {320, 320, 140}
};
static constexpr ToneStep P_MAINS_LOST[] = {
  {430, 240, 128}, {0, 90, 0}, {330, 320, 128}
};
static constexpr ToneStep P_MAINS_RESTORED[] = {
  {1700, 70, 110}, {0, 30, 0}, {2300, 80, 118}, {0, 30, 0}, {3000, 110, 126}
};
static constexpr ToneStep P_VOLT_LOW[] = {
  {700, 180, 120}, {0, 70, 0}, {620, 220, 124}
};
static constexpr ToneStep P_VOLT_HIGH[] = {
  {2500, 120, 118}, {0, 60, 0}, {2900, 150, 124}
};
static constexpr ToneStep P_DEVICE_PLUG[] = {
  {1400, 55, 108}, {0, 25, 0}, {1750, 85, 116}
};
static constexpr ToneStep P_FAULT_ARC[] = {
  {4100, 28, 132}, {0, 18, 0}, {3450, 34, 132}, {0, 20, 0},
  {3900, 30, 132}, {0, 24, 0}, {3000, 48, 132}, {0, 140, 0}
};
static constexpr ToneStep P_FAULT_HEAT[] = {
  {1650, 180, 126}, {0, 70, 0}, {1650, 180, 126}, {0, 70, 0}, {1650, 180, 126},
  {0, 220, 0}
};
static constexpr ToneStep P_FAULT_OVER[] = {
  {2450, 120, 118}, {0, 100, 0}, {2450, 120, 118}, {0, 280, 0}
};
static constexpr ToneStep P_FAULT_OVER_HARD[] = {
  {1650, 180, 138}, {2450, 180, 138}, {1650, 180, 138}, {2450, 180, 138}, {0, 80, 0}
};
static constexpr ToneStep P_RESET_ACK[] = {
  {1200, 80, 110}, {0, 40, 0}, {1200, 80, 110}
};

static constexpr TonePattern PATTERNS[] = {
  { P_BOOT,             (uint8_t)(sizeof(P_BOOT) / sizeof(P_BOOT[0])),                       false, 1 },
  { P_WIFI_PORTAL,      (uint8_t)(sizeof(P_WIFI_PORTAL) / sizeof(P_WIFI_PORTAL[0])),         false, 1 },
  { P_WIFI_OK,          (uint8_t)(sizeof(P_WIFI_OK) / sizeof(P_WIFI_OK[0])),                 false, 1 },
  { P_LOGGER_ON,        (uint8_t)(sizeof(P_LOGGER_ON) / sizeof(P_LOGGER_ON[0])),             false, 1 },
  { P_OTA_START,        (uint8_t)(sizeof(P_OTA_START) / sizeof(P_OTA_START[0])),             false, 2 },
  { P_OTA_OK,           (uint8_t)(sizeof(P_OTA_OK) / sizeof(P_OTA_OK[0])),                   false, 2 },
  { P_OTA_FAIL,         (uint8_t)(sizeof(P_OTA_FAIL) / sizeof(P_OTA_FAIL[0])),               false, 2 },
  { P_MAINS_LOST,       (uint8_t)(sizeof(P_MAINS_LOST) / sizeof(P_MAINS_LOST[0])),           false, 2 },
  { P_MAINS_RESTORED,   (uint8_t)(sizeof(P_MAINS_RESTORED) / sizeof(P_MAINS_RESTORED[0])),   false, 2 },
  { P_VOLT_LOW,         (uint8_t)(sizeof(P_VOLT_LOW) / sizeof(P_VOLT_LOW[0])),               false, 2 },
  { P_VOLT_HIGH,        (uint8_t)(sizeof(P_VOLT_HIGH) / sizeof(P_VOLT_HIGH[0])),             false, 2 },
  { P_DEVICE_PLUG,      (uint8_t)(sizeof(P_DEVICE_PLUG) / sizeof(P_DEVICE_PLUG[0])),         false, 1 },
  { P_FAULT_ARC,        (uint8_t)(sizeof(P_FAULT_ARC) / sizeof(P_FAULT_ARC[0])),             true,  4 },
  { P_FAULT_HEAT,       (uint8_t)(sizeof(P_FAULT_HEAT) / sizeof(P_FAULT_HEAT[0])),           true,  5 },
  { P_FAULT_OVER,       (uint8_t)(sizeof(P_FAULT_OVER) / sizeof(P_FAULT_OVER[0])),           true,  3 },
  { P_FAULT_OVER_HARD,  (uint8_t)(sizeof(P_FAULT_OVER_HARD) / sizeof(P_FAULT_OVER_HARD[0])), true,  6 },
  { P_RESET_ACK,        (uint8_t)(sizeof(P_RESET_ACK) / sizeof(P_RESET_ACK[0])),             false, 1 },
};

static int s_buzzPin = -1;
static bool s_pwmReady = false;
static uint8_t s_activeId = 255;
static uint8_t s_step = 0;
static uint32_t s_t0 = 0;
static uint8_t s_activePrio = 0;
static uint8_t s_lastFaultSound = 255;

static inline bool isFaultPattern(uint8_t id) {
  return id == SND_FAULT_ARC || id == SND_FAULT_HEAT || id == SND_FAULT_OVER || id == SND_FAULT_OVER_HARD;
}

static inline bool startupMuteActive() {
  return millis() < BUZZER_STARTUP_MUTE_MS;
}

static inline bool statusSoundAllowed() {
  if (!BUZZER_STATUS_ENABLED) return false;
  if (startupMuteActive()) return false;
  return true;
}

static inline bool eventSoundAllowed(uint8_t id) {
  if (isFaultPattern(id)) return true;
  return statusSoundAllowed();
}

static void pwmAttachIfNeeded(int pin) {
  if (pin < 0 || s_pwmReady) return;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(pin, 2000, BUZZER_PWM_BITS);
#else
  ledcSetup(BUZZ_CH, 2000, BUZZER_PWM_BITS);
  ledcAttachPin(pin, BUZZ_CH);
#endif
  s_pwmReady = true;
}

static void pwmStop() {
  if (!s_pwmReady || s_buzzPin < 0) return;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(s_buzzPin, 0);
  ledcWrite(s_buzzPin, 0);
#else
  ledcWriteTone(BUZZ_CH, 0);
  ledcWrite(BUZZ_CH, 0);
#endif
}

static void pwmTone(uint16_t hz, uint8_t duty) {
  if (!s_pwmReady || s_buzzPin < 0) return;
  if (hz == 0 || duty == 0) {
    pwmStop();
    return;
  }
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(s_buzzPin, hz);
  ledcWrite(s_buzzPin, duty);
#else
  ledcWriteTone(BUZZ_CH, hz);
  ledcWrite(BUZZ_CH, duty);
#endif
}

static void soundBegin(int pinBuzzer) {
  s_buzzPin = pinBuzzer;
  pwmAttachIfNeeded(pinBuzzer);
  pwmStop();
}

static void soundStop() {
  pwmStop();
  s_activeId = 255;
  s_step = 0;
  s_t0 = 0;
  s_activePrio = 0;
}

static void soundStart(uint8_t id) {
  if (id >= (sizeof(PATTERNS) / sizeof(PATTERNS[0]))) return;
  if (s_activeId == id) return;
  if (!eventSoundAllowed(id)) return;

  const TonePattern& p = PATTERNS[id];
  if (s_activeId != 255 && p.priority < s_activePrio) return;

  s_activeId = id;
  s_activePrio = p.priority;
  s_step = 0;
  s_t0 = 0;
}

static void soundLoop() {
  if (s_activeId == 255) return;

  const TonePattern& p = PATTERNS[s_activeId];
  const uint32_t now = millis();
  if (s_t0 == 0) {
    s_t0 = now;
    pwmTone(p.steps[s_step].hz, p.steps[s_step].duty);
    return;
  }
  if ((uint32_t)(now - s_t0) >= p.steps[s_step].dur_ms) {
    s_step++;
    if (s_step >= p.count) {
      if (p.repeat) s_step = 0;
      else {
        soundStop();
        return;
      }
    }
    s_t0 = 0;
  }
}

static inline float heatAlarmTempC() {
#ifdef DATA_COLLECTION_MODE
  return TEMP_DATA_WARN_C;
#else
  return TEMP_TRIP_C;
#endif
}

void Actuators::begin(int pinRelay, int pinBuzzer, OLED_NOTIF* oled) {
  _pinRelay = pinRelay;
  _pinBuzzer = pinBuzzer;
  _oled = oled;

  pinMode(_pinRelay, OUTPUT);
  setRelay(true);

  soundBegin(_pinBuzzer);
  soundStop();
}

void Actuators::setRelay(bool on) {
  if (_pinRelay < 0) return;
  const bool level = RELAY_ACTIVE_LOW ? !on : on;
  digitalWrite(_pinRelay, level ? HIGH : LOW);
}

void Actuators::notify(SoundEvent ev) {
  soundBegin(_pinBuzzer);
  soundStart((uint8_t)ev);
}

void Actuators::apply(FaultState st, float vDisplay, float vProtect, float i, float t) {
  (void)vDisplay;
  const bool surge = (vProtect >= VOLT_SURGE_TRIP_V);
  const bool heatWarn = (t >= heatAlarmTempC()) || (st == STATE_HEATING);
  const bool hardOver = (i >= OVERLOAD_HARD_TRIP_A);

  bool relayOn = true;
#ifdef DATA_COLLECTION_MODE
  relayOn = !(surge || (t >= TEMP_DATA_HARD_C));
#else
  if (surge || hardOver || heatWarn || st == STATE_ARCING) relayOn = false;
#endif
  setRelay(relayOn);

  uint8_t wantedFaultSound = 255;
  if (surge) wantedFaultSound = SND_FAULT_OVER_HARD;
  else if (heatWarn) wantedFaultSound = SND_FAULT_HEAT;
  else if (st == STATE_ARCING) wantedFaultSound = SND_FAULT_ARC;
  else if (hardOver) wantedFaultSound = SND_FAULT_OVER_HARD;
  else if (st == STATE_OVERLOAD || i >= OVERLOAD_WARN_A) wantedFaultSound = SND_FAULT_OVER;

  if (wantedFaultSound != s_lastFaultSound) {
    s_lastFaultSound = wantedFaultSound;
    if (wantedFaultSound == 255) {
      if (isFaultPattern(s_activeId)) soundStop();
    } else {
      soundStart(wantedFaultSound);
    }
  }

  static bool mainsWasOn = false;
  const bool mainsOn = (vProtect >= MAINS_PRESENT_ON_V);
  if (mainsWasOn && !mainsOn) notify(SND_MAINS_LOST);
  mainsWasOn = mainsOn;

  soundLoop();
}
