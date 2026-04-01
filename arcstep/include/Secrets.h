#pragma once

// WiFiManager will handle Wi-Fi provisioning through its captive portal.
// No SSID/PASSWORD are needed here anymore.
static const char* WM_AP_NAME = "ArcGenerator-Setup";
static const char* WM_AP_PASS = "12345678";   // must be at least 8 chars if used
static const uint16_t WM_PORTAL_TIMEOUT_S = 180;

// HiveMQ Cloud credentials.
static const char* MQTT_HOST = "fb1c9a9111b64467abcbc907ca756ee4.s1.eu.hivemq.cloud";
static const uint16_t MQTT_PORT = 8883;
static const char* MQTT_USER = "arcgenerator";
static const char* MQTT_PASS = "Stepper1234";

// Topic base for this project.
static const char* MQTT_BASE = "arcgenerator";
