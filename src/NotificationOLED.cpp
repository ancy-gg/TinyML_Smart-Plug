#include "NotificationOLED.h"
#include "SmartPlugConfig.h"
#include <math.h>
#include <string.h>

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

NotificationOLED::NotificationOLED(uint8_t address) {
  _address = address;
  display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}

NotificationOLED::~NotificationOLED() {
  delete display;
}

bool NotificationOLED::begin() {
  if (!display->begin(SSD1306_SWITCHCAPVCC, _address)) return false;
  display->cp437(true);
  display->clearDisplay();
  display->display();
  return true;
}

void NotificationOLED::setMeasurements(float voltage, float current, float apparentPower, float temperature) {
  _voltage = voltage;
  _current = current;
  _apparentPower = apparentPower;
  _temperature = temperature;
}

void NotificationOLED::setState(FaultState state) {
  _state = state;
}

void NotificationOLED::setWiFi(bool connected, int rssi, bool blocking, bool inPortal, bool timedOut, bool apWindow) {
  _wifiConnected = connected;
  _wifiRssi = rssi;
  _wifiBlocking = blocking;
  _wifiPortal = inPortal;
  _wifiTimedOut = timedOut;
  _wifiApWindow = apWindow;
}

void NotificationOLED::triggerCollecting(uint32_t durMs) {
  _collectUntilMs = millis() + durMs;
}

void NotificationOLED::triggerNotice(const char* line1, const char* line2, uint32_t durMs) {
  const char* l1 = line1 ? line1 : "";
  const char* l2 = line2 ? line2 : "";
  strncpy(_noticeLine1, l1, sizeof(_noticeLine1) - 1);
  _noticeLine1[sizeof(_noticeLine1) - 1] = '\0';
  strncpy(_noticeLine2, l2, sizeof(_noticeLine2) - 1);
  _noticeLine2[sizeof(_noticeLine2) - 1] = '\0';
  _noticeUntilMs = millis() + durMs;
}


void NotificationOLED::triggerConnected(uint32_t durMs) {
  uint32_t start = millis();
  if (_bootUntilMs && (int32_t)(_bootUntilMs - start) > 0) start = _bootUntilMs + 120UL;
  _connectedStartMs = start;
  _connectedUntilMs = start + durMs;
}

void NotificationOLED::startBootSequence(uint32_t durMs) {
  _bootStartMs = millis();
  _bootUntilMs = _bootStartMs + durMs;
}

void NotificationOLED::setOta(bool active, uint8_t progress) {
  _otaActive = active;
  _otaProgress = progress;
}

void NotificationOLED::setOverlay(OledOverlay overlay) {
  _overlay = overlay;
}

void NotificationOLED::clearOverlay() {
  _overlay = OledOverlay::NONE;
}

void NotificationOLED::drawWiFiBars(int x, int y, int bars, bool crossed) {
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

void NotificationOLED::drawLogo(int x, int y, bool invert) {
  if (invert) {
    display->fillRect(x, y, 24, 24, SSD1306_WHITE);
    display->drawBitmap(x, y, kBootLogo24x24, 24, 24, SSD1306_BLACK);
  } else {
    display->drawBitmap(x, y, kBootLogo24x24, 24, 24, SSD1306_WHITE);
  }
}

void NotificationOLED::drawCenteredText(const char* txt, int y, uint8_t size) {
  display->setTextSize(size);
  int16_t x1, y1; uint16_t w, h;
  display->getTextBounds(txt, 0, y, &x1, &y1, &w, &h);
  display->setCursor((SCREEN_WIDTH - (int)w) / 2, y);
  display->print(txt);
  display->setTextSize(1);
}

void NotificationOLED::drawRightText(const char* txt, int xRight, int y, uint8_t size) {
  display->setTextSize(size);
  int16_t x1, y1; uint16_t w, h;
  display->getTextBounds(txt, 0, y, &x1, &y1, &w, &h);
  display->setCursor(xRight - (int)w, y);
  display->print(txt);
  display->setTextSize(1);
}

void NotificationOLED::drawDashboard(uint32_t nowMs) {
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

void NotificationOLED::drawBootSequence(uint32_t nowMs) {
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

void NotificationOLED::drawConnected(uint32_t nowMs) {
  (void)nowMs;
  drawCenteredText("WIFI", 7, 1);
  drawCenteredText("CONNECTED", 19, 1);
}

void NotificationOLED::drawWiFiWait(uint32_t nowMs, bool portal) {
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

void NotificationOLED::drawCollecting(uint32_t nowMs) {
  (void)nowMs;
  drawCenteredText("COLLECTING", 7, 1);
  drawCenteredText("DATA", 18, 1);
}

void NotificationOLED::drawNotice(uint32_t nowMs) {
  (void)nowMs;
  const bool hasLine2 = (_noticeLine2[0] != '\0');
  if (hasLine2) {
    drawCenteredText(_noticeLine1, 7, 1);
    drawCenteredText(_noticeLine2, 18, 1);
  } else {
    drawCenteredText(_noticeLine1, 12, 1);
  }
}

void NotificationOLED::drawOta(uint32_t nowMs) {
  (void)nowMs;
  drawCenteredText("UPDATING", 4, 1);
  display->drawRect(8, 18, 112, 8, SSD1306_WHITE);
  int w = (int)lroundf((110.0f * (float)_otaProgress) / 100.0f);
  if (w > 0) display->fillRect(9, 19, w, 6, SSD1306_WHITE);
}

void NotificationOLED::drawUnplugged(uint32_t nowMs) {
  (void)nowMs;
  drawLogo(8, 4);
  display->drawLine(6, 4, 30, 28, SSD1306_WHITE);
  display->drawLine(6, 28, 30, 4, SSD1306_WHITE);
  display->setCursor(44, 12);
  display->print("UNPLUGGED");
}

void NotificationOLED::drawFaultSlide(uint32_t nowMs, OledOverlay ov) {
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

void NotificationOLED::render() {
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

void NotificationOLED::showStatus(const char* title, const char* msg) {
  display->clearDisplay();
  drawCenteredText(title ? title : "STATUS", 2, 1);
  drawCenteredText(msg ? msg : "", 18, 1);
  display->display();
}

void NotificationOLED::clear() {
  display->clearDisplay();
  display->display();
}
