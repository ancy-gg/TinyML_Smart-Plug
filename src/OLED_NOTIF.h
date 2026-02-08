#ifndef OLED_NOTIF_H
#define OLED_NOTIF_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1 

enum FaultState {
    STATE_NORMAL,
    STATE_OVERLOAD,
    STATE_HEATING,
    STATE_ARCING
};

class OLED_NOTIF {
private:
    Adafruit_SSD1306* display;
    uint8_t _address;

public:
    OLED_NOTIF(uint8_t address = 0x3C);
    ~OLED_NOTIF();
    
    bool begin();
    
    // Updated to include temperature
    void updateDashboard(float voltage, float current, float temperature, FaultState state);
    void showStatus(const char* title, const char* msg);    
    void clear();
};

#endif