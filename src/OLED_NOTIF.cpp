#include "OLED_NOTIF.h"
#include <math.h>
#include <stdio.h>

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

// 24x32 monochrome portrait derived from the attached photo for the OLED idle screen.
static const unsigned char portrait24x32[] PROGMEM = {
  0x51,0x2a,0xa9,0x90,0x55,0x55,0x29,0x48,0x4a,0x59,0x12,0x55,0x90,0x64,0x0a,
  0x59,0x49,0xe4,0xa1,0x96,0xf4,0xd9,0x37,0xe4,0xb9,0x5d,0x6a,0x79,0x57,0xea,
  0xf9,0xae,0xf4,0xb9,0x7b,0xea,0xf9,0xd7,0xe4,0xf9,0x6e,0xea,0xf1,0x5b,0xa9,
  0xb9,0x6e,0x12,0xf9,0x2b,0x49,0xf9,0x5e,0x90,0xb1,0x0d,0xe5,0x7b,0x6a,0x81,
  0x0b,0xac,0x09,0x4b,0x79,0x22,0x13,0x50,0x89,0xa3,0xa4,0x52,0x05,0x10,0x02,
  0x28,0xa1,0xa8,0x40,0x48,0x42,0x08,0x40,0x49,0x60,0x01,0x96,0x90,0x04,0x0f,
  0x00,0x11,0xaa,0xa2,0x08,0x48
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
  const int tw = (nowMs / 220U) % 4;

  display->drawBitmap(2, bob, portrait24x32, 24, 32, SSD1306_WHITE);
  display->drawLine(29, 0, 29, 31, SSD1306_WHITE);

  display->setTextSize(1);
  display->setCursor(36, 3);
  display->print("ALL GOOD");
  display->setCursor(36, 14);
  display->print("SMARTPLUG");
  display->setCursor(36, 25);
  display->print(":)");

  const int sx[4] = {112, 118, 108, 121};
  const int sy[4] = {5, 11, 21, 27};
  for (int i = 0; i < 4; ++i) {
    if (i == tw) {
      display->drawFastHLine(sx[i] - 1, sy[i], 3, SSD1306_WHITE);
      display->drawFastVLine(sx[i], sy[i] - 1, 3, SSD1306_WHITE);
    } else {
      display->drawPixel(sx[i], sy[i], SSD1306_WHITE);
    }
  }
}

void OLED_NOTIF::updateDashboard(float voltage, float current, float temperature, FaultState state) {
  const uint32_t now = millis();

  if (state != _lastState) {
    _lastState = state;
    _normalSinceMs = (state == STATE_NORMAL) ? now : 0;
  } else if (state == STATE_NORMAL && _normalSinceMs == 0) {
    _normalSinceMs = now;
  }

  bool showScreensaver = false;
  if (state == STATE_NORMAL && _normalSinceMs != 0) {
    const uint32_t held = now - _normalSinceMs;
    if (held >= 10000UL) {
      const uint32_t cyc = (held - 10000UL) % 15000UL;
      showScreensaver = (cyc < 5000UL);
    }
  }

  char vBuf[12];
  char iBuf[12];
  char tBuf[12];
  if (voltage < 0.0f) voltage = 0.0f;
  if (current < 0.0f) current = 0.0f;
  snprintf(vBuf, sizeof(vBuf), "%06.2fV", voltage);
  snprintf(iBuf, sizeof(iBuf), "%06.3fA", current);
  snprintf(tBuf, sizeof(tBuf), "%05.2f", temperature);

  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setTextSize(1);

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
