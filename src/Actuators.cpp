#include "Actuators.h"
#include "OLED_NOTIF.h"
#include "SmartPlugConfig.h"

static constexpr uint32_t OLED_MS = 250;
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

// Pleasant status tones
static constexpr ToneStep P_BOOT[] = {
  {1046, 70, 110}, {0, 30, 0}, {1318, 70, 110}, {0, 30, 0}, {1568, 130, 120}
};
static constexpr ToneStep P_WIFI_PORTAL[] = {
  {740, 120, 90}, {0, 70, 0}, {988, 120, 90}, {0, 850, 0}
};
static constexpr ToneStep P_WIFI_OK[] = {
  {988, 70, 100}, {0, 30, 0}, {1318, 80, 105}, {0, 30, 0}, {1760, 120, 115}
};
static constexpr ToneStep P_LOGGER_ON[] = {
  {1200, 60, 95}, {0, 40, 0}, {1600, 60, 95}, {0, 40, 0}, {2000, 80, 100}
};
static constexpr ToneStep P_OTA_START[] = {
  {1700, 80, 100}, {0, 30, 0}, {1400, 90, 100}, {0, 30, 0}, {1100, 110, 100}
};
static constexpr ToneStep P_OTA_OK[] = {
  {1200, 60, 95}, {0, 25, 0}, {1600, 60, 105}, {0, 25, 0}, {2100, 160, 115}
};
static constexpr ToneStep P_OTA_FAIL[] = {
  {520, 180, 110}, {0, 70, 0}, {420, 220, 120}, {0, 70, 0}, {320, 280, 125}
};
static constexpr ToneStep P_MAINS_LOST[] = {
  {430, 300, 120}, {0, 120, 0}, {320, 420, 120}
};
static constexpr ToneStep P_DEVICE_PLUG[] = {
  {1500, 60, 95}, {0, 30, 0}, {1800, 90, 105}
};

// Fault tones: intentionally urgent / annoying
static constexpr ToneStep P_FAULT_ARC[] = {
  {1900, 70, 125}, {0, 30, 0}, {1200, 70, 120}, {0, 25, 0},
  {2100, 60, 125}, {0, 35, 0}, {900, 90, 120}, {0, 180, 0}
};
static constexpr ToneStep P_FAULT_HEAT[] = {
  {900, 180, 125}, {1200, 180, 125}, {0, 80, 0},
  {900, 180, 125}, {1200, 180, 125}, {0, 80, 0},
  {900, 180, 125}, {1200, 180, 125}, {0, 260, 0}
};
static constexpr ToneStep P_FAULT_OVER[] = {
  {950, 160, 110}, {0, 120, 0}, {950, 160, 110}, {0, 420, 0}
};
static constexpr ToneStep P_FAULT_OVER_HARD[] = {
  {760, 160, 125}, {1180, 160, 125}, {760, 160, 125}, {1180, 160, 125}, {0, 110, 0}
};
static constexpr ToneStep P_RESET_ACK[] = {
  {1200, 80, 100}, {0, 40, 0}, {1200, 80, 100}
};

static constexpr TonePattern PATTERNS[] = {
  { P_BOOT,             (uint8_t)(sizeof(P_BOOT) / sizeof(P_BOOT[0])),                   false, 0 },
  { P_WIFI_PORTAL,      (uint8_t)(sizeof(P_WIFI_PORTAL) / sizeof(P_WIFI_PORTAL[0])),     true,  1 },
  { P_WIFI_OK,          (uint8_t)(sizeof(P_WIFI_OK) / sizeof(P_WIFI_OK[0])),             false, 1 },
  { P_LOGGER_ON,        (uint8_t)(sizeof(P_LOGGER_ON) / sizeof(P_LOGGER_ON[0])),         false, 1 },
  { P_OTA_START,        (uint8_t)(sizeof(P_OTA_START) / sizeof(P_OTA_START[0])),         false, 2 },
  { P_OTA_OK,           (uint8_t)(sizeof(P_OTA_OK) / sizeof(P_OTA_OK[0])),               false, 2 },
  { P_OTA_FAIL,         (uint8_t)(sizeof(P_OTA_FAIL) / sizeof(P_OTA_FAIL[0])),           false, 2 },
  { P_MAINS_LOST,       (uint8_t)(sizeof(P_MAINS_LOST) / sizeof(P_MAINS_LOST[0])),       false, 2 },
  { P_DEVICE_PLUG,      (uint8_t)(sizeof(P_DEVICE_PLUG) / sizeof(P_DEVICE_PLUG[0])),     false, 1 },
  { P_FAULT_ARC,        (uint8_t)(sizeof(P_FAULT_ARC) / sizeof(P_FAULT_ARC[0])),         true,  4 },
  { P_FAULT_HEAT,       (uint8_t)(sizeof(P_FAULT_HEAT) / sizeof(P_FAULT_HEAT[0])),       true,  5 },
  { P_FAULT_OVER,       (uint8_t)(sizeof(P_FAULT_OVER) / sizeof(P_FAULT_OVER[0])),       true,  3 },
  { P_FAULT_OVER_HARD,  (uint8_t)(sizeof(P_FAULT_OVER_HARD) / sizeof(P_FAULT_OVER_HARD[0])), true, 5 },
  { P_RESET_ACK,        (uint8_t)(sizeof(P_RESET_ACK) / sizeof(P_RESET_ACK[0])),         false, 1 },
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
  setRelay(true); // default ON = AC conducting

  soundBegin(_pinBuzzer);
  soundStart(SND_BOOT);
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

void Actuators::apply(FaultState st, float v, float i, float t) {
  const bool surge = (v >= VOLT_SURGE_TRIP_V);
  const bool heatWarn = (t >= heatAlarmTempC()) || (st == STATE_HEATING);
  const bool hardOver = (i >= OVERLOAD_HARD_TRIP_A);

  bool relayOn = true;
#ifdef DATA_COLLECTION_MODE
  relayOn = !(surge || (t >= TEMP_DATA_HARD_C));
#else
  if (surge || hardOver || heatWarn || st == STATE_ARCING) relayOn = false;
  else relayOn = true; // mild overload intentionally stays ON
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
  const bool mainsOn = (v >= MAINS_PRESENT_ON_V);
  if (mainsWasOn && !mainsOn) notify(SND_MAINS_LOST);
  mainsWasOn = mainsOn;

  soundLoop();

  if (_oled && (millis() - _lastOled) >= OLED_MS) {
    _lastOled = millis();
    _oled->updateDashboard(v, i, t, st);
  }
}
