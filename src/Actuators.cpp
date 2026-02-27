#include "Actuators.h"
#include "OLED_NOTIF.h"
#include "SmartPlugConfig.h"

static constexpr uint32_t OLED_MS = 250;

// -------- Sound engine (non-blocking) --------
struct ToneStep {
  uint16_t hz;       // 0 = silence
  uint16_t dur_ms;
};

struct TonePattern {
  const ToneStep* steps;
  uint8_t count;
  bool repeat;
  uint8_t priority; // higher wins
};

static constexpr ToneStep P_BOOT[] = {
  { 880,  80 }, { 0, 40 }, { 1320, 80 }, { 0, 40 }, { 1760, 120 }
};

static constexpr ToneStep P_WIFI_PORTAL[] = {
  { 660, 120 }, { 0, 120 }, { 660, 120 }, { 0, 900 }
};

static constexpr ToneStep P_WIFI_OK[] = {
  { 1200, 120 }, { 0, 60 }, { 1600, 120 }
};

static constexpr ToneStep P_LOGGER_ON[] = {
  { 2000, 70 }, { 0, 60 }, { 2000, 70 }, { 0, 60 }, { 2000, 90 }
};

static constexpr ToneStep P_OTA_START[] = {
  { 1400, 120 }, { 0, 50 }, { 1000, 140 }, { 0, 50 }, { 700, 180 }
};

static constexpr ToneStep P_OTA_OK[] = {
  { 1800, 220 }
};

static constexpr ToneStep P_OTA_FAIL[] = {
  { 400, 250 }, { 0, 120 }, { 400, 250 }, { 0, 120 }, { 400, 350 }
};

static constexpr ToneStep P_MAINS_LOST[] = {
  { 320, 500 }
};

static constexpr ToneStep P_DEVICE_PLUG[] = {
  { 1500, 60 }, { 0, 40 }, { 1800, 60 }
};

static constexpr ToneStep P_FAULT_ARC[] = {
  { 1400, 160 }, { 0, 60 }, { 900, 160 }, { 0, 60 }
};

static constexpr ToneStep P_FAULT_HEAT[] = {
  { 800, 220 }, { 0, 180 }, { 800, 220 }, { 0, 900 }
};

static constexpr ToneStep P_FAULT_OVER[] = {
  { 600, 120 }, { 0, 120 }, { 600, 120 }, { 0, 600 }
};

static constexpr ToneStep P_RESET_ACK[] = {
  { 1200, 80 }, { 0, 40 }, { 1200, 80 }
};

// priority: FAULT=3, OTA=2, PORTAL=1, misc=0
static constexpr TonePattern PATTERNS[] = {
  { P_BOOT,        (uint8_t)(sizeof(P_BOOT)/sizeof(P_BOOT[0])),        false, 0 },
  { P_WIFI_PORTAL, (uint8_t)(sizeof(P_WIFI_PORTAL)/sizeof(P_WIFI_PORTAL[0])), true,  1 },
  { P_WIFI_OK,     (uint8_t)(sizeof(P_WIFI_OK)/sizeof(P_WIFI_OK[0])),  false, 0 },
  { P_LOGGER_ON,   (uint8_t)(sizeof(P_LOGGER_ON)/sizeof(P_LOGGER_ON[0])),false, 0 },
  { P_OTA_START,   (uint8_t)(sizeof(P_OTA_START)/sizeof(P_OTA_START[0])),false, 2 },
  { P_OTA_OK,      (uint8_t)(sizeof(P_OTA_OK)/sizeof(P_OTA_OK[0])),     false, 2 },
  { P_OTA_FAIL,    (uint8_t)(sizeof(P_OTA_FAIL)/sizeof(P_OTA_FAIL[0])), false, 2 },
  { P_MAINS_LOST,  (uint8_t)(sizeof(P_MAINS_LOST)/sizeof(P_MAINS_LOST[0])),false,0 },
  { P_DEVICE_PLUG, (uint8_t)(sizeof(P_DEVICE_PLUG)/sizeof(P_DEVICE_PLUG[0])),false,0 },
  { P_FAULT_ARC,   (uint8_t)(sizeof(P_FAULT_ARC)/sizeof(P_FAULT_ARC[0])),true,  3 },
  { P_FAULT_HEAT,  (uint8_t)(sizeof(P_FAULT_HEAT)/sizeof(P_FAULT_HEAT[0])),true,  3 },
  { P_FAULT_OVER,  (uint8_t)(sizeof(P_FAULT_OVER)/sizeof(P_FAULT_OVER[0])),true,  3 },
  { P_RESET_ACK,   (uint8_t)(sizeof(P_RESET_ACK)/sizeof(P_RESET_ACK[0])),false,0 },
};

static int s_ch = 0;
static bool s_pwmInit = false;

static uint8_t s_activeId = 255;
static uint8_t s_step = 0;
static uint32_t s_t0 = 0;
static uint8_t s_activePrio = 0;

static void soundBegin(int pinBuzzer) {
  if (s_pwmInit) return;
  ledcSetup(s_ch, 2400, 8);
  ledcAttachPin(pinBuzzer, s_ch);
  s_pwmInit = true;
}

static void soundStop() {
  if (!s_pwmInit) return;
  ledcWrite(s_ch, 0);
  s_activeId = 255;
  s_step = 0;
  s_t0 = 0;
  s_activePrio = 0;
}

static void soundStart(uint8_t id) {
  if (id >= (sizeof(PATTERNS)/sizeof(PATTERNS[0]))) return;

  const TonePattern& p = PATTERNS[id];

  // Priority: don’t interrupt higher priority pattern with lower one
  if (s_activeId != 255 && p.priority < s_activePrio) return;

  s_activeId = id;
  s_activePrio = p.priority;
  s_step = 0;
  s_t0 = 0; // triggers immediate step
}

static void soundLoop(int pinBuzzer) {
  if (s_activeId == 255) return;
  const TonePattern& p = PATTERNS[s_activeId];

  const uint32_t now = millis();
  if (s_t0 == 0) {
    s_t0 = now;
    const ToneStep& st = p.steps[s_step];
    if (st.hz == 0) {
      ledcWrite(s_ch, 0);
    } else {
      ledcWriteTone(s_ch, st.hz);
      ledcWrite(s_ch, 128);
    }
    return;
  }

  const ToneStep& st = p.steps[s_step];
  if ((uint32_t)(now - s_t0) >= st.dur_ms) {
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

// -------- Actuators --------
void Actuators::begin(int pinRelay, int pinBuzzer, int pinResetBtn, OLED_NOTIF* oled) {
  _pinRelay = pinRelay;
  _pinBuzzer = pinBuzzer;
  _pinReset = pinResetBtn;
  _oled = oled;

  pinMode(_pinRelay, OUTPUT);
  digitalWrite(_pinRelay, HIGH);   // relay ON immediately at boot

  pinMode(_pinBuzzer, OUTPUT);
  digitalWrite(_pinBuzzer, LOW);

  pinMode(_pinReset, INPUT_PULLUP);

  soundBegin(_pinBuzzer);
  soundStart(SND_BOOT);
}

void Actuators::setRelay(bool on) {
  digitalWrite(_pinRelay, on ? HIGH : LOW);
}

void Actuators::buzzerTone(bool on, uint16_t freqHz) {
  soundBegin(_pinBuzzer);
  if (!on) {
    ledcWrite(s_ch, 0);
    return;
  }
  ledcWriteTone(s_ch, freqHz);
  ledcWrite(s_ch, 128);
}

void Actuators::notify(SoundEvent ev) {
  soundBegin(_pinBuzzer);
  soundStart((uint8_t)ev);
}

void Actuators::apply(FaultState st, float v, float i, float t) {
  // Relay rules:
#ifndef DATA_COLLECTION_MODE
  // Normal protection: arc/heating cut power, overload does NOT cut.
  const bool relayOn = (st == STATE_NORMAL || st == STATE_OVERLOAD);
  setRelay(relayOn);
#else
  // DATA COLLECTION MODE:
  // - Still report fault states (and play fault sounds)
  // - BUT keep relay ON during ARC/OVER/HEAT so you can capture data
  // - Only force relay OFF when temperature crosses the "hard" cutoff (75C).
  static bool hardOff = false;
  static constexpr float HARD_OFF_C = TEMP_TRIP_C + 5.0f; // 75C when TEMP_TRIP_C=70C
  static constexpr float HARD_ON_C  = HARD_OFF_C - 2.0f;  // 73C hysteresis

  if (!hardOff && t >= HARD_OFF_C) hardOff = true;
  else if (hardOff && t <= HARD_ON_C) hardOff = false;

  setRelay(!hardOff);
#endif

  // Distinct fault sounds
  static FaultState lastSt = STATE_NORMAL;
  if (st != lastSt) {
    if      (st == STATE_ARCING)   notify(SND_FAULT_ARC);
    else if (st == STATE_HEATING)  notify(SND_FAULT_HEAT);
    else if (st == STATE_OVERLOAD) notify(SND_FAULT_OVER);
    else {
      // Returning to normal stops repeating fault pattern
      soundStop();
    }
    lastSt = st;
  }

  // Mains lost (Vrms near 0 while previously present)
  static bool mainsWasOn = false;
  const bool mainsOn = (v > 60.0f);
  if (mainsWasOn && !mainsOn) notify(SND_MAINS_LOST);
  mainsWasOn = mainsOn;

  // Device plugged detection (current rises above a small threshold)
  static bool loadWasOn = false;
  const bool loadOn = (i > 0.06f);
  if (!loadWasOn && loadOn) notify(SND_DEVICE_PLUG);
  loadWasOn = loadOn;

  // Run sound engine
  soundLoop(_pinBuzzer);

  // OLED refresh
  if (_oled && (millis() - _lastOled) >= OLED_MS) {
    _lastOled = millis();
    _oled->updateDashboard(v, i, t, st);
  }
}

bool Actuators::resetLongPressed() {
  const bool down = (digitalRead(_pinReset) == LOW);
  const uint32_t now = millis();

  if (down) {
    if (_btnDown == 0) _btnDown = now;
    if (!_btnHeld && (now - _btnDown) > 1200) {
      _btnHeld = true;
      notify(SND_RESET_ACK);
      return true;
    }
  } else {
    _btnDown = 0;
    _btnHeld = false;
  }
  return false;
}