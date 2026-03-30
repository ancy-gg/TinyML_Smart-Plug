#ifndef NotificationOLED_H
#define NotificationOLED_H

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
  FAULT_UNDERVOLT,
  FAULT_OVERVOLT,
  FAULT_SURGE,
  FAULT_TEMP_CRITICAL,
  UNPLUGGED
};

class NotificationOLED {
public:
  NotificationOLED(uint8_t address = 0x3C);
  ~NotificationOLED();

  bool begin();

  void setMeasurements(float voltage, float current, float apparentPower, float temperature);
  void setState(FaultState state);
  void setWiFi(bool connected, int rssi, bool blocking, bool inPortal, bool timedOut = false, bool apWindow = false);
  void triggerCollecting(uint32_t durMs = 1000UL);
  void triggerNotice(const char* line1, const char* line2 = "", uint32_t durMs = 1200UL);
  void triggerConnected(uint32_t durMs = 900UL);
  void startBootSequence(uint32_t durMs = 3000UL);
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
  bool _wifiTimedOut = false;
  bool _wifiApWindow = false;
  int  _wifiRssi = -127;

  bool _otaActive = false;
  uint8_t _otaProgress = 0;
  uint32_t _collectUntilMs = 0;
  uint32_t _noticeUntilMs = 0;
  char _noticeLine1[17] = {0};
  char _noticeLine2[17] = {0};
  uint32_t _bootUntilMs = 0;
  uint32_t _bootStartMs = 0;
  uint32_t _connectedUntilMs = 0;
  uint32_t _connectedStartMs = 0;
  uint32_t _noPowerSinceMs = 0;
  OledOverlay _overlay = OledOverlay::NONE;

  void drawDashboard(uint32_t nowMs);
  void drawBootSequence(uint32_t nowMs);
  void drawConnected(uint32_t nowMs);
  void drawWiFiWait(uint32_t nowMs, bool portal);
  void drawCollecting(uint32_t nowMs);
  void drawNotice(uint32_t nowMs);
  void drawOta(uint32_t nowMs);
  void drawUnplugged(uint32_t nowMs);
  void drawFaultSlide(uint32_t nowMs, OledOverlay ov);

  void drawWiFiBars(int x, int y, int bars, bool crossed);
  void drawLogo(int x, int y, bool invert = false);
  void drawCenteredText(const char* txt, int y, uint8_t size = 1);
  void drawRightText(const char* txt, int xRight, int y, uint8_t size = 1);
};

#endif
