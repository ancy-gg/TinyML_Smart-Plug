#include "Actuators.h"
#include "OLED_NOTIF.h"
#include "SmartPlugConfig.h"

static constexpr uint32_t OLED_MS = 250;
static constexpr uint8_t  BUZZ_CH = 0;

struct ToneStep {
  uint16_t hz;
  uint16_t dur_ms;
};

struct TonePattern {
  const ToneStep* steps;
  uint8_t count;
  bool repeat;
  uint8_t priority;
};

// ---------- Pleasant/status sounds (higher freq, smoother for passive piezo) ----------
static constexpr ToneStep P_BOOT[] = {
  {2600, 70}, {3000, 70}, {3400, 90}, {3800, 120}
};

static constexpr ToneStep P_WIFI_PORTAL[] = {
  {2600, 80}, {3000, 90}, {3400, 110}, {0, 650}
};

static constexpr ToneStep P_WIFI_OK[] = {
  {2800, 70}, {3300, 80}, {3900, 130}
};

static constexpr ToneStep P_LOGGER_ON[] = {
  {2500, 60}, {3000, 60}, {3500, 100}
};

static constexpr ToneStep P_OTA_START[] = {
  {3600, 80}, {3200, 80}, {2800, 100}
};

static constexpr ToneStep P_OTA_OK[] = {
  {2700, 60}, {3200, 70}, {3800, 140}
};

static constexpr ToneStep P_OTA_FAIL[] = {
  {2800, 120}, {2300, 160}, {1900, 220}
};

static constexpr ToneStep P_MAINS_LOST[] = {
  {3000, 120}, {2500, 180}, {2100, 260}
};

static constexpr ToneStep P_DEVICE_PLUG[] = {
  {2900, 60}, {3400, 90}
};

static constexpr ToneStep P_RESET_ACK[] = {
  {3000, 70}, {3600, 80}
};

static constexpr TonePattern PATTERNS[] = {
  { P_BOOT,         (uint8_t)(sizeof(P_BOOT) / sizeof(P_BOOT[0])),               false, 0 },
  { P_WIFI_PORTAL,  (uint8_t)(sizeof(P_WIFI_PORTAL) / sizeof(P_WIFI_PORTAL[0])), true,  1 },
  { P_WIFI_OK,      (uint8_t)(sizeof(P_WIFI_OK) / sizeof(P_WIFI_OK[0])),         false, 1 },
  { P_LOGGER_ON,    (uint8_t)(sizeof(P_LOGGER_ON) / sizeof(P_LOGGER_ON[0])),     false, 1 },
  { P_OTA_START,    (uint8_t)(sizeof(P_OTA_START) / sizeof(P_OTA_START[0])),     false, 2 },
  { P_OTA_OK,       (uint8_t)(sizeof(P_OTA_OK) / sizeof(P_OTA_OK[0])),           false, 2 },
  { P_OTA_FAIL,     (uint8_t)(sizeof(P_OTA_FAIL) / sizeof(P_OTA_FAIL[0])),       false, 2 },
  { P_MAINS_LOST,   (uint8_t)(sizeof(P_MAINS_LOST) / sizeof(P_MAINS_LOST[0])),   false, 2 },
  { P_DEVICE_PLUG,  (uint8_t)(sizeof(P_DEVICE_PLUG) / sizeof(P_DEVICE_PLUG[0])), false, 1 },

  // Fault slots are dynamically generated in soundLoop():
  { nullptr, 0, true,  4 }, // SND_FAULT_ARC
  { nullptr, 0, true,  5 }, // SND_FAULT_HEAT
  { nullptr, 0, true,  3 }, // SND_FAULT_OVER
  { nullptr, 0, true,  5 }, // SND_FAULT_OVER_HARD

  { P_RESET_ACK,    (uint8_t)(sizeof(P_RESET_ACK) / sizeof(P_RESET_ACK[0])),     false, 1 },
};

static int s_buzzPin = -1;
static bool s_pwmReady = false;
static uint8_t s_activeId = 255;
static uint8_t s_step = 0;
static uint32_t s_t0 = 0;
static uint8_t s_activePrio = 0;
static uint8_t s_lastFaultSound = 255;
static uint16_t s_lastHz = 0;

static inline bool isFaultPattern(uint8_t id) {
  return id == SND_FAULT_ARC ||
         id == SND_FAULT_HEAT ||
         id == SND_FAULT_OVER ||
         id == SND_FAULT_OVER_HARD;
}

static void pwmAttachIfNeeded(int pin) {
  if (pin < 0 || s_pwmReady) return;

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(pin, 3000, BUZZER_PWM_BITS);
#else
  ledcSetup(BUZZ_CH, 3000, BUZZER_PWM_BITS);
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
  s_lastHz = 0;
}

static void pwmTone(uint16_t hz) {
  if (!s_pwmReady || s_buzzPin < 0) return;

  if (hz == 0) {
    pwmStop();
    return;
  }

  if (hz == s_lastHz) return;
  s_lastHz = hz;

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(s_buzzPin, hz);
  ledcWrite(s_buzzPin, BUZZER_PWM_DUTY);
#else
  ledcWriteTone(BUZZ_CH, hz);
  ledcWrite(BUZZ_CH, BUZZER_PWM_DUTY);
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
  s_lastHz = 0;
}

static uint16_t triSweepHz(uint32_t now, uint16_t lo, uint16_t hi, uint16_t periodMs) {
  if (hi <= lo || periodMs < 20) return lo;
  uint32_t t = now % periodMs;
  uint32_t half = periodMs / 2;
  if (half == 0) return lo;

  if (t < half) {
    return lo + (uint32_t)(hi - lo) * t / half;
  } else {
    t -= half;
    return hi - (uint32_t)(hi - lo) * t / half;
  }
}

static uint16_t twoToneHz(uint32_t now, uint16_t a, uint16_t b, uint16_t segMs) {
  if (segMs == 0) return a;
  return ((now / segMs) & 1U) ? b : a;
}

static uint16_t arcZapHz(uint32_t now) {
  // Fast jagged pattern, still continuous, to sound "electrical"
  static constexpr uint16_t HZ[] = { 4100, 3200, 3900, 2800, 4050, 3000, 3700, 2600 };
  static constexpr uint8_t N = sizeof(HZ) / sizeof(HZ[0]);
  uint32_t idx = (now / 28U) % N;
  return HZ[idx];
}

static void soundLoop() {
  if (s_activeId == 255) return;

  const uint32_t now = millis();

  // Dynamic fault sounds: continuous, smoother, less clicky
  if (isFaultPattern(s_activeId)) {
    switch (s_activeId) {
      case SND_FAULT_OVER:
        // Mild overload: soft warble, no gaps
        pwmTone(twoToneHz(now, 2700, 3200, 170));
        return;

      case SND_FAULT_OVER_HARD:
        // Hard overload / surge: ambulance style wang-wang
        pwmTone(twoToneHz(now, 2400, 3400, 110));
        return;

      case SND_FAULT_HEAT:
        // Fire alarm style: sweeping up/down continuously
        pwmTone(triSweepHz(now, 1800, 3600, 900));
        return;

      case SND_FAULT_ARC:
        // Crackly electrical style, but still continuous
        pwmTone(arcZapHz(now));
        return;

      default:
        break;
    }
  }

  // Sequenced one-shot / repeating status sounds
  const TonePattern& p = PATTERNS[s_activeId];
  if (p.steps == nullptr || p.count == 0) {
    soundStop();
    return;
  }

  if (s_t0 == 0) {
    s_t0 = now;
    pwmTone(p.steps[s_step].hz);
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
  _pinRelay  = pinRelay;
  _pinBuzzer = pinBuzzer;
  _oled      = oled;

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
  const bool surge    = (v >= VOLT_SURGE_TRIP_V);
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