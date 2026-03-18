#include "OLED_NOTIF.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static void trimLeadingZeros(char* s) {
  if (!s || !*s) return;
  bool neg = (s[0] == '-');
  char* p = s + (neg ? 1 : 0);
  while (p[0] == '0' && p[1] && p[1] != '.') {
    memmove(p, p + 1, strlen(p));
  }
  if (neg && s[1] == '\0') strcpy(s, "0");
}

static void fmtUnit(char* dst, size_t n, float v, const char* unit, uint8_t dec) {
  if (fabsf(v) < 0.0005f) v = 0.0f;
  if (fabsf(v) < 0.0005f) {
    snprintf(dst, n, "0%s", unit);
    return;
  }

  char num[16];
  snprintf(num, sizeof(num), "%.*f", (int)dec, v);
  trimLeadingZeros(num);
  snprintf(dst, n, "%s%s", num, unit);
}

static void fmtVA(char* dst, size_t n, float v) {
  if (fabsf(v) < 0.05f) {
    snprintf(dst, n, "0VA");
  } else if (fabsf(v) >= 1000.0f) {
    snprintf(dst, n, "%.0fVA", v);
  } else {
    char num[16];
    snprintf(num, sizeof(num), "%.1f", v);
    trimLeadingZeros(num);
    snprintf(dst, n, "%sVA", num);
  }
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

void OLED_NOTIF::drawWiFiBars(int x, int y, int bars, bool crossed) {
  bars = constrain(bars, 0, 4);
  for (int i = 0; i < 4; ++i) {
    const int bx = x + i * 3;
    const int bh = i + 1;
    const int by = y + (4 - bh);
    if (i < bars) display->fillRect(bx, by, 2, bh, SSD1306_WHITE);
    else          display->drawRect(bx, by, 2, bh, SSD1306_WHITE);
  }
  if (crossed) {
    display->drawLine(x - 1, y + 5, x + 12, y - 1, SSD1306_WHITE);
  }
}

void OLED_NOTIF::drawCenteredText(const char* txt, int y, uint8_t size, int minX) {
  if (!txt) txt = "";
  display->setTextSize(size);
  int16_t x1, y1;
  uint16_t w, h;
  display->getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  int x = (SCREEN_WIDTH - (int)w) / 2;
  if (x < minX) x = minX;
  display->setCursor(x, y);
  display->print(txt);
}

void OLED_NOTIF::drawPlugXIcon(int cx, int cy, bool blinkOn) {
  if (!blinkOn) return;

  const int px = cx - 7;
  const int py = cy - 2;

  display->fillRoundRect(px, py, 14, 8, 2, SSD1306_WHITE);
  display->fillRect(px + 3, py - 5, 2, 5, SSD1306_WHITE);
  display->fillRect(px + 9, py - 5, 2, 5, SSD1306_WHITE);
  display->fillRect(px + 6, py + 8, 2, 5, SSD1306_WHITE);
  display->drawLine(px + 7, py + 13, px + 7, py + 19, SSD1306_WHITE);

  display->drawLine(cx - 5, cy - 12, cx + 5, cy - 2, SSD1306_WHITE);
  display->drawLine(cx - 5, cy - 2, cx + 5, cy - 12, SSD1306_WHITE);
}

void OLED_NOTIF::drawArcIcon(int x, int y, bool blinkOn) {
  display->drawLine(x + 4,  y,     x + 10, y + 8,  SSD1306_WHITE);
  display->drawLine(x + 10, y + 8, x + 7,  y + 8,  SSD1306_WHITE);
  display->drawLine(x + 7,  y + 8, x + 13, y + 18, SSD1306_WHITE);
  display->drawLine(x + 13, y + 18, x + 9, y + 18, SSD1306_WHITE);
  if (blinkOn) {
    display->drawLine(x + 1,  y + 18, x + 6,  y + 10, SSD1306_WHITE);
    display->drawLine(x + 15, y + 18, x + 20, y + 10, SSD1306_WHITE);
  }
}

void OLED_NOTIF::drawUpArrow(int x, int y, int phase) {
  const int lift = phase & 0x03;
  display->drawFastVLine(x + 5, y + 6 - lift, 10, SSD1306_WHITE);
  display->drawLine(x + 5, y - lift, x,     y + 5 - lift, SSD1306_WHITE);
  display->drawLine(x + 5, y - lift, x + 10, y + 5 - lift, SSD1306_WHITE);
}

void OLED_NOTIF::drawFireIcon(int x, int y, int sway) {
  const int ox = sway - 1;
  display->drawCircle(x + 6 + ox, y + 11, 5, SSD1306_WHITE);
  display->drawLine(x + 6 + ox, y + 1, x + 2 + ox, y + 10, SSD1306_WHITE);
  display->drawLine(x + 6 + ox, y + 1, x + 10 + ox, y + 10, SSD1306_WHITE);
  display->drawLine(x + 4 + ox, y + 9, x + 8 + ox, y + 9, SSD1306_WHITE);
}

void OLED_NOTIF::drawDashboard(uint32_t nowMs) {
  (void)nowMs;
  const int bars = _wifiConnected ? ((_wifiRssi >= -55) ? 4 : (_wifiRssi >= -67) ? 3 : (_wifiRssi >= -78) ? 2 : 1) : 0;
  drawWiFiBars(2, 1, bars, !_wifiConnected);

  drawCenteredText("NORMAL", 0, 2, 18);

  char v[16], i[16], p[16], t[16];
  fmtUnit(v, sizeof(v), _voltage, "V", 1);
  fmtUnit(i, sizeof(i), _current, "A", 2);
  fmtVA(p, sizeof(p), _apparentPower);
  fmtUnit(t, sizeof(t), _temperature, "C", 1);

  display->setTextSize(1);

  display->setCursor(0, 18);
  display->print(v);

  int16_t x1, y1;
  uint16_t w, h;
  display->getTextBounds(p, 0, 0, &x1, &y1, &w, &h);
  display->setCursor(SCREEN_WIDTH - (int)w, 18);
  display->print(p);

  display->setCursor(0, 24);
  display->print(i);

  display->getTextBounds(t, 0, 0, &x1, &y1, &w, &h);
  display->setCursor(SCREEN_WIDTH - (int)w, 24);
  display->print(t);
}

void OLED_NOTIF::drawWiFiWait(uint32_t nowMs, bool portal) {
  const int phase = (nowMs / 220U) % 4;
  drawWiFiBars(2, 1, constrain(phase + 1, 1, 4), false);

  drawCenteredText("WIFI", 7, 1, 0);
  drawCenteredText(portal ? "PORTAL" : "CONNECTING", 17, 1, 0);

  if (!portal) {
    const int dots = (phase % 3) + 1;
    for (int i = 0; i < dots; ++i) {
      display->fillCircle(58 + i * 6, 29, 1, SSD1306_WHITE);
    }
  }
}

void OLED_NOTIF::drawCollecting(uint32_t nowMs) {
  (void)nowMs;
  drawCenteredText("LOGGING", 2, 2, 0);
  drawCenteredText("COLLECTING", 20, 1, 0);
}

void OLED_NOTIF::drawOta(uint32_t nowMs) {
  (void)nowMs;
  drawCenteredText("UPDATING", 2, 1, 0);
  display->drawRect(8, 16, 112, 10, SSD1306_WHITE);
  int w = (int)lroundf((110.0f * (float)_otaProgress) / 100.0f);
  if (w > 0) display->fillRect(9, 17, w, 8, SSD1306_WHITE);
}

void OLED_NOTIF::drawUnplugged(uint32_t nowMs) {
  const bool blinkOn = (((nowMs / 320U) & 1U) == 0U);
  drawPlugXIcon(64, 17, blinkOn);
}

void OLED_NOTIF::drawFaultSlide(uint32_t nowMs, OledOverlay ov) {
  const bool blinkOn = (((nowMs / 220U) & 1U) == 0U);

  if (ov == OledOverlay::FAULT_ARC) {
    drawArcIcon(10, 6, blinkOn);
    if (blinkOn) drawCenteredText("ARC", 8, 2, 34);
  } else if (ov == OledOverlay::FAULT_OVERLOAD) {
    drawUpArrow(12, 6, (int)((nowMs / 160U) % 4));
    if (blinkOn) drawCenteredText("OVERLOAD", 10, 1, 34);
  } else if (ov == OledOverlay::FAULT_HEAT) {
    drawFireIcon(11, 6, (int)((nowMs / 180U) % 3));
    if (blinkOn) drawCenteredText("HEATING", 10, 1, 34);
  } else if (ov == OledOverlay::FAULT_SURGE) {
    drawArcIcon(10, 6, blinkOn);
    if (blinkOn) drawCenteredText("SURGE", 8, 2, 34);
  } else {
    drawFireIcon(11, 6, (int)((nowMs / 180U) % 3));
    if (blinkOn) drawCenteredText("TEMP", 8, 2, 34);
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

  if (_otaActive) {
    drawOta(now);
  } else if (_overlay != OledOverlay::NONE) {
    drawFaultSlide(now, _overlay);
  } else if (_collectUntilMs && (int32_t)(_collectUntilMs - now) > 0) {
    drawCollecting(now);
  } else if (_wifiBlocking) {
    drawWiFiWait(now, _wifiPortal);
  } else if (_noPowerSinceMs && (now - _noPowerSinceMs) >= 10000UL) {
    drawUnplugged(now);
  } else {
    drawDashboard(now);
  }
  display->display();
}

void OLED_NOTIF::showStatus(const char* title, const char* msg) {
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  drawCenteredText(title ? title : "STATUS", 2, 1, 0);
  drawCenteredText(msg ? msg : "", 18, 1, 0);
  display->display();
}

void OLED_NOTIF::clear() {
  display->clearDisplay();
  display->display();
}
