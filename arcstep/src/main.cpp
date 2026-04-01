#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>
#include <AccelStepper.h>
#include <Arduino.h>

#include "Pins.h"
#include "MotionConfig.h"

// 28BYJ-48 works properly with HALF4WIRE and this pin order: IN1, IN3, IN2, IN4
AccelStepper stepper(
  AccelStepper::HALF4WIRE,
  Pins::STEPPER_IN1,
  Pins::STEPPER_IN3,
  Pins::STEPPER_IN2,
  Pins::STEPPER_IN4
);

enum MotionMode {
  MODE_IDLE,
  MODE_MANUAL,
  MODE_TARGET,
  MODE_SWEEP,
  MODE_ARC,
  MODE_BURST
};

struct ButtonSnapshot {
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool triangle = false;
  bool circle = false;
  bool cross = false;
  bool square = false;
  bool start = false;
  bool select = false;
};

MotionMode mode = MODE_IDLE;
ButtonSnapshot prevBtn;
ButtonSnapshot curBtn;

bool outputsEnabled = false;
long homePosition = 0;

long sweepCenter = 0;
long sweepSpan = MotionConfig::DEFAULT_SWEEP_SPAN;

long arcCenter = 0;
uint8_t arcStrength = MotionConfig::DEFAULT_ARC_STRENGTH;
unsigned long arcKickIntervalMs = MotionConfig::DEFAULT_ARC_INTERVAL_MS;
unsigned long lastArcKickMs = 0;

long burstBasePosition = 0;
uint8_t burstRemaining = 0;
bool burstForward = true;

static float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  if (x <= in_min) return out_min;
  if (x >= in_max) return out_max;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static bool rising(bool now, bool before) {
  return now && !before;
}

void enableMotorOutputs() {
  if (!outputsEnabled) {
    stepper.enableOutputs();
    outputsEnabled = true;
  }
}

void disableMotorOutputs() {
  if (outputsEnabled) {
    stepper.disableOutputs();
    outputsEnabled = false;
  }
}

void stopMotion(bool releaseMotor) {
  stepper.stop();
  stepper.setSpeed(0);
  mode = MODE_IDLE;
  burstRemaining = 0;

  if (releaseMotor) {
    disableMotorOutputs();
  } else {
    enableMotorOutputs();
  }
}

void goRelative(long steps) {
  enableMotorOutputs();
  mode = MODE_TARGET;
  stepper.moveTo(stepper.currentPosition() + steps);
}

void startSweep(long spanSteps) {
  enableMotorOutputs();
  sweepCenter = stepper.currentPosition();
  sweepSpan = spanSteps;
  mode = MODE_SWEEP;
  stepper.moveTo(sweepCenter + sweepSpan);
}

void startArcMode(uint8_t strengthLevel, unsigned long intervalMs) {
  enableMotorOutputs();
  arcCenter = stepper.currentPosition();
  arcStrength = strengthLevel;
  arcKickIntervalMs = intervalMs;
  lastArcKickMs = 0;
  mode = MODE_ARC;
}

void startBurst(uint8_t count, long pulseSteps) {
  enableMotorOutputs();
  burstBasePosition = stepper.currentPosition();
  burstRemaining = count * 2;
  burstForward = true;
  mode = MODE_BURST;
  stepper.moveTo(burstBasePosition + pulseSteps);
}

void readButtons() {
  curBtn.up       = GamePad.isUpPressed();
  curBtn.down     = GamePad.isDownPressed();
  curBtn.left     = GamePad.isLeftPressed();
  curBtn.right    = GamePad.isRightPressed();
  curBtn.triangle = GamePad.isTrianglePressed();
  curBtn.circle   = GamePad.isCirclePressed();
  curBtn.cross    = GamePad.isCrossPressed();
  curBtn.square   = GamePad.isSquarePressed();
  curBtn.start    = GamePad.isStartPressed();
  curBtn.select   = GamePad.isSelectPressed();
}

void printHelp() {
  Serial.println();
  Serial.println("=== Dabble 28BYJ-48 Stepper Control ===");
  Serial.println("Joystick Left/Right : manual continuous control");
  Serial.println("Joystick Up/Down    : scales speed or effect strength");
  Serial.println("Right               : small CW pulse");
  Serial.println("Left                : small CCW pulse");
  Serial.println("Up                  : large CW pulse");
  Serial.println("Down                : large CCW pulse");
  Serial.println("Triangle            : continuous to-and-fro sweep");
  Serial.println("Circle              : continuous arc-like jitter mode");
  Serial.println("Square              : pulse burst train");
  Serial.println("Cross               : stop and release motor");
  Serial.println("Start               : save current position as home");
  Serial.println("Select              : return to saved home");
  Serial.println("=======================================");
  Serial.println();
}

void handleButtonActions() {
  if (rising(curBtn.right, prevBtn.right)) {
    goRelative(+MotionConfig::SMALL_PULSE_STEPS);
  }

  if (rising(curBtn.left, prevBtn.left)) {
    goRelative(-MotionConfig::SMALL_PULSE_STEPS);
  }

  if (rising(curBtn.up, prevBtn.up)) {
    goRelative(+MotionConfig::LARGE_PULSE_STEPS);
  }

  if (rising(curBtn.down, prevBtn.down)) {
    goRelative(-MotionConfig::LARGE_PULSE_STEPS);
  }

  if (rising(curBtn.triangle, prevBtn.triangle)) {
    float y = fabs(GamePad.getYaxisData());
    long span = (long)mapFloat(y, 0.0f, 7.0f, 90.0f, 240.0f);
    startSweep(span);
  }

  if (rising(curBtn.circle, prevBtn.circle)) {
    float y = fabs(GamePad.getYaxisData());
    uint8_t strength = (uint8_t)round(mapFloat(y, 0.0f, 7.0f, 1.0f, 5.0f));
    unsigned long intervalMs = (unsigned long)mapFloat(y, 0.0f, 7.0f, 40.0f, 12.0f);
    startArcMode(strength, intervalMs);
  }

  if (rising(curBtn.square, prevBtn.square)) {
    float y = fabs(GamePad.getYaxisData());
    uint8_t count = (uint8_t)round(mapFloat(y, 0.0f, 7.0f, 4.0f, 12.0f));
    startBurst(count, MotionConfig::BURST_PULSE_STEPS);
  }

  if (rising(curBtn.cross, prevBtn.cross)) {
    stopMotion(true);
  }

  if (rising(curBtn.start, prevBtn.start)) {
    homePosition = stepper.currentPosition();
    enableMotorOutputs();
    Serial.print("Home saved at: ");
    Serial.println(homePosition);
  }

  if (rising(curBtn.select, prevBtn.select)) {
    enableMotorOutputs();
    mode = MODE_TARGET;
    stepper.moveTo(homePosition);
  }
}

void handleManualJoystick() {
  float x = GamePad.getXaxisData();
  float y = fabs(GamePad.getYaxisData());

  if (fabs(x) > MotionConfig::JOY_DEADZONE) {
    enableMotorOutputs();
    mode = MODE_MANUAL;

    float speedScale = mapFloat(y, 0.0f, 7.0f, 0.55f, 1.0f);
    float speedMag = mapFloat(fabs(x), MotionConfig::JOY_DEADZONE, 7.0f,
                              MotionConfig::MIN_MANUAL_SPEED,
                              MotionConfig::MAX_MANUAL_SPEED);
    float signedSpeed = (x > 0.0f ? 1.0f : -1.0f) * speedMag * speedScale;
    stepper.setSpeed(signedSpeed);
  } else {
    if (mode == MODE_MANUAL) {
      stepper.setSpeed(0);
      mode = MODE_IDLE;
      disableMotorOutputs();
    }
  }
}

void updateMotion() {
  switch (mode) {
    case MODE_MANUAL:
      stepper.runSpeed();
      break;

    case MODE_TARGET:
      stepper.run();
      if (stepper.distanceToGo() == 0) {
        mode = MODE_IDLE;
      }
      break;

    case MODE_SWEEP:
      if (stepper.distanceToGo() == 0) {
        long current = stepper.currentPosition();
        if (current >= (sweepCenter + sweepSpan)) {
          stepper.moveTo(sweepCenter - sweepSpan);
        } else {
          stepper.moveTo(sweepCenter + sweepSpan);
        }
      }
      stepper.run();
      break;

    case MODE_ARC:
      if (millis() - lastArcKickMs >= arcKickIntervalMs && stepper.distanceToGo() == 0) {
        lastArcKickMs = millis();
        long kick = random(-18 * arcStrength, 19 * arcStrength);
        if (kick == 0) kick = 1;
        stepper.moveTo(arcCenter + kick);
      }
      stepper.run();
      break;

    case MODE_BURST:
      if (stepper.distanceToGo() == 0) {
        if (burstRemaining == 0) {
          mode = MODE_IDLE;
          break;
        }

        burstRemaining--;
        burstForward = !burstForward;
        long target = burstForward
                        ? burstBasePosition + MotionConfig::BURST_PULSE_STEPS
                        : burstBasePosition - MotionConfig::BURST_PULSE_STEPS;
        stepper.moveTo(target);
      }
      stepper.run();
      break;

    case MODE_IDLE:
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  randomSeed((uint32_t)micros());

  stepper.setMaxSpeed(MotionConfig::MAX_MANUAL_SPEED);
  stepper.setAcceleration(MotionConfig::DEFAULT_ACCEL);
  stepper.setCurrentPosition(0);
  stepper.disableOutputs();
  outputsEnabled = false;

  Dabble.begin(MotionConfig::BLE_NAME);
  printHelp();
}

void loop() {
  Dabble.processInput();

  readButtons();
  handleButtonActions();
  handleManualJoystick();
  updateMotion();

  prevBtn = curBtn;
}
