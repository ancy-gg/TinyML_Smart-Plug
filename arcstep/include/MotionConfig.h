#pragma once

namespace MotionConfig {
  // BLE name shown in Dabble app
  static constexpr const char* BLE_NAME = "ESP32C3_ArcStepper";

  // 28BYJ-48 + ULN2003 tuning
  static constexpr float JOY_DEADZONE = 1.0f;
  static constexpr float MIN_MANUAL_SPEED = 120.0f;   // steps per second
  static constexpr float MAX_MANUAL_SPEED = 850.0f;   // steps per second
  static constexpr float DEFAULT_ACCEL = 1200.0f;     // steps per second^2

  // One-shot motions
  static constexpr long SMALL_PULSE_STEPS = 48;
  static constexpr long LARGE_PULSE_STEPS = 192;
  static constexpr long BURST_PULSE_STEPS = 32;
  static constexpr uint8_t BURST_COUNT = 8;

  // Repeating motions
  static constexpr long DEFAULT_SWEEP_SPAN = 140;
  static constexpr uint8_t DEFAULT_ARC_STRENGTH = 2;
  static constexpr unsigned long DEFAULT_ARC_INTERVAL_MS = 22;
}
