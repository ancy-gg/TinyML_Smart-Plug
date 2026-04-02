#include "Notification.h"
#include "MainConfiguration.h"
#include <math.h>
#include <string.h>

static constexpr uint8_t BUZZ_CH = 0;

struct ToneStep { uint16_t hz; uint16_t dur_ms; uint8_t duty; };
struct TonePattern { const ToneStep* steps; uint8_t count; bool repeat; uint8_t priority; };

static constexpr ToneStep P_BOOT[] = {{1850,80,112},{0,40,0},{2350,90,120},{0,40,0},{2950,130,128}};
static constexpr ToneStep P_WIFI_PORTAL[] = {{880,120,112},{0,70,0},{1175,120,116},{0,70,0},{1568,150,120}};
static constexpr ToneStep P_WIFI_OK[] = {{1800,60,110},{0,25,0},{2400,70,118},{0,25,0},{3200,110,126}};
static constexpr ToneStep P_LOGGER_ON[] = {{620,36,36},{0,22,0},{820,42,42}};
static constexpr ToneStep P_OTA_START[] = {{1500,80,112},{0,30,0},{1300,90,112},{0,30,0},{1100,120,118}};
static constexpr ToneStep P_OTA_OK[] = {{1300,60,110},{0,25,0},{1800,60,118},{0,25,0},{2500,140,126}};
static constexpr ToneStep P_OTA_FAIL[] = {{520,200,132},{0,80,0},{420,240,136},{0,80,0},{320,320,140}};
static constexpr ToneStep P_MAINS_LOST[] = {{430,240,128},{0,90,0},{330,320,128}};
static constexpr ToneStep P_MAINS_RESTORED[] = {{1700,70,110},{0,30,0},{2300,80,118},{0,30,0},{3000,110,126}};
static constexpr ToneStep P_VOLT_LOW[] = {{700,180,120},{0,70,0},{620,220,124}};
static constexpr ToneStep P_VOLT_HIGH[] = {{2500,120,118},{0,60,0},{2900,150,124}};
static constexpr ToneStep P_DEVICE_PLUG[] = {{720,34,34},{0,18,0},{900,44,40}};
static constexpr ToneStep P_FAULT_ARC[] = {{4100,28,132},{0,18,0},{3450,34,132},{0,20,0},{3900,30,132},{0,24,0},{3000,48,132},{0,140,0}};
static constexpr ToneStep P_FAULT_HEAT[] = {{1650,180,126},{0,70,0},{1650,180,126},{0,70,0},{1650,180,126},{0,220,0}};
static constexpr ToneStep P_FAULT_OVER[] = {{2450,120,118},{0,100,0},{2450,120,118},{0,280,0}};
static constexpr ToneStep P_FAULT_UNDERVOLT[] = {{760,180,124},{0,70,0},{680,220,128},{0,220,0}};
static constexpr ToneStep P_FAULT_OVERVOLT[] = {{2600,140,126},{0,60,0},{3200,160,130},{0,220,0}};
static constexpr ToneStep P_RESET_ACK[] = {{520,34,34}};

static constexpr TonePattern PATTERNS[] = {
  {P_BOOT,(uint8_t)(sizeof(P_BOOT)/sizeof(P_BOOT[0])),false,1},
  {P_WIFI_PORTAL,(uint8_t)(sizeof(P_WIFI_PORTAL)/sizeof(P_WIFI_PORTAL[0])),false,1},
  {P_WIFI_OK,(uint8_t)(sizeof(P_WIFI_OK)/sizeof(P_WIFI_OK[0])),false,1},
  {P_LOGGER_ON,(uint8_t)(sizeof(P_LOGGER_ON)/sizeof(P_LOGGER_ON[0])),false,1},
  {P_OTA_START,(uint8_t)(sizeof(P_OTA_START)/sizeof(P_OTA_START[0])),false,2},
  {P_OTA_OK,(uint8_t)(sizeof(P_OTA_OK)/sizeof(P_OTA_OK[0])),false,2},
  {P_OTA_FAIL,(uint8_t)(sizeof(P_OTA_FAIL)/sizeof(P_OTA_FAIL[0])),false,2},
  {P_MAINS_LOST,(uint8_t)(sizeof(P_MAINS_LOST)/sizeof(P_MAINS_LOST[0])),false,2},
  {P_MAINS_RESTORED,(uint8_t)(sizeof(P_MAINS_RESTORED)/sizeof(P_MAINS_RESTORED[0])),false,2},
  {P_VOLT_LOW,(uint8_t)(sizeof(P_VOLT_LOW)/sizeof(P_VOLT_LOW[0])),false,2},
  {P_VOLT_HIGH,(uint8_t)(sizeof(P_VOLT_HIGH)/sizeof(P_VOLT_HIGH[0])),false,2},
  {P_DEVICE_PLUG,(uint8_t)(sizeof(P_DEVICE_PLUG)/sizeof(P_DEVICE_PLUG[0])),false,1},
  {P_FAULT_ARC,(uint8_t)(sizeof(P_FAULT_ARC)/sizeof(P_FAULT_ARC[0])),true,4},
  {P_FAULT_HEAT,(uint8_t)(sizeof(P_FAULT_HEAT)/sizeof(P_FAULT_HEAT[0])),true,5},
  {P_FAULT_OVER,(uint8_t)(sizeof(P_FAULT_OVER)/sizeof(P_FAULT_OVER[0])),true,3},
  {P_FAULT_UNDERVOLT,(uint8_t)(sizeof(P_FAULT_UNDERVOLT)/sizeof(P_FAULT_UNDERVOLT[0])),true,3},
  {P_FAULT_OVERVOLT,(uint8_t)(sizeof(P_FAULT_OVERVOLT)/sizeof(P_FAULT_OVERVOLT[0])),true,4},
  {P_RESET_ACK,(uint8_t)(sizeof(P_RESET_ACK)/sizeof(P_RESET_ACK[0])),false,1}
};

static int s_buzzPin = -1;
static bool s_pwmReady = false;
static uint8_t s_activeId = 255;
static uint8_t s_step = 0;
static uint32_t s_t0 = 0;
static uint8_t s_activePrio = 0;
static uint8_t s_faultHoldSound = 255;
static uint32_t s_faultHoldUntil = 0;
static uint32_t s_artifactSuppressUntil = 0;

static inline bool isFaultPattern(uint8_t id) {
  return id == SND_FAULT_ARC || id == SND_FAULT_HEAT || id == SND_FAULT_OVER || id == SND_FAULT_UNDERVOLT || id == SND_FAULT_OVERVOLT;
}
static inline bool isArtifactSensitiveStatusPattern(uint8_t id) {
  return id == SND_BOOT || id == SND_WIFI_PORTAL || id == SND_WIFI_OK || id == SND_LOGGER_ON ||
         id == SND_OTA_START || id == SND_OTA_OK || id == SND_OTA_FAIL || id == SND_MAINS_LOST ||
         id == SND_MAINS_RESTORED || id == SND_VOLT_LOW || id == SND_VOLT_HIGH ||
         id == SND_DEVICE_PLUG || id == SND_RESET_ACK;
}
static inline bool statusSoundAllowed() { return BUZZER_STATUS_ENABLED && millis() >= BUZZER_STARTUP_MUTE_MS; }
static inline bool eventSoundAllowed(uint8_t id) { return isFaultPattern(id) || statusSoundAllowed(); }

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
  ledcWriteTone(s_buzzPin, 0); ledcWrite(s_buzzPin, 0);
#else
  ledcWriteTone(BUZZ_CH, 0); ledcWrite(BUZZ_CH, 0);
#endif
}
static void pwmTone(uint16_t hz, uint8_t duty) {
  if (!s_pwmReady || s_buzzPin < 0) return;
  if (hz == 0 || duty == 0) { pwmStop(); return; }
  if (isFaultPattern(s_activeId)) {
    if (duty > 104) duty = 104;
  } else {
    if (duty > BUZZER_STATUS_MAX_DUTY) duty = BUZZER_STATUS_MAX_DUTY;
    if (hz > BUZZER_STATUS_MAX_HZ) hz = BUZZER_STATUS_MAX_HZ;
  }
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(s_buzzPin, hz); ledcWrite(s_buzzPin, duty);
#else
  ledcWriteTone(BUZZ_CH, hz); ledcWrite(BUZZ_CH, duty);
#endif
}
static void soundBegin(int pinBuzzer) { s_buzzPin = pinBuzzer; pwmAttachIfNeeded(pinBuzzer); pwmStop(); }
static void soundStop() {
  if (isArtifactSensitiveStatusPattern(s_activeId)) s_artifactSuppressUntil = millis() + BUZZER_ARTIFACT_SUPPRESS_HOLD_MS;
  pwmStop(); s_activeId = 255; s_step = 0; s_t0 = 0; s_activePrio = 0;
}
static void soundStart(uint8_t id) {
  if (id >= (sizeof(PATTERNS) / sizeof(PATTERNS[0])) || s_activeId == id || !eventSoundAllowed(id)) return;
  const TonePattern& p = PATTERNS[id];
  if (s_activeId != 255 && p.priority < s_activePrio) return;
  s_activeId = id; s_activePrio = p.priority; s_step = 0; s_t0 = 0;
  if (isArtifactSensitiveStatusPattern(id)) s_artifactSuppressUntil = millis() + BUZZER_ARTIFACT_SUPPRESS_HOLD_MS;
}
static void soundLoop() {
  if (s_activeId == 255) return;
  const TonePattern& p = PATTERNS[s_activeId];
  const uint32_t now = millis();
  if (s_t0 == 0) { s_t0 = now; pwmTone(p.steps[s_step].hz, p.steps[s_step].duty); return; }
  if ((uint32_t)(now - s_t0) >= p.steps[s_step].dur_ms) {
    s_step++;
    if (s_step >= p.count) { if (p.repeat) s_step = 0; else { soundStop(); return; } }
    s_t0 = 0;
  }
}


static const uint8_t PROGMEM kBootLogo24x24[] = {
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x03, 0x81, 0xC0, 0x06, 0x00, 0x60,
  0x0C, 0x00, 0x30, 0x18, 0x42, 0x18, 0x30, 0x42, 0x0C, 0x20, 0x42, 0x04,
  0x60, 0x42, 0x06, 0x61, 0xFF, 0x86, 0x41, 0xFF, 0x82, 0x41, 0xFF, 0x02,
  0x41, 0xFF, 0x82, 0x41, 0xFF, 0x82, 0x60, 0xE7, 0x06, 0x60, 0xFF, 0x06,
  0x20, 0x7E, 0x04, 0x30, 0x3C, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x00, 0x30,
  0x06, 0x18, 0x60, 0x03, 0x99, 0xC0, 0x00, 0xCF, 0x00, 0x00, 0x00, 0x00
};

static void formatMeasure(char* dst, size_t n, float v, uint8_t decimals, const char* unit) {
  if (fabsf(v) < 0.0005f) v = 0.0f;
  char num[16];
  snprintf(num, sizeof(num), "%.*f", (int)decimals, v);

  char* p = num;
  while (p[0] == '0' && p[1] != '\0' && p[1] != '.') p++;
  if (p[0] == '\0') p = num;

  snprintf(dst, n, "%s%s", p, unit ? unit : "");
}

Notification::Notification(uint8_t address) {
  _address = address;
  display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}

Notification::~Notification() {
  delete display;
}

bool Notification::begin(int pinBuzzer) {
  _pinBuzzer = pinBuzzer;
  soundBegin(_pinBuzzer);
  soundStop();
  if (!display->begin(SSD1306_SWITCHCAPVCC, _address)) return false;
  display->cp437(true);
  display->clearDisplay();
  display->display();
  return true;
}

void Notification::setMeasurements(float voltage, float current, float apparentPower, float temperature) {
  _voltage = voltage;
  _current = current;
  _apparentPower = apparentPower;
  _temperature = temperature;
}

void Notification::setState(FaultState state) {
  _state = state;
}

void Notification::setWiFi(bool connected, int rssi, bool blocking, bool inPortal, bool timedOut, bool apWindow) {
  _wifiConnected = connected;
  _wifiRssi = rssi;
  _wifiBlocking = blocking;
  _wifiPortal = inPortal;
  _wifiTimedOut = timedOut;
  _wifiApWindow = apWindow;
}

void Notification::triggerCollecting(uint32_t durMs) {
  _collectUntilMs = millis() + durMs;
}

void Notification::triggerNotice(const char* line1, const char* line2, uint32_t durMs) {
  const char* l1 = line1 ? line1 : "";
  const char* l2 = line2 ? line2 : "";
  strncpy(_noticeLine1, l1, sizeof(_noticeLine1) - 1);
  _noticeLine1[sizeof(_noticeLine1) - 1] = '\0';
  strncpy(_noticeLine2, l2, sizeof(_noticeLine2) - 1);
  _noticeLine2[sizeof(_noticeLine2) - 1] = '\0';
  _noticeUntilMs = millis() + durMs;
}


void Notification::triggerConnected(uint32_t durMs) {
  uint32_t start = millis();
  if (_bootUntilMs && (int32_t)(_bootUntilMs - start) > 0) start = _bootUntilMs + 120UL;
  _connectedStartMs = start;
  _connectedUntilMs = start + durMs;
}

void Notification::startBootSequence(uint32_t durMs) {
  _bootStartMs = millis();
  _bootUntilMs = _bootStartMs + durMs;
}

void Notification::setOta(bool active, uint8_t progress) {
  _otaActive = active;
  _otaProgress = progress;
}

void Notification::setOverlay(OledOverlay overlay) {
  _overlay = overlay;
}

void Notification::clearOverlay() {
  _overlay = OledOverlay::NONE;
}

void Notification::drawWiFiBars(int x, int y, int bars, bool crossed) {
  bars = constrain(bars, 0, 4);
  for (int i = 0; i < 4; ++i) {
    const int bx = x + i * 3;
    const int bh = i + 1;
    const int by = y + (4 - bh);
    if (i < bars) display->fillRect(bx, by, 2, bh, SSD1306_WHITE);
    else display->drawRect(bx, by, 2, bh, SSD1306_WHITE);
  }
  if (crossed) {
    display->drawLine(x - 1, y + 5, x + 12, y - 1, SSD1306_WHITE);
  }
}

void Notification::drawLogo(int x, int y, bool invert) {
  if (invert) {
    display->fillRect(x, y, 24, 24, SSD1306_WHITE);
    display->drawBitmap(x, y, kBootLogo24x24, 24, 24, SSD1306_BLACK);
  } else {
    display->drawBitmap(x, y, kBootLogo24x24, 24, 24, SSD1306_WHITE);
  }
}

void Notification::drawCenteredText(const char* txt, int y, uint8_t size) {
  display->setTextSize(size);
  int16_t x1, y1; uint16_t w, h;
  display->getTextBounds(txt, 0, y, &x1, &y1, &w, &h);
  display->setCursor((SCREEN_WIDTH - (int)w) / 2, y);
  display->print(txt);
  display->setTextSize(1);
}

void Notification::drawRightText(const char* txt, int xRight, int y, uint8_t size) {
  display->setTextSize(size);
  int16_t x1, y1; uint16_t w, h;
  display->getTextBounds(txt, 0, y, &x1, &y1, &w, &h);
  display->setCursor(xRight - (int)w, y);
  display->print(txt);
  display->setTextSize(1);
}

void Notification::drawDashboard(uint32_t nowMs) {
  (void)nowMs;
  const char* status = "NORMAL";
  if (_state == STATE_ARCING) status = "ARCING";
  else if (_state == STATE_HEATING) status = "HEATING";
  else if (_state == STATE_OVERVOLTAGE) status = "OVERVOLT";
  else if (_state == STATE_UNDERVOLTAGE) status = "UNDERVOLT";
  else if (_state == STATE_OVERLOAD) status = "OVERLOAD";
  else if (_state == STATE_SUSTAINED_OVERLOAD) status = "SUSTAIN";

  const int bars = _wifiConnected ? ((_wifiRssi >= -55) ? 4 : (_wifiRssi >= -67) ? 3 : (_wifiRssi >= -78) ? 2 : 1) : 0;
  drawWiFiBars(1, 1, bars, !_wifiConnected);

  display->setTextColor(SSD1306_WHITE);
  drawCenteredText(status, 0, 1);

  char v[16], i[16], va[16], t[16];
  formatMeasure(v, sizeof(v), _voltage, 1, "V");
  formatMeasure(i, sizeof(i), _current, 2, "A");
  formatMeasure(va, sizeof(va), _apparentPower, 0, "VA");
  formatMeasure(t, sizeof(t), _temperature, 1, "C");

  display->setTextSize(1);
  display->setCursor(0, 12);
  display->print(v);
  drawRightText(va, 127, 12, 1);

  display->setCursor(0, 22);
  display->print(i);
  drawRightText(t, 127, 22, 1);
}

void Notification::drawBootSequence(uint32_t nowMs) {
  if (_bootUntilMs <= _bootStartMs) return;
  const uint32_t total = _bootUntilMs - _bootStartMs;
  const uint32_t t = nowMs - _bootStartMs;
  const int logoTargetX = 4;
  const int logoY = 4;
  const int tinyTargetX = 34;
  const int plugTargetX = 34;

  int logoX = logoTargetX;
  int tinyY = 6;
  int plugY = 18;

  const uint32_t intro = (total * 40U) / 100U;
  const uint32_t hold  = (total * 28U) / 100U;
  const uint32_t outro = total - intro - hold;

  if (t < intro) {
    const float u = (float)t / (float)((intro == 0U) ? 1U : intro);
    logoX = -24 + (int)lroundf((logoTargetX + 24) * u);
    tinyY = -8 + (int)lroundf((6 + 8) * u);
    plugY = 32 - (int)lroundf((32 - 18) * u);
  } else if (t < (intro + hold)) {
    const float phase = (float)(t - intro) / (float)((hold == 0U) ? 1U : hold);
    const int bop = (phase < 0.5f) ? (int)lroundf(2.0f * phase) : (int)lroundf(2.0f * (1.0f - phase));
    logoX = logoTargetX;
    tinyY = 6 - bop;
    plugY = 18 + bop;
  } else {
    const float u = (float)(t - intro - hold) / (float)((outro == 0U) ? 1U : outro);
    logoX = logoTargetX - (int)lroundf((logoTargetX + 28) * u);
    tinyY = 6 - (int)lroundf(10.0f * u);
    plugY = 18 + (int)lroundf(14.0f * u);
  }

  drawLogo(logoX, logoY);
  display->setTextColor(SSD1306_WHITE);
  display->setTextSize(1);
  display->setCursor(tinyTargetX, tinyY);
  display->print("TINYML");
  display->setCursor(plugTargetX, plugY);
  display->print("SMARTPLUG");
}

void Notification::drawConnected(uint32_t nowMs) {
  (void)nowMs;
  drawCenteredText("WIFI", 7, 1);
  drawCenteredText("CONNECTED", 19, 1);
}

void Notification::drawWiFiWait(uint32_t nowMs, bool portal) {
  const int phase = (nowMs / 200U) % 4;

  if (portal) {
    drawCenteredText("ENTERING", 7, 1);
    drawCenteredText("CREDENTIALS", 19, 1);
    return;
  }

  if (_wifiApWindow) {
    drawCenteredText("AP OPEN", 7, 1);
    drawCenteredText("CONNECT NOW", 19, 1);
    return;
  }

  if (_wifiTimedOut) {
    drawCenteredText("NETWORK", 7, 1);
    drawCenteredText("FAILED", 19, 1);
    return;
  }

  drawWiFiBars(6, 2, phase + 1, false);
  drawCenteredText("WIFI", 9, 1);
  drawCenteredText("CONNECTING", 19, 1);
}

void Notification::drawCollecting(uint32_t nowMs) {
  (void)nowMs;
  drawCenteredText("COLLECTING", 7, 1);
  drawCenteredText("DATA", 18, 1);
}

void Notification::drawNotice(uint32_t nowMs) {
  (void)nowMs;
  const bool hasLine2 = (_noticeLine2[0] != '\0');
  if (hasLine2) {
    drawCenteredText(_noticeLine1, 7, 1);
    drawCenteredText(_noticeLine2, 18, 1);
  } else {
    drawCenteredText(_noticeLine1, 12, 1);
  }
}

void Notification::drawOta(uint32_t nowMs) {
  (void)nowMs;
  drawCenteredText("UPDATING", 4, 1);
  display->drawRect(8, 18, 112, 8, SSD1306_WHITE);
  int w = (int)lroundf((110.0f * (float)_otaProgress) / 100.0f);
  if (w > 0) display->fillRect(9, 19, w, 6, SSD1306_WHITE);
}

void Notification::drawUnplugged(uint32_t nowMs) {
  (void)nowMs;
  drawLogo(8, 4);
  display->drawLine(6, 4, 30, 28, SSD1306_WHITE);
  display->drawLine(6, 28, 30, 4, SSD1306_WHITE);
  display->setCursor(44, 12);
  display->print("UNPLUGGED");
}

void Notification::drawFaultSlide(uint32_t nowMs, OledOverlay ov) {
  (void)nowMs;

  if (ov == OledOverlay::FAULT_OVERVOLT || ov == OledOverlay::FAULT_SURGE) {
    drawCenteredText("OVERVOLTAGE", 12, 1);
    return;
  }

  if (ov == OledOverlay::FAULT_UNDERVOLT) {
    drawCenteredText("UNDERVOLTAGE", 12, 1);
    return;
  }

  if (ov == OledOverlay::FAULT_ARC) {
    drawCenteredText("ARC", 7, 1);
    drawCenteredText("FAULT", 18, 1);
    return;
  }

  if (ov == OledOverlay::FAULT_HEAT || ov == OledOverlay::FAULT_TEMP_CRITICAL) {
    drawCenteredText("HEATING", 12, 1);
    return;
  }

  if (ov == OledOverlay::FAULT_OVERLOAD) {
    if (_state == STATE_SUSTAINED_OVERLOAD) {
      drawCenteredText("SUSTAINED", 7, 1);
      drawCenteredText("OVERLOAD", 18, 1);
    } else {
      drawCenteredText("OVERLOAD", 12, 1);
    }
    return;
  }

  drawCenteredText("FAULT", 12, 1);
}

void Notification::render() {
  const uint32_t now = millis();
  if (_voltage <= 0.2f) {
    if (_noPowerSinceMs == 0) _noPowerSinceMs = now;
  } else {
    _noPowerSinceMs = 0;
  }

  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setTextSize(1);

  if (_bootUntilMs && now < _bootUntilMs) {
    drawBootSequence(now);
  } else if (_otaActive) {
    drawOta(now);
  } else if (_connectedUntilMs && now >= _connectedStartMs && now < _connectedUntilMs) {
    drawConnected(now);
  } else if (_overlay != OledOverlay::NONE) {
    if (_overlay == OledOverlay::UNPLUGGED) drawUnplugged(now);
    else drawFaultSlide(now, _overlay);
  } else if (_noticeUntilMs && (int32_t)(_noticeUntilMs - now) > 0) {
    drawNotice(now);
  } else if (_collectUntilMs && (int32_t)(_collectUntilMs - now) > 0) {
    drawCollecting(now);
  } else if (_wifiBlocking) {
    drawWiFiWait(now, _wifiPortal);
  } else if (_noPowerSinceMs && (now - _noPowerSinceMs) >= UNPLUGGED_STATE_DELAY_MS) {
    drawUnplugged(now);
  } else {
    drawDashboard(now);
  }
  display->display();
}

void Notification::showStatus(const char* title, const char* msg) {
  display->clearDisplay();
  drawCenteredText(title ? title : "STATUS", 2, 1);
  drawCenteredText(msg ? msg : "", 18, 1);
  display->display();
}

void Notification::clear() {
  display->clearDisplay();
  display->display();
}


void Notification::notify(SoundEvent ev) { soundBegin(_pinBuzzer); soundStart((uint8_t)ev); }

bool Notification::shouldSuppressCurrentArtifacts() const {
  return ((isArtifactSensitiveStatusPattern(s_activeId) && !isFaultPattern(s_activeId)) || ((int32_t)(s_artifactSuppressUntil - millis()) > 0));
}

void Notification::clearFaultAlert() {
  s_faultHoldSound = 255;
  s_faultHoldUntil = 0;
  if (isFaultPattern(s_activeId)) soundStop();
}

void Notification::updateBuzzer(FaultState st, float vProtect, float i, float t) {
  const uint32_t now = millis();
#if COLLECTION_ONLY_MODE
  const float heatWarnC = TEMP_DATA_WARN_C;
#else
  const float heatWarnC = TEMP_WARN_C;
#endif
  const bool heatActive = (st == STATE_HEATING) || (t >= heatWarnC);
  const bool arcActive  = (st == STATE_ARCING);
  const bool underVoltActive = (st == STATE_UNDERVOLTAGE);
  const bool overVoltActive  = (st == STATE_OVERVOLTAGE);
  const bool overloadActive  = (st == STATE_OVERLOAD) || (st == STATE_SUSTAINED_OVERLOAD) || (i >= OVERLOAD_WARN_A);

  uint8_t wantedFaultSound = 255;
  if (heatActive) wantedFaultSound = SND_FAULT_HEAT;
  else if (arcActive) wantedFaultSound = SND_FAULT_ARC;
  else if (overVoltActive) wantedFaultSound = SND_FAULT_OVERVOLT;
  else if (underVoltActive) wantedFaultSound = SND_FAULT_UNDERVOLT;
  else if (overloadActive) wantedFaultSound = SND_FAULT_OVER;

  if (wantedFaultSound != 255) {
    if (wantedFaultSound != s_faultHoldSound) {
      s_faultHoldSound = wantedFaultSound;
      s_faultHoldUntil = now + FAULT_ALERT_MIN_MS;
      soundStart(wantedFaultSound);
    } else if (s_faultHoldSound != 255 && (int32_t)(s_faultHoldUntil - now) > 0) {
      soundStart(s_faultHoldSound);
    } else if (isFaultPattern(s_activeId)) {
      soundStop();
    }
  } else {
    if (s_faultHoldSound != 255 && (int32_t)(s_faultHoldUntil - now) > 0) {
      soundStart(s_faultHoldSound);
    } else {
      s_faultHoldSound = 255;
      s_faultHoldUntil = 0;
      if (isFaultPattern(s_activeId)) soundStop();
    }
  }

  static bool mainsStable = false;
  static bool mainsInit = false;
  static uint32_t mainsOffSince = 0;
  static uint32_t mainsOnSince = 0;
  const bool rawOn = (vProtect >= MAINS_PRESENT_ON_V);
  const bool rawOff = (vProtect <= MAINS_PRESENT_OFF_V);
  if (!mainsInit) { mainsStable = rawOn; mainsInit = true; }
  if (rawOff) {
    if (mainsOffSince == 0) mainsOffSince = now;
    mainsOnSince = 0;
    if (mainsStable && (now - mainsOffSince) >= UNPLUGGED_BUZZ_DELAY_MS) {
      mainsStable = false;
      notify(SND_MAINS_LOST);
    }
  } else if (rawOn) {
    if (mainsOnSince == 0) mainsOnSince = now;
    mainsOffSince = 0;
    if (!mainsStable && (now - mainsOnSince) >= MAINS_EDGE_DEBOUNCE_MS) {
      mainsStable = true;
      notify(SND_MAINS_RESTORED);
    }
  } else {
    mainsOffSince = 0;
    mainsOnSince = 0;
  }

  soundLoop();
}
