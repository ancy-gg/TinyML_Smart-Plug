#include "Actuators.h"
#include "OLED_NOTIF.h"
#include "SmartPlugConfig.h"

static constexpr uint32_t OLED_MS = 250;

struct ToneStep { uint16_t hz; uint16_t dur_ms; };
struct TonePattern { const ToneStep* steps; uint8_t count; bool repeat; uint8_t priority; };

static constexpr ToneStep P_BOOT[]        = { {880,80},{0,40},{1320,80},{0,40},{1760,120} };
static constexpr ToneStep P_WIFI_PORTAL[] = { {660,120},{0,120},{660,120},{0,900} };
static constexpr ToneStep P_WIFI_OK[]     = { {1200,120},{0,60},{1600,120} };
static constexpr ToneStep P_LOGGER_ON[]   = { {2000,70},{0,60},{2000,70},{0,60},{2000,90} };
static constexpr ToneStep P_OTA_START[]   = { {1400,120},{0,50},{1000,140},{0,50},{700,180} };
static constexpr ToneStep P_OTA_OK[]      = { {1800,220} };
static constexpr ToneStep P_OTA_FAIL[]    = { {400,250},{0,120},{400,250},{0,120},{400,350} };
static constexpr ToneStep P_MAINS_LOST[]  = { {320,500} };
static constexpr ToneStep P_DEVICE_PLUG[] = { {1500,60},{0,40},{1800,60} };
static constexpr ToneStep P_FAULT_ARC[]   = { {1400,160},{0,60},{900,160},{0,60} };
static constexpr ToneStep P_FAULT_HEAT[]  = { {800,220},{0,180},{800,220},{0,900} };
static constexpr ToneStep P_FAULT_OVER[]  = { {600,120},{0,120},{600,120},{0,600} };
static constexpr ToneStep P_RESET_ACK[]   = { {1200,80},{0,40},{1200,80} };

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

static int s_buzzPin = -1;
static bool s_buzzLevel = false;

static uint8_t s_activeId = 255;
static uint8_t s_step = 0;
static uint32_t s_t0 = 0;
static uint8_t s_activePrio = 0;

static inline void buzzWrite(bool on) {
#if BUZZER_ACTIVE_HIGH
  const bool level = on;
#else
  const bool level = !on;
#endif
  if (s_buzzPin < 0) return;
  if (level != s_buzzLevel) {
    digitalWrite(s_buzzPin, level ? HIGH : LOW);
    s_buzzLevel = level;
  }
}

static void soundBegin(int pinBuzzer) { s_buzzPin = pinBuzzer; buzzWrite(false); }

static void soundStop() {
  buzzWrite(false);
  s_activeId = 255; s_step = 0; s_t0 = 0; s_activePrio = 0;
}

static void soundStart(uint8_t id) {
  if (id >= (sizeof(PATTERNS)/sizeof(PATTERNS[0]))) return;
  if (s_activeId == id) return; // don't restart same repeat
  const TonePattern& p = PATTERNS[id];
  if (s_activeId != 255 && p.priority < s_activePrio) return;
  s_activeId = id; s_activePrio = p.priority; s_step = 0; s_t0 = 0;
}

static void soundLoop() {
  if (s_activeId == 255) return;
  const TonePattern& p = PATTERNS[s_activeId];
  const uint32_t now = millis();

  if (s_t0 == 0) {
    s_t0 = now;
    buzzWrite(p.steps[s_step].hz != 0);
    return;
  }

  if ((uint32_t)(now - s_t0) >= p.steps[s_step].dur_ms) {
    s_step++;
    if (s_step >= p.count) {
      if (p.repeat) s_step = 0;
      else { soundStop(); return; }
    }
    s_t0 = 0;
  }
}

void Actuators::begin(int pinRelay, int pinBuzzer, int pinResetBtn, OLED_NOTIF* oled) {
  _pinRelay = pinRelay;
  _pinBuzzer = pinBuzzer;
  _pinReset = pinResetBtn;
  _oled = oled;

  pinMode(_pinRelay, OUTPUT);
  digitalWrite(_pinRelay, HIGH);

  pinMode(_pinBuzzer, OUTPUT);
#if BUZZER_ACTIVE_HIGH
  digitalWrite(_pinBuzzer, LOW);
#else
  digitalWrite(_pinBuzzer, HIGH);
#endif

  pinMode(_pinReset, INPUT_PULLUP);

  soundBegin(_pinBuzzer);
  soundStart(SND_BOOT);
}

void Actuators::setRelay(bool on) { digitalWrite(_pinRelay, on ? HIGH : LOW); }
void Actuators::notify(SoundEvent ev) { soundBegin(_pinBuzzer); soundStart((uint8_t)ev); }

void Actuators::apply(FaultState st, float v, float i, float t) {
#ifndef DATA_COLLECTION_MODE
  setRelay(st == STATE_NORMAL || st == STATE_OVERLOAD);
#else
  // Data collection: relay ON even in faults, only hard-off at TEMP_TRIP+5C
  static bool hardOff = false;
  static constexpr float HARD_OFF_C = TEMP_TRIP_C + 5.0f;
  static constexpr float HARD_ON_C  = HARD_OFF_C - 2.0f;
  if (!hardOff && t >= HARD_OFF_C) hardOff = true;
  else if (hardOff && t <= HARD_ON_C) hardOff = false;
  setRelay(!hardOff);
#endif

  static FaultState lastSt = STATE_NORMAL;
  if (st != lastSt) {
    if      (st == STATE_ARCING)   notify(SND_FAULT_ARC);
    else if (st == STATE_HEATING)  notify(SND_FAULT_HEAT);
    else if (st == STATE_OVERLOAD) notify(SND_FAULT_OVER);
    else soundStop();
    lastSt = st;
  }

  static bool mainsWasOn = false;
  const bool mainsOn = (v > 60.0f);
  if (mainsWasOn && !mainsOn) notify(SND_MAINS_LOST);
  mainsWasOn = mainsOn;

  soundLoop();

  if (_oled && (millis() - _lastOled) >= OLED_MS) {
    _lastOled = millis();

    bool toneOn = false;
    if (s_activeId != 255) toneOn = (PATTERNS[s_activeId].steps[s_step].hz != 0);

    buzzWrite(false);
    _oled->updateDashboard(v, i, t, st);
    if (s_activeId != 255) buzzWrite(toneOn);
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