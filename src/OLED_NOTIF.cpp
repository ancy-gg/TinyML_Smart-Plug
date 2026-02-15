#include "OLED_NOTIF.h"

// -10 by 10 Bitmaps

// Arrow Up
const unsigned char icon_arrow_up [] PROGMEM = {
  0x18, 0x00, 0x3C, 0x00, 0x7E, 0x00, 0xFF, 0xC0, 0x18, 0x00, 
  0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00 
};

// Lightning
const unsigned char icon_lightning [] PROGMEM = {
  0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x1F, 0x80, 0x3F, 0xC0, 
  0x07, 0xE0, 0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x18, 0x00 
};

// Fire
const unsigned char icon_fire_better [] PROGMEM = {
  0x08, 0x00, 0x1C, 0x00, 0x36, 0x00, 0x63, 0x00, 0x5D, 0x00, 
  0x5D, 0x00, 0x7F, 0x00, 0x3E, 0x00, 0x1C, 0x00, 0x08, 0x00 
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

void OLED_NOTIF::updateDashboard(float voltage, float current, float temperature, FaultState state) {
    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    display->setTextSize(1); 

    // Data (LEFT)
    display->setCursor(0, 0); 
    display->print(voltage, 1); display->print("V");

    display->setCursor(0, 11); 
    display->print(current, 2); display->print("A");

    display->setCursor(0, 22); 
    display->print(temperature, 1); display->print((char)248); display->print("C");


    // Status (RIGHT)
    int centerX = 91;

    // Header
    const char* label = "STATUS";
    int16_t x1, y1;
    uint16_t w, h;
    display->setTextSize(1);
    display->getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    display->setCursor(centerX - (w / 2), 0); 
    display->print(label);

    // State Logic with Blinking for Faults
    bool showContent = true;
    if (state != STATE_NORMAL) {
        if ((millis() / 750) % 2 != 0) showContent = false;
    }

    if (showContent) {
        display->setTextSize(2);
        
        if (state == STATE_NORMAL) {
             const char* text = "NORMAL";
             display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
             display->setCursor(centerX - (w / 2), 14);
             display->print(text);
        } 
        else if (state == STATE_OVERLOAD) {
             // Overload
             const char* text = "LOAD";
             display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
             int startX = centerX - (72 / 2); 
             
             display->drawBitmap(startX, 16, icon_arrow_up, 10, 10, SSD1306_WHITE);
             display->setCursor(startX + 12, 14);
             display->print(text);
             display->drawBitmap(startX + 12 + w + 2, 16, icon_arrow_up, 10, 10, SSD1306_WHITE);
        } 
        else if (state == STATE_HEATING) {
             // Heating
             const char* text = "HEAT";
             display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
             int startX = centerX - (72 / 2); 
             
             display->drawBitmap(startX, 16, icon_fire_better, 10, 10, SSD1306_WHITE);
             display->setCursor(startX + 12, 14);
             display->print(text);
             display->drawBitmap(startX + 12 + w + 2, 16, icon_fire_better, 10, 10, SSD1306_WHITE);
        }
        else if (state == STATE_ARCING) {
             // Arcing
             const char* text = "ARC";
             display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
             int startX = centerX - (60 / 2);
             
             display->drawBitmap(startX, 16, icon_lightning, 10, 10, SSD1306_WHITE);
             display->setCursor(startX + 12, 14);
             display->print(text);
             display->drawBitmap(startX + 12 + w + 2, 16, icon_lightning, 10, 10, SSD1306_WHITE);
        }
    }

    display->display();
}

void OLED_NOTIF::showStatus(const char* title, const char* msg) {
    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    
    // Draw Title
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->println(title);
    
    // Draw Divider Line
    display->drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Draw Message
    display->setTextSize(1);
    display->setCursor(0, 15);
    display->println(msg);
    
    display->display();
}

void OLED_NOTIF::clear() {
    display->clearDisplay();
    display->display();
}
