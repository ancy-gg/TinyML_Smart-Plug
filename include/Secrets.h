#pragma once
#include <Arduino.h>

static const char* MQTT_HOST = "fb1c9a9111b64467abcbc907ca756ee4.s1.eu.hivemq.cloud";
static constexpr uint16_t MQTT_PORT = 8883; // HiveMQ Cloud TLS port; this project uses plain TCP client by default.
static const char* MQTT_USER = "arcgenerator";
static const char* MQTT_PASS = "Stepper1234";

// WiFiManager AP
static const char* CONFIG_AP_NAME = "ArcGenerator-Setup";
static const char* CONFIG_AP_PASS = "12345678";
