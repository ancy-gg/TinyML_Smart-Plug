#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>

class OTAHandler {
public:
  // Start ArduinoOTA after WiFi is connected
  bool begin(const char* mdnsName,
             const char* password,
             uint16_t port = 3232);

  // Call frequently in loop (non-blocking)
  void handle();

  // Disable OTA if WiFi drops (optional)
  void setEnabled(bool en);

  bool isReady() const { return _started && _enabled; }
  String ip() const { return _ip; }

private:
  bool _enabled = false;
  bool _started = false;
  String _ip = "";
};

#endif