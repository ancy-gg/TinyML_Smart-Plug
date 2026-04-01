#pragma once
#include <Arduino.h>

// Application-level direction fix: true swaps forward/reverse.
static constexpr bool INVERT_APPLICATION_DIR = true;

// Step timing: lower microseconds = faster, but too low will stall/chatter.
static constexpr uint32_t STARTUP_STEP_US = 1400;
static constexpr uint32_t DEFAULT_TEST_STEP_US = 320;
static constexpr uint32_t DEFAULT_TEST_PAUSE_MS = 10;
static constexpr uint32_t DEFAULT_NUDGE_STEP_US = 700;
static constexpr uint32_t DEFAULT_JOG_STEP_US = 520;
static constexpr uint32_t DEFAULT_SHOT_STEP_US = 420;
static constexpr uint32_t MIN_STEP_US = 180;
static constexpr uint32_t MAX_STEP_US = 4000;

// Travel sizes.
static constexpr int32_t STARTUP_WIGGLE_STEPS = 80;
static constexpr int32_t DEFAULT_MANUAL_STEPS = 80;
static constexpr int32_t DEFAULT_TEST_STEPS = 320;
static constexpr int32_t MIN_STROKE_STEPS = 1;
static constexpr int32_t MAX_STROKE_STEPS = 1200;

// Portal.
static constexpr uint16_t CONFIG_PORTAL_TIMEOUT_S = 30;

// MQTT topics.
static const char* TOPIC_CMD = "arcgenerator/cmd";
static const char* TOPIC_STATUS = "arcgenerator/status";
static const char* TOPIC_DEBUG = "arcgenerator/debug";
static const char* TOPIC_SLIDER_MANUAL_STEPS = "arcgenerator/slider/manual_steps";
static const char* TOPIC_SLIDER_TEST_STEPS = "arcgenerator/slider/test_steps";
static const char* TOPIC_SLIDER_TEST_US = "arcgenerator/slider/test_us";
static const char* TOPIC_SLIDER_TEST_PAUSE_MS = "arcgenerator/slider/test_pause_ms";
