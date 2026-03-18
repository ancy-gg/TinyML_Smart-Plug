
#include "OLED_NOTIF.h"
#include <math.h>
#include <stdio.h>

static const uint8_t PROGMEM kLogo24[] = {
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x03, 0x81, 0xC0, 0x06, 0x00, 0x60, 0x08, 0x00, 0x10, 0x10, 0x42, 0x18, 0x30, 0x42, 0x0C, 0x20, 0x42, 0x04, 0x20, 0x00, 0x04, 0x41, 0xFF, 0x86, 0x41, 0xFF, 0x82, 0x40, 0xFF, 0x02, 0x41, 0xFF, 0x02, 0x41, 0xFF, 0x02, 0x40, 0xE7, 0x02, 0x20, 0xFF, 0x04, 0x20, 0x7E, 0x04, 0x30, 0x38, 0x08, 0x10, 0x18, 0x08, 0x08, 0x00, 0x10, 0x06, 0x10, 0x60, 0x03, 0x19, 0xC0, 0x00, 0xCF, 0x00, 0x00, 0x00, 0x00
};

static int barsFromRssi(int rssi, bool connected) {
  if (!connected) return 0;
  if (rssi >= -55) return 4;
  if (rssi >= -67) return 3;
  if (rssi >= -78) return 2;
  return 1;
}

static void formatValue(char* dst, size_t n, float v, uint8_t decLo, uint8_t decHi, const char* unit) {
  if (fabsf(v) < 0.0005f) v = 0.0f;
  uint8_t dec = (fabsf(v) >= 100.0f) ? decHi : decLo;
  char num[16];
  snprintf(num, sizeof(num), "%.*f", (int)dec, v);

  if (strchr(num, '.')) {
    size_t len = strlen(num);
    while (len > 0 && num[len - 1] == '0') {
      num[--len] = '\0';
    }
    if (len > 0 && num[len - 1] == '.') num[--len] = '\0';
  }
  snprintf(dst, n, "%s%s", num, unit ? unit : "");
}

OLED_NOTIF::OLED_NOTIF(uint8_t address) {
  _address = address;
  display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}

OLED_NOTIF::~OLED_NOTIF() {
  delete display;
}

bool OLED_NOTIF::begin() {
  if (!display->begin(SSD1306_SWITCHCAPVCC, _address)) return false;
  display->cp437(true);
  display->clearDisplay();
  display->display();
  return true;
}

void OLED_NOTIF::setMeasurements(float voltage, float current, float apparentPower, float temperature) {
  _voltage = voltage;
  _current = current;
  _apparentPower = apparentPower;
  _temperature = temperature;
}

void OLED_NOTIF::setState(FaultState state) {
  _state = state;
}

void OLED_NOTIF::setWiFi(bool connected, int rssi, bool blocking, bool inPortal) {
  _wifiConnected = connected;
  _wifiRssi = rssi;
  _wifiBlocking = blocking;
  _wifiPortal = inPortal;
}

void OLED_NOTIF::triggerCollecting(uint32_t durMs) {
  _collectUntilMs = millis() + durMs;
}

void OLED_NOTIF::setOta(bool active, uint8_t progress) {
  _otaActive = active;
  _otaProgress = progress;
}

void OLED_NOTIF::setOverlay(OledOverlay overlay) {
  _overlay = overlay;
}

void OLED_NOTIF::clearOverlay() {
  _overlay = OledOverlay::NONE;
}

void OLED_NOTIF::startBootSplash(uint32_t durMs) {
  _bootStartedMs = millis();
  _bootUntilMs = _bootStartedMs + durMs;
}

void OLED_NOTIF::triggerConnected(uint32_t durMs) {
  _connectedUntilMs = millis() + durMs;
}

void OLED_NOTIF::drawTextRight(int rightX, int y, const char* txt, uint8_t size) {
  int16_t x1, y1; uint16_t w, h;
  display->setTextSize(size);
  display->getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  display->setCursor(rightX - (int)w, y);
  display->print(txt);
}

void OLED_NOTIF::drawTextCentered(int cx, int y, const char* txt, uint8_t size) {
  int16_t x1, y1; uint16_t w, h;
  display->setTextSize(size);
  display->getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  display->setCursor(cx - ((int)w / 2), y);
  display->print(txt);
}

void OLED_NOTIF::drawWiFiBars(int x, int y, int bars, bool crossed) {
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

void OLED_NOTIF::drawLogoBitmap(int x, int y, bool invert) {
  if (invert) {
    display->fillRect(x, y, 24, 24, SSD1306_WHITE);
    display->drawBitmap(x, y, kLogo24, 24, 24, SSD1306_BLACK);
  } else {
    display->drawBitmap(x, y, kLogo24, 24, 24, SSD1306_WHITE);
  }
}

void OLED_NOTIF::drawBoltIcon(int x, int y, int phase) {
  const int dy = (phase & 1) ? 1 : 0;
  display->drawLine(x + 7, y + 0 + dy, x + 3, y + 8 + dy, SSD1306_WHITE);
  display->drawLine(x + 3, y + 8 + dy, x + 7, y + 8 + dy, SSD1306_WHITE);
  display->drawLine(x + 7, y + 8 + dy, x + 5, y + 15 + dy, SSD1306_WHITE);
  display->drawLine(x + 5, y + 15 + dy, x + 11, y + 6 + dy, SSD1306_WHITE);
  display->drawLine(x + 11, y + 6 + dy, x + 7, y + 6 + dy, SSD1306_WHITE);
}

void OLED_NOTIF::drawArrowUpIcon(int x, int y, int phase) {
  const int bob = (phase % 3) - 1;
  display->drawFastVLine(x + 6, y + 4 + bob, 10, SSD1306_WHITE);
  display->drawLine(x + 6, y + 0 + bob, x + 1, y + 5 + bob, SSD1306_WHITE);
  display->drawLine(x + 6, y + 0 + bob, x + 11, y + 5 + bob, SSD1306_WHITE);
}

void OLED_NOTIF::drawFireIcon(int x, int y, int phase) {
  const int sway = (phase & 1) ? 1 : -1;
  display->drawLine(x + 6 + sway, y + 0, x + 2 + sway, y + 6, SSD1306_WHITE);
  display->drawLine(x + 2 + sway, y + 6, x + 4, y + 15, SSD1306_WHITE);
  display->drawLine(x + 4, y + 15, x + 9, y + 13, SSD1306_WHITE);
  display->drawLine(x + 9, y + 13, x + 11 + sway, y + 7, SSD1306_WHITE);
  display->drawLine(x + 11 + sway, y + 7, x + 8 + sway, y + 2, SSD1306_WHITE);
  display->drawLine(x + 8 + sway, y + 2, x + 6 + sway, y + 0, SSD1306_WHITE);
}

void OLED_NOTIF::drawDashboard(uint32_t nowMs) {
  (void)nowMs;
  const int bars = barsFromRssi(_wifiRssi, _wifiConnected);
  drawWiFiBars(1, 1, bars, !_wifiConnected);

  display->setTextColor(SSD1306_WHITE);
  const char* top = "NORMAL";
  if (_state == STATE_OVERLOAD) top = "OVERLOAD";
  else if (_state == STATE_HEATING) top = "HEATING";
  else if (_state == STATE_ARCING) top = "ARCING";
  drawTextCentered(74, 0, top, 2);

  char v[16], a[16], va[16], t[16];
  formatValue(v, sizeof(v), _voltage, 1, 0, "V");
  formatValue(a, sizeof(a), _current, 2, 1, "A");
  formatValue(va, sizeof(va), _apparentPower, 1, 0, "VA");
  formatValue(t, sizeof(t), _temperature, 1, 0, "C");

  display->setTextSize(1);
  display->setCursor(0, 16); display->print(v);
  drawTextRight(127, 16, va, 1);

  display->setCursor(0, 24); display->print(a);
  drawTextRight(127, 24, t, 1);
}

void OLED_NOTIF::drawWiFiWait(uint32_t nowMs, bool portal) {
  const int phase = (nowMs / 220U) % 4;
  drawWiFiBars(1, 1, phase + 1, false);
  drawTextCentered(68, 8, portal ? "WIFI" : "WIFI", 1);
  drawTextCentered(68, 18, portal ? "CONNECTING" : "CONNECTING", 1);
}

void OLED_NOTIF::drawConnected(uint32_t nowMs) {
  (void)nowMs;
  drawWiFiBars(1, 1, barsFromRssi(_wifiRssi, _wifiConnected), false);
  drawTextCentered(68, 10, "CONNECTED", 1);
}

void OLED_NOTIF::drawStartup(uint32_t nowMs) {
  const uint32_t elapsed = nowMs - _bootStartedMs;
  float p = 1.0f;
  if (_bootUntilMs > _bootStartedMs) {
    p = elapsed / float(_bootUntilMs - _bootStartedMs);
    if (p < 0.0f) p = 0.0f;
    if (p > 1.0f) p = 1.0f;
  }
  const float ease = (p < 0.6f) ? (p / 0.6f) : 1.0f;
  const int logoY = 32 - (int)lroundf(ease * 26.0f);

  drawLogoBitmap(4, logoY, false);

  const int textX = 34;
  const int textY = 8 + ((p < 0.4f) ? (int)lroundf((1.0f - (p / 0.4f)) * 12.0f) : 0);

  display->setTextSize(1);
  display->setCursor(textX, textY);
  display->print("[ LO ]   TINYML");
  display->setCursor(textX, textY + 10);
  display->print("[ GO ] SMARTPLUG");
}

void OLED_NOTIF::drawCollecting(uint32_t nowMs) {
  (void)nowMs;
  drawTextCentered(64, 8, "COLLECTING", 1);
  drawTextCentered(64, 18, "DATA", 1);
}

void OLED_NOTIF::drawOta(uint32_t nowMs) {
  (void)nowMs;
  drawTextCentered(64, 4, "UPDATING", 1);
  display->drawRect(10, 16, 108, 10, SSD1306_WHITE);
  int w = (int)lroundf((106.0f * (float)_otaProgress) / 100.0f);
  if (w > 0) display->fillRect(11, 17, w, 8, SSD1306_WHITE);
}

void OLED_NOTIF::drawUnplugged(uint32_t nowMs) {
  const int phase = (nowMs / 220U) % 4;
  const int x = 44 + ((phase < 2) ? 0 : 2);
  const bool showX = ((nowMs / 260U) & 1U) == 0U;

  drawLogoBitmap(x, 4, false);
  if (showX) {
    display->drawLine(x - 2, 2, x + 26, 30, SSD1306_WHITE);
    display->drawLine(x - 2, 30, x + 26, 2, SSD1306_WHITE);
  }
}

void OLED_NOTIF::drawFaultSlide(uint32_t nowMs, OledOverlay ov) {
  const int phase = (nowMs / 180U) % 4;
  const bool showText = ((nowMs / 240U) & 1U) == 0U;

  const char* title = "FAULT";
  if (ov == OledOverlay::FAULT_ARC) title = "ARC FAULT";
  else if (ov == OledOverlay::FAULT_HEAT) title = "HEATING";
  else if (ov == OledOverlay::FAULT_OVERLOAD) title = "OVERLOAD";
  else if (ov == OledOverlay::FAULT_SURGE) title = "SURGE";
  else if (ov == OledOverlay::FAULT_TEMP_CRITICAL) title = "TEMP";

  if (showText) drawTextCentered(64, 0, title, 1);

  if (ov == OledOverlay::FAULT_ARC) {
    drawBoltIcon(24, 10, phase);
  } else if (ov == OledOverlay::FAULT_OVERLOAD) {
    drawArrowUpIcon(24, 10, phase);
  } else if (ov == OledOverlay::FAULT_HEAT) {
    drawFireIcon(24, 10, phase);
  } else if (ov == OledOverlay::FAULT_SURGE) {
    drawBoltIcon(24, 10, phase);
  } else {
    drawFireIcon(24, 10, phase);
  }
}

void OLED_NOTIF::render() {
  const uint32_t now = millis();

  if (_voltage <= 0.2f) {
    if (_noPowerSinceMs == 0) _noPowerSinceMs = now;
  } else {
    _noPowerSinceMs = 0;
  }

  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setTextSize(1);

  if (_bootUntilMs && (int32_t)(_bootUntilMs - now) > 0) {
    drawStartup(now);
  } else if (_otaActive) {
    drawOta(now);
  } else if (_collectUntilMs && (int32_t)(_collectUntilMs - now) > 0) {
    drawCollecting(now);
  } else if (_connectedUntilMs && (int32_t)(_connectedUntilMs - now) > 0) {
    drawConnected(now);
  } else if (_wifiBlocking) {
    drawWiFiWait(now, _wifiPortal);
  } else if (_overlay != OledOverlay::NONE) {
    drawFaultSlide(now, _overlay);
  } else if (_noPowerSinceMs && (now - _noPowerSinceMs) >= 10000UL) {
    drawUnplugged(now);
  } else {
    drawDashboard(now);
  }

  display->display();
}

void OLED_NOTIF::showStatus(const char* title, const char* msg) {
  display->clearDisplay();
  if (title) drawTextCentered(64, 4, title, 1);
  if (msg) drawTextCentered(64, 18, msg, 1);
  display->display();
}

void OLED_NOTIF::clear() {
  display->clearDisplay();
  display->display();
}
