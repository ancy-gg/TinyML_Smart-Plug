#ifndef OLED_NOTIF_H
#define OLED_NOTIF_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SmartPlugTypes.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1

enum class OledOverlay : uint8_t {
  NONE = 0,
  WIFI_WAIT,
  PORTAL,
  COLLECTING,
  OTA,
  FAULT_ARC,
  FAULT_HEAT,
  FAULT_OVERLOAD,
  FAULT_SURGE,
  FAULT_TEMP_CRITICAL,
  UNPLUGGED
};

class OLED_NOTIF {
public:
  OLED_NOTIF(uint8_t address = 0x3C);
  ~OLED_NOTIF();

  bool begin();

  void setMeasurements(float voltage, float current, float apparentPower, float temperature);
  void setState(FaultState state);
  void setWiFi(bool connected, int rssi, bool blocking, bool inPortal);
  void triggerCollecting(uint32_t durMs = 1000UL);
  void setOta(bool active, uint8_t progress = 0);
  void setOverlay(OledOverlay overlay);
  void clearOverlay();
  void render();
  void showStatus(const char* title, const char* msg);
  void clear();

private:
  Adafruit_SSD1306* display;
  uint8_t _address;

  float _voltage = 0.0f;
  float _current = 0.0f;
  float _apparentPower = 0.0f;
  float _temperature = 0.0f;
  FaultState _state = STATE_NORMAL;

  bool _wifiConnected = false;
  bool _wifiBlocking = false;
  bool _wifiPortal = false;
  int  _wifiRssi = -127;

  bool _otaActive = false;
  uint8_t _otaProgress = 0;
  uint32_t _collectUntilMs = 0;
  uint32_t _noPowerSinceMs = 0;
  OledOverlay _overlay = OledOverlay::NONE;

  void drawDashboard(uint32_t nowMs);
  void drawWiFiWait(uint32_t nowMs, bool portal);
  void drawCollecting(uint32_t nowMs);
  void drawOta(uint32_t nowMs);
  void drawUnplugged(uint32_t nowMs);
  void drawFaultSlide(uint32_t nowMs, OledOverlay ov);

  void drawWiFiBars(int x, int y, int bars, bool crossed);
  void drawCenteredText(const char* txt, int y, uint8_t size, int minX = 0);
  void drawPlugXIcon(int cx, int cy, bool blinkOn);
  void drawArcIcon(int x, int y, bool blinkOn);
  void drawUpArrow(int x, int y, int phase);
  void drawFireIcon(int x, int y, int sway);
};

#endif
