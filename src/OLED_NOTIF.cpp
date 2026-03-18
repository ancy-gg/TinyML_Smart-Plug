#include "OLED_NOTIF.h"
#include <math.h>

static void fmt3(char* dst, size_t n, float v, uint8_t dec) {
  if (fabsf(v) < 0.0005f) v = 0.0f;
  snprintf(dst, n, "%.*f", (int)dec, v);
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

void OLED_NOTIF::drawWiFiIcon(int x, int y, int bars, bool crossed) {
  bars = constrain(bars, 0, 4);
  // 4-bar tiny indicator, top-left friendly.
  for (int i = 0; i < 4; ++i) {
    int bx = x + i * 3;
    int bh = i + 1;
    int by = y + (4 - bh);
    if (i < bars) display->fillRect(bx, by, 2, bh, SSD1306_WHITE);
    else display->drawRect(bx, by, 2, bh, SSD1306_WHITE);
  }
  if (crossed) {
    display->drawLine(x - 1, y + 5, x + 12, y - 1, SSD1306_WHITE);
  }
}

void OLED_NOTIF::drawUploadArrow(int x, int y, int phase) {
  const int lift = phase & 0x03;
  display->drawFastVLine(x + 3, y + 4 - lift, 7, SSD1306_WHITE);
  display->drawLine(x + 3, y - lift, x, y + 3 - lift, SSD1306_WHITE);
  display->drawLine(x + 3, y - lift, x + 6, y + 3 - lift, SSD1306_WHITE);
  display->drawRect(x - 1, y + 11, 9, 3, SSD1306_WHITE);
}

void OLED_NOTIF::drawStatusBand(const char* txt, bool invert) {
  if (invert) display->fillRect(14, 0, 114, 8, SSD1306_WHITE);
  display->setTextColor(invert ? SSD1306_BLACK : SSD1306_WHITE);
  display->setTextSize(1);
  int16_t x1, y1; uint16_t w, h;
  display->getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  int tx = 14 + ((114 - (int)w) / 2);
  display->setCursor(tx, 0);
  display->print(txt);
  display->setTextColor(SSD1306_WHITE);
}

void OLED_NOTIF::drawBottomWiFiBar(uint32_t nowMs) {
  display->drawFastHLine(0, 31, 128, SSD1306_WHITE);
  display->setCursor(2, 24);
  if (_wifiConnected) display->print("WiFi");
  else if (_wifiPortal) display->print("Portal");
  else if (_wifiBlocking) display->print("Connect");
  else display->print("Offline");

  int step = (nowMs / 260U) % 3;
  for (int i = 0; i < 3; ++i) {
    int r = (i == step) ? 2 : 1;
    display->fillCircle(52 + i * 6, 27, r, SSD1306_WHITE);
  }

  if (_wifiConnected) {
    drawWiFiIcon(114, 24, (_wifiRssi >= -55) ? 4 : (_wifiRssi >= -67) ? 3 : (_wifiRssi >= -78) ? 2 : 1, false);
  } else {
    drawWiFiIcon(114, 24, 1, true);
  }
}

void OLED_NOTIF::drawDashboard(uint32_t nowMs) {
  const char* title = "NORMAL";
  if (_state == STATE_ARCING) title = "ARC";
  else if (_state == STATE_HEATING) title = "HEAT";
  else if (_state == STATE_OVERLOAD) title = "LOAD";

  drawStatusBand(title, true);
  drawWiFiIcon(1, 1, _wifiConnected ? ((_wifiRssi >= -55) ? 4 : (_wifiRssi >= -67) ? 3 : (_wifiRssi >= -78) ? 2 : 1) : 0, !_wifiConnected);

  char v[12], i[12], p[12], t[12];
  fmt3(v, sizeof(v), _voltage, 1);
  fmt3(i, sizeof(i), _current, 3);
  fmt3(p, sizeof(p), _apparentPower, 1);
  fmt3(t, sizeof(t), _temperature, 1);

  display->setTextSize(1);
  display->setCursor(0, 10); display->print("V:"); display->print(v);
  display->setCursor(68, 10); display->print("P:"); display->print(p);
  display->setCursor(0, 19); display->print("I:"); display->print(i);
  display->setCursor(68, 19); display->print("T:"); display->print(t);

  drawBottomWiFiBar(nowMs);
}

void OLED_NOTIF::drawWiFiWait(uint32_t nowMs, bool portal) {
  drawStatusBand(portal ? "SETUP MODE" : "WIFI START", true);
  int phase = (nowMs / 220U) % 4;
  int bars = phase + 1;
  drawWiFiIcon(14, 12, bars, false);
  display->setCursor(30, 12);
  display->print(portal ? "Join AP" : "Connecting");
  drawBottomWiFiBar(nowMs);
}

void OLED_NOTIF::drawCollecting(uint32_t nowMs) {
  drawStatusBand("COLLECTING DATA", true);
  int phase = (nowMs / 150U) % 4;
  drawUploadArrow(18, 10, phase);
  display->setCursor(34, 13);
  display->print("Logging...");
  drawBottomWiFiBar(nowMs);
}

void OLED_NOTIF::drawOta(uint32_t nowMs) {
  (void)nowMs;
  drawStatusBand("UPDATING", true);
  display->setCursor(8, 12);
  display->print("Firmware");
  display->drawRect(8, 20, 112, 8, SSD1306_WHITE);
  int w = (int)lroundf((110.0f * (float)_otaProgress) / 100.0f);
  if (w > 0) display->fillRect(9, 21, w, 6, SSD1306_WHITE);
}

void OLED_NOTIF::drawUnplugged(uint32_t nowMs) {
  drawStatusBand("UNPLUGGED", true);
  // clean small plug + animated X
  int px = 28 + (int)((nowMs / 180U) % 7);
  display->drawRect(px, 13, 14, 8, SSD1306_WHITE);        // plug body
  display->drawFastVLine(px + 3, 10, 3, SSD1306_WHITE);   // prongs
  display->drawFastVLine(px + 10, 10, 3, SSD1306_WHITE);
  display->drawFastHLine(px + 14, 16, 20, SSD1306_WHITE); // cord
  display->drawRect(54, 12, 12, 10, SSD1306_WHITE);       // socket
  display->fillRect(57, 14, 1, 4, SSD1306_WHITE);
  display->fillRect(61, 14, 1, 4, SSD1306_WHITE);
  display->drawLine(88, 11, 104, 25, SSD1306_WHITE);
  display->drawLine(88, 25, 104, 11, SSD1306_WHITE);
  drawBottomWiFiBar(nowMs);
}

void OLED_NOTIF::drawFaultSlide(uint32_t nowMs, OledOverlay ov) {
  const char* title = "FAULT";
  const char* body = "Protection on";
  if (ov == OledOverlay::FAULT_ARC) title = "ARC FAULT";
  else if (ov == OledOverlay::FAULT_HEAT) title = "HEATING";
  else if (ov == OledOverlay::FAULT_OVERLOAD) title = "OVERLOAD";
  else if (ov == OledOverlay::FAULT_SURGE) title = "SURGE";
  else if (ov == OledOverlay::FAULT_TEMP_CRITICAL) title = "TEMP CRIT";
  drawStatusBand(title, true);

  bool blink = ((nowMs / 220U) & 1U) == 0U;
  if (ov == OledOverlay::FAULT_ARC) {
    display->drawLine(10, 22, 16, 12, SSD1306_WHITE);
    display->drawLine(16, 12, 14, 17, SSD1306_WHITE);
    display->drawLine(14, 17, 20, 17, SSD1306_WHITE);
    display->drawLine(20, 17, 14, 27, SSD1306_WHITE);
  } else if (ov == OledOverlay::FAULT_SURGE) {
    display->drawFastVLine(15, 11, 12, SSD1306_WHITE);
    if (blink) display->drawCircle(15, 17, 8, SSD1306_WHITE);
  } else {
    display->drawRect(7, 11, 16, 12, SSD1306_WHITE);
    if (blink) display->fillRect(11, 14, 8, 6, SSD1306_WHITE);
  }
  display->setCursor(30, 13);
  display->print(body);
  drawBottomWiFiBar(nowMs);
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
  drawStatusBand(title ? title : "STATUS", true);
  display->setCursor(4, 14);
  display->print(msg ? msg : "");
  display->display();
}

void OLED_NOTIF::clear() {
  display->clearDisplay();
  display->display();
}
