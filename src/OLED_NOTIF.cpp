#include "OLED_NOTIF.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static void fmt3(char* dst, size_t n, float v, char unit, int frac = 1) {
  if (!dst || n < 4) return;
  if (fabsf(v) < 0.0005f) v = 0.0f;
  char buf[16];
  snprintf(buf, sizeof(buf), "%.*f", frac, v);
  char* p = buf + strlen(buf) - 1;
  while (p > buf && *p == '0' && strchr(buf, '.') != nullptr) *p-- = '\0';
  if (p > buf && *p == '.') *p = '\0';
  snprintf(dst, n, "%s%c", buf, unit);
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
  _voltage = fmaxf(0.0f, voltage);
  _current = fmaxf(0.0f, current);
  _apparentPower = fmaxf(0.0f, apparentPower);
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

void OLED_NOTIF::drawStatusBand(const char* txt, bool invert) {
  display->fillRoundRect(0, 0, 128, 10, 3, invert ? SSD1306_WHITE : SSD1306_BLACK);
  display->drawRoundRect(0, 0, 128, 10, 3, SSD1306_WHITE);
  display->setTextSize(1);
  display->setTextColor(invert ? SSD1306_BLACK : SSD1306_WHITE);
  int16_t x1, y1; uint16_t w, h;
  display->getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  display->setCursor((128 - (int)w) / 2, 1);
  display->print(txt);
  display->setTextColor(SSD1306_WHITE);
}

void OLED_NOTIF::drawWiFiIcon(int x, int y, int bars, bool crossed) {
  for (int i = 0; i < 3; ++i) {
    const int w = 2 + i * 2;
    const int yy = y - i * 2;
    if (i < bars) display->drawFastHLine(x - w / 2, yy, w, SSD1306_WHITE);
  }
  display->fillCircle(x, y + 1, 1, SSD1306_WHITE);
  if (crossed) {
    display->drawLine(x - 5, y - 6, x + 5, y + 3, SSD1306_WHITE);
    display->drawLine(x - 5, y + 3, x + 5, y - 6, SSD1306_WHITE);
  }
}

void OLED_NOTIF::drawBottomWiFiBar(uint32_t nowMs) {
  display->drawFastHLine(0, 24, 128, SSD1306_WHITE);
  display->setCursor(2, 25);
  if (_wifiPortal) display->print("Portal");
  else if (_wifiBlocking) display->print("WiFi");
  else if (_wifiConnected) display->print("WiFi OK");
  else display->print("WiFi Off");

  const int dots = (nowMs / 280U) % 4;
  for (int i = 0; i < 3; ++i) {
    display->fillCircle(42 + i * 4, 28, (i < dots) ? 1 : 0, SSD1306_WHITE);
  }

  int bars = 0;
  if (_wifiConnected) {
    if (_wifiRssi >= -60) bars = 3;
    else if (_wifiRssi >= -72) bars = 2;
    else if (_wifiRssi >= -84) bars = 1;
  }
  drawWiFiIcon(120, 29, bars, !_wifiConnected);
}

void OLED_NOTIF::drawDashboard(uint32_t nowMs) {
  const char* st = "NORMAL";
  if (_state == STATE_ARCING) st = "ARC FAULT";
  else if (_state == STATE_HEATING) st = "HEATING";
  else if (_state == STATE_OVERLOAD) st = "OVERLOAD";
  drawStatusBand(st, true);

  char b0[12], b1[12], b2[12], b3[12];
  fmt3(b0, sizeof(b0), _voltage, 'V', 1);
  fmt3(b1, sizeof(b1), _current, 'A', 3);
  fmt3(b2, sizeof(b2), _apparentPower, 'P', 1);
  snprintf(b3, sizeof(b3), "%.1fC", _temperature);

  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);

  display->setCursor(2, 12);  display->print("V:"); display->print(b0);
  display->setCursor(66, 12); display->print("I:"); display->print(b1);
  display->setCursor(2, 18);  display->print("P:"); display->print(b2);
  display->setCursor(66, 18); display->print("T:"); display->print(b3);

  drawBottomWiFiBar(nowMs);
}

void OLED_NOTIF::drawWiFiWait(uint32_t nowMs, bool portal) {
  drawStatusBand(portal ? "SETUP PORTAL" : "WIFI STARTUP", true);
  const int phase = (nowMs / 180U) % 4;
  const int x = 64;
  const int y = 18;
  drawWiFiIcon(x, y, phase == 0 ? 0 : phase, false);
  display->setCursor(20, 14);
  display->print(portal ? "Join AP to configure" : "Connecting...");
  display->setCursor(22, 20);
  display->print(portal ? "TinyML SmartPlug" : "ADC held deterministic");
}

void OLED_NOTIF::drawUploadArrow(int x, int y, int phase) {
  display->drawRect(x + 1, y + 7, 10, 5, SSD1306_WHITE);
  display->fillRect(x + 5, y + 2 + phase, 2, 6, SSD1306_WHITE);
  display->fillTriangle(x + 6, y, x + 2, y + 4 + phase, x + 10, y + 4 + phase, SSD1306_WHITE);
}

void OLED_NOTIF::drawCollecting(uint32_t nowMs) {
  drawStatusBand("COLLECTING DATA", true);
  const int phase = (nowMs / 140U) % 4;
  drawUploadArrow(56, 12, phase);
  display->setCursor(22, 14);
  display->print("Sampling for logger");
  display->setCursor(28, 20);
  display->print("Upload cue active");
}

void OLED_NOTIF::drawOta(uint32_t nowMs) {
  drawStatusBand("UPDATING", true);
  const int phase = (nowMs / 180U) % 4;
  drawUploadArrow(56, 12, phase);
  display->setCursor(24, 14);
  display->print("Firmware update");
  display->drawRect(8, 24, 112, 6, SSD1306_WHITE);
  const int w = (int)lroundf((110.0f * (float)_otaProgress) / 100.0f);
  if (w > 0) display->fillRect(9, 25, w, 4, SSD1306_WHITE);
}

void OLED_NOTIF::drawUnplugged(uint32_t nowMs) {
  drawStatusBand("DEVICE UNPLUGGED", true);
  const int shift = (nowMs / 180U) % 12;
  // socket
  display->drawRect(14, 12, 20, 12, SSD1306_WHITE);
  display->fillRect(20, 15, 2, 5, SSD1306_WHITE);
  display->fillRect(26, 15, 2, 5, SSD1306_WHITE);
  // plug body moving out
  const int px = 54 + shift;
  display->drawRect(px, 14, 14, 10, SSD1306_WHITE);
  display->drawFastHLine(34, 18, px - 34, SSD1306_WHITE);
  display->drawFastVLine(px + 2, 11, 3, SSD1306_WHITE);
  display->drawFastVLine(px + 10, 11, 3, SSD1306_WHITE);
  display->drawLine(95, 12, 111, 24, SSD1306_WHITE);
  display->drawLine(95, 24, 111, 12, SSD1306_WHITE);
}

void OLED_NOTIF::drawFaultSlide(uint32_t nowMs, OledOverlay ov) {
  const bool blink = ((nowMs / 220U) & 1U) == 0U;
  const char* title = "FAULT";
  if (ov == OledOverlay::FAULT_ARC) title = "ARC FAULT";
  else if (ov == OledOverlay::FAULT_HEAT) title = "HEATING";
  else if (ov == OledOverlay::FAULT_OVERLOAD) title = "OVERLOAD";
  else if (ov == OledOverlay::FAULT_SURGE) title = "SURGE";
  else if (ov == OledOverlay::FAULT_TEMP_CRITICAL) title = "TEMP CRIT";
  drawStatusBand(title, blink);

  display->setTextSize(2);
  display->setTextColor(SSD1306_WHITE);
  if (ov == OledOverlay::FAULT_ARC) {
    display->drawLine(18, 24, 24, 14, SSD1306_WHITE);
    display->drawLine(24, 14, 22, 20, SSD1306_WHITE);
    display->drawLine(22, 20, 28, 20, SSD1306_WHITE);
    display->drawLine(28, 20, 22, 30, SSD1306_WHITE);
    display->setCursor(40, 14); display->print("ARC");
  } else if (ov == OledOverlay::FAULT_HEAT || ov == OledOverlay::FAULT_TEMP_CRITICAL) {
    display->setCursor(18, 14); display->print("HEAT");
  } else if (ov == OledOverlay::FAULT_SURGE) {
    display->setCursor(16, 14); display->print("SURG");
  } else {
    display->setCursor(10, 14); display->print("LOAD");
  }
  display->setTextSize(1);
  display->setCursor(10, 24);
  display->print("Protection active");
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
  drawStatusBand(title, true);
  display->setTextSize(1);
  display->setCursor(4, 14);
  display->print(msg ? msg : "");
  display->display();
}

void OLED_NOTIF::clear() {
  display->clearDisplay();
  display->display();
}
