#pragma once

#include <Arduino.h>

// Aggressive defaults for a 5V 28BYJ-48. If the motor chatters or skips,
// raise STEP_US values toward 800-1200 us.
static const uint32_t DEFAULT_ARC_STEP_US    = 500;
static const uint32_t DEFAULT_NUDGE_STEP_US  = 700;
static const uint32_t DEFAULT_HOME_STEP_US   = 700;
static const uint32_t DEFAULT_JOG_STEP_US    = 600;
static const uint32_t STARTUP_STEP_US        = 900;

static const int32_t DEFAULT_ARC_STROKE_STEPS = 300;  // one-shot out and back
static const int32_t DEFAULT_NUDGE_STEPS      = 40;
static const int32_t STARTUP_WIGGLE_STEPS     = 80;

static const uint32_t MQTT_RECONNECT_MS       = 2500;
static const uint32_t WIFI_RECONNECT_MS       = 5000;
static const uint32_t STATUS_PUBLISH_MS       = 5000;
