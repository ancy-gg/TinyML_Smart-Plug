#include "OLED_NOTIF.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// 10x10 icons
static const unsigned char icon_arrow_up[] PROGMEM = {
  0x18, 0x00, 0x3C, 0x00, 0x7E, 0x00, 0xFF, 0xC0, 0x18, 0x00,
  0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00
};
static const unsigned char icon_lightning[] PROGMEM = {
  0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x1F, 0x80, 0x3F, 0xC0,
  0x07, 0xE0, 0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x18, 0x00
};
static const unsigned char icon_fire_better[] PROGMEM = {
  0x08, 0x00, 0x1C, 0x00, 0x36, 0x00, 0x63, 0x00, 0x5D, 0x00,
  0x5D, 0x00, 0x7F, 0x00, 0x3E, 0x00, 0x1C, 0x00, 0x08, 0x00
};

// 28x32 monochrome portrait generated from the uploaded professor photo.
static const unsigned char portrait28x32[] PROGMEM = {
  0x00,0x01,0xe0,0x00,0x00,0x0f,0xfc,0x00,0x00,0x3f,0xfe,0x00,0x00,0x7f,0xff,0x00,
  0x00,0xff,0xff,0x80,0x00,0xff,0xff,0xc0,0x01,0xff,0xff,0xc0,0x01,0xfc,0x3f,0xc0,
  0x01,0xfc,0x3f,0xe0,0x00,0xfe,0x1f,0xe0,0x00,0x76,0x0f,0xe0,0x00,0x70,0x08,0xc0,
  0x00,0x20,0x08,0xc0,0x00,0x00,0x00,0xc0,0x00,0x30,0x00,0xc0,0x00,0x00,0x01,0xb0,
  0x00,0x30,0x03,0xf0,0x00,0x30,0x03,0xf0,0x00,0x38,0x03,0xf0,0x00,0x19,0xc3,0xf0,
  0x06,0xff,0x03,0xf0,0x3f,0x7e,0x03,0xf0,0x1f,0xff,0x83,0xf0,0x1f,0xf9,0x87,0xf0,
  0x3f,0xf9,0x87,0xf0,0x13,0xf9,0x07,0xf0,0x67,0xf8,0x0f,0xf0,0xef,0xf8,0x0f,0xf0,
  0x8f,0xf8,0x0f,0xf0,0x0f,0xfc,0x0f,0xf0,0x1f,0xfc,0x0f,0xf0,0x3f,0xfc,0x0f,0xf0
};


OLED_NOTIF::OLED_NOTIF(uint8_t address) {
  _address = address;
  display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}

OLED_NOTIF::~OLED_NOTIF() {
  delete display;
}

bool OLED_NOTIF::begin() {
  if (!display->begin(SSD1306_SWITCHCAPVCC, _address)) {
    return false;
  }
  display->cp437(true);
  display->clearDisplay();
  display->display();
  return true;
}

void OLED_NOTIF::drawNormalPane(float current, uint32_t nowMs) {
  const int x0 = 64;
  const int phase = (nowMs / 140) % 48;

  display->setTextSize(1);
  display->setCursor(x0 + 7, 0);
  display->print("NORMAL");

  for (int x = x0 + 2; x < 126; ++x) {
    const float a = (float)(x + phase) * 0.28f;
    const int y = 18 + (int)roundf(sinf(a) * 2.0f);
    display->drawPixel(x, y, SSD1306_WHITE);
  }

  const int dotBase = x0 + 12;
  const int step = (nowMs / 240) % 3;
  for (int i = 0; i < 3; ++i) {
    const int r = (i == step) ? 2 : 1;
    display->fillCircle(dotBase + i * 10, 27, r, SSD1306_WHITE);
  }

  int bars = (int)(current * 2.0f + 0.5f);
  if (bars < 0) bars = 0;
  if (bars > 6) bars = 6;
  for (int i = 0; i < 6; ++i) {
    const int bx = x0 + 45 + i * 3;
    const int bh = 2 + i;
    if (i < bars) display->fillRect(bx, 30 - bh, 2, bh, SSD1306_WHITE);
    else display->drawRect(bx, 30 - bh, 2, bh, SSD1306_WHITE);
  }
}

void OLED_NOTIF::drawFaultPane(FaultState state, uint32_t nowMs) {
  const int centerX = 94;
  const bool blinkOn = ((nowMs / 400U) % 2U) == 0U;
  const int wobble = blinkOn ? 0 : 1;

  display->setTextSize(1);
  display->setCursor(centerX - 18, 0);
  display->print("STATUS");

  display->setTextSize(2);
  if (state == STATE_OVERLOAD) {
    const char* text = "LOAD";
    int16_t x1, y1; uint16_t w, h;
    display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    const int startX = centerX - 28;
    display->drawBitmap(startX - wobble, 16, icon_arrow_up, 10, 10, SSD1306_WHITE);
    display->setCursor(startX + 12, 14);
    display->print(text);
    display->drawBitmap(startX + 12 + w + 2 + wobble, 16, icon_arrow_up, 10, 10, SSD1306_WHITE);
  } else if (state == STATE_HEATING) {
    const char* text = "HEAT";
    int16_t x1, y1; uint16_t w, h;
    display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    const int startX = centerX - 28;
    display->drawBitmap(startX - wobble, 16, icon_fire_better, 10, 10, SSD1306_WHITE);
    display->setCursor(startX + 12, 14);
    display->print(text);
    display->drawBitmap(startX + 12 + w + 2 + wobble, 16, icon_fire_better, 10, 10, SSD1306_WHITE);
  } else if (state == STATE_ARCING) {
    const char* text = "ARC";
    int16_t x1, y1; uint16_t w, h;
    display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    const int startX = centerX - 24;
    display->drawBitmap(startX - wobble, 16, icon_lightning, 10, 10, SSD1306_WHITE);
    display->setCursor(startX + 12, 14);
    display->print(text);
    display->drawBitmap(startX + 12 + w + 2 + wobble, 16, icon_lightning, 10, 10, SSD1306_WHITE);
  } else {
    display->setCursor(centerX - 18, 14);
    display->print("READY");
  }

  if (blinkOn) {
    display->drawRect(64, 11, 63, 20, SSD1306_WHITE);
  }
}

void OLED_NOTIF::drawScreensaver(uint32_t nowMs) {
  const int bob = ((nowMs / 350U) % 2U) ? 1 : 0;
  const int tw = (nowMs / 220U) % 6;
  const int x = (SCREEN_WIDTH - 28) / 2;
  display->drawBitmap(x, bob, portrait28x32, 28, 32, SSD1306_WHITE);

  const int sx[6] = {14, 28, 44, 84, 100, 114};
  const int sy[6] = {6, 24, 10, 22, 7, 26};
  for (int i = 0; i < 6; ++i) {
    if (i == tw) {
      display->drawFastHLine(sx[i] - 1, sy[i], 3, SSD1306_WHITE);
      display->drawFastVLine(sx[i], sy[i] - 1, 3, SSD1306_WHITE);
    } else {
      display->drawPixel(sx[i], sy[i], SSD1306_WHITE);
    }
  }
}

void OLED_NOTIF::drawNoPowerPane() {
  display->drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 4, SSD1306_WHITE);
  display->setTextSize(2);
  display->setTextColor(SSD1306_WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  const char* line1 = "No";
  const char* line2 = "Power";
  display->getTextBounds(line1, 0, 0, &x1, &y1, &w, &h);
  display->setCursor((SCREEN_WIDTH - (int)w) / 2, 4);
  display->print(line1);
  display->getTextBounds(line2, 0, 0, &x1, &y1, &w, &h);
  display->setCursor((SCREEN_WIDTH - (int)w) / 2, 18);
  display->print(line2);
}

static void formatCompactUnit(char* dst, size_t n, float value, char unit, uint8_t fracDigits, float zeroThresh = 0.0005f) {
  if (!dst || n < 4) return;
  if (fabsf(value) < zeroThresh) value = 0.0f;

  char tmp[16];
  if (value == 0.0f) {
    snprintf(dst, n, "0%c", unit);
    return;
  }

  snprintf(tmp, sizeof(tmp), "%.*f", (int)fracDigits, value);

  // Trim trailing zeros and a dangling decimal point.
  char* p = tmp + strlen(tmp) - 1;
  while (p > tmp && *p == '0' && strchr(tmp, '.') != nullptr) {
    *p-- = '\0';
  }
  if (p > tmp && *p == '.') *p = '\0';

  snprintf(dst, n, "%s%c", tmp, unit);
}

void OLED_NOTIF::updateDashboard(float voltage, float current, float temperature, FaultState state) {
  const uint32_t now = millis();

  if (state != _lastState) {
    _lastState = state;
    _normalSinceMs = (state == STATE_NORMAL) ? now : 0;
  } else if (state == STATE_NORMAL && _normalSinceMs == 0) {
    _normalSinceMs = now;
  }

  if (voltage < 0.0f) voltage = 0.0f;
  if (current < 0.0f) current = 0.0f;

  const bool noPowerNow = (voltage <= 0.20f);
  if (noPowerNow) {
    if (_noPowerSinceMs == 0) _noPowerSinceMs = now;
  } else {
    _noPowerSinceMs = 0;
  }

  const bool showNoPower = (_noPowerSinceMs != 0) && ((now - _noPowerSinceMs) >= 10000UL);

  bool showScreensaver = false;
  if (!showNoPower && state == STATE_NORMAL && _normalSinceMs != 0) {
    const uint32_t held = now - _normalSinceMs;
    if (held >= 10000UL) {
      const uint32_t cyc = (held - 10000UL) % 15000UL;
      showScreensaver = (cyc < 5000UL);
    }
  }

  char vBuf[12];
  char iBuf[12];
  char tBuf[12];
  formatCompactUnit(vBuf, sizeof(vBuf), voltage, 'V', 2, 0.05f);
  formatCompactUnit(iBuf, sizeof(iBuf), current, 'A', 3, 0.002f);
  snprintf(tBuf, sizeof(tBuf), "%.1f", temperature);

  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setTextSize(1);

  if (showNoPower) {
    drawNoPowerPane();
    display->display();
    return;
  }

  if (showScreensaver) {
    drawScreensaver(now);
    display->display();
    return;
  }

  display->setCursor(0, 0);
  display->print(vBuf);

  display->setCursor(0, 11);
  display->print(iBuf);

  display->setCursor(0, 22);
  display->print(tBuf);
  display->print((char)248);
  display->print("C");

  display->drawFastVLine(60, 0, 32, SSD1306_WHITE);

  if (state == STATE_NORMAL) drawNormalPane(current, now);
  else drawFaultPane(state, now);

  display->display();
}

void OLED_NOTIF::showStatus(const char* title, const char* msg) {
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setTextSize(1);
  display->setCursor(0, 0);
  display->println(title);
  display->drawLine(0, 10, 128, 10, SSD1306_WHITE);
  display->setCursor(0, 15);
  display->println(msg);
  display->display();
}

void OLED_NOTIF::clear() {
  display->clearDisplay();
  display->display();
}
