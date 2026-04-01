#pragma once

namespace MotionConfig {
  static constexpr const char* BLE_NAME = "ESP32C3_ArcStepper";

  // Stepper tuning kept intentionally slow/stable for 28BYJ-48 on USB power
  static constexpr float MAX_SPEED = 320.0f;        // steps/s
  static constexpr float ACCELERATION = 220.0f;     // steps/s^2
  static constexpr float MANUAL_SPEED = 220.0f;     // steps/s while D-pad is held

  // One-shot arc movement: go out once, then come back once
  static constexpr long ARC_SHOT_STEPS = 96;

  // Small manual nudges on face buttons
  static constexpr long NUDGE_STEPS = 24;

  // Startup hardware confidence check
  static constexpr long STARTUP_WIGGLE_STEPS = 40;
}
