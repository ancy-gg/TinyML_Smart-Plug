#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>
#include <AccelStepper.h>
#include <Arduino.h>

#include "Pins.h"
#include "MotionConfig.h"

// 28BYJ-48 + ULN2003 usually works in HALF4WIRE with IN1, IN3, IN2, IN4 order
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
  MODE_ARC_ONESHOT
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
long arcOrigin = 0;
long arcPeak = 0;
bool arcReturnPhase = false;

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
  arcReturnPhase = false;

  if (releaseMotor) {
    disableMotorOutputs();
  } else {
    enableMotorOutputs();
  }

  Serial.println("STOP");
}

void goRelative(long steps) {
  enableMotorOutputs();
  mode = MODE_TARGET;
  stepper.moveTo(stepper.currentPosition() + steps);
}

void goHome() {
  enableMotorOutputs();
  mode = MODE_TARGET;
  stepper.moveTo(homePosition);
  Serial.print("GO HOME -> ");
  Serial.println(homePosition);
}

void startArcShot(long strokeSteps) {
  enableMotorOutputs();
  arcOrigin = stepper.currentPosition();
  arcPeak = arcOrigin + strokeSteps;
  arcReturnPhase = false;
  mode = MODE_ARC_ONESHOT;
  stepper.moveTo(arcPeak);

  Serial.print("ARC SHOT -> origin: ");
  Serial.print(arcOrigin);
  Serial.print(", peak: ");
  Serial.println(arcPeak);
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
  Serial.println("=== Dabble Arc Generator Control ===");
  Serial.println("Use GamePad in DIGITAL MODE");
  Serial.println("Right (hold)  : manual CW motion");
  Serial.println("Left (hold)   : manual CCW motion");
  Serial.println("Up            : one arc shot forward then back");
  Serial.println("Down          : one arc shot backward then forward");
  Serial.println("Triangle      : small forward nudge");
  Serial.println("Circle        : small backward nudge");
  Serial.println("Square        : stop and hold");
  Serial.println("Cross         : stop and release motor");
  Serial.println("Start         : save current position as home");
  Serial.println("Select        : go to saved home");
  Serial.println("====================================");
  Serial.println();
}

void startupWiggle() {
  Serial.println("Startup wiggle test...");
  enableMotorOutputs();

  stepper.moveTo(MotionConfig::STARTUP_WIGGLE_STEPS);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  delay(180);

  stepper.moveTo(-MotionConfig::STARTUP_WIGGLE_STEPS);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  delay(180);

  stepper.moveTo(0);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }

  disableMotorOutputs();
  Serial.println("Startup wiggle done.");
}

void handleButtonActions() {
  if (rising(curBtn.up, prevBtn.up)) {
    startArcShot(+MotionConfig::ARC_SHOT_STEPS);
  }

  if (rising(curBtn.down, prevBtn.down)) {
    startArcShot(-MotionConfig::ARC_SHOT_STEPS);
  }

  if (rising(curBtn.triangle, prevBtn.triangle)) {
    Serial.println("TRIANGLE -> small forward nudge");
    goRelative(+MotionConfig::NUDGE_STEPS);
  }

  if (rising(curBtn.circle, prevBtn.circle)) {
    Serial.println("CIRCLE -> small backward nudge");
    goRelative(-MotionConfig::NUDGE_STEPS);
  }

  if (rising(curBtn.square, prevBtn.square)) {
    stopMotion(false);
  }

  if (rising(curBtn.cross, prevBtn.cross)) {
    stopMotion(true);
  }

  if (rising(curBtn.start, prevBtn.start)) {
    homePosition = stepper.currentPosition();
    enableMotorOutputs();
    Serial.print("HOME SAVED -> ");
    Serial.println(homePosition);
  }

  if (rising(curBtn.select, prevBtn.select)) {
    goHome();
  }
}

void handleManualDpad() {
  if (curBtn.right && !curBtn.left) {
    enableMotorOutputs();
    mode = MODE_MANUAL;
    stepper.setSpeed(MotionConfig::MANUAL_SPEED);
    stepper.runSpeed();
    return;
  }

  if (curBtn.left && !curBtn.right) {
    enableMotorOutputs();
    mode = MODE_MANUAL;
    stepper.setSpeed(-MotionConfig::MANUAL_SPEED);
    stepper.runSpeed();
    return;
  }

  if (mode == MODE_MANUAL) {
    stepper.setSpeed(0);
    mode = MODE_IDLE;
    disableMotorOutputs();
    Serial.println("MANUAL RELEASE");
  }
}

void updateMotion() {
  switch (mode) {
    case MODE_MANUAL:
      // runSpeed() is already called from handleManualDpad() while button is held
      break;

    case MODE_TARGET:
      stepper.run();
      if (stepper.distanceToGo() == 0) {
        mode = MODE_IDLE;
      }
      break;

    case MODE_ARC_ONESHOT:
      stepper.run();
      if (stepper.distanceToGo() == 0) {
        if (!arcReturnPhase) {
          arcReturnPhase = true;
          stepper.moveTo(arcOrigin);
        } else {
          mode = MODE_IDLE;
          arcReturnPhase = false;
          Serial.println("ARC SHOT DONE");
        }
      }
      break;

    case MODE_IDLE:
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  stepper.setMaxSpeed(MotionConfig::MAX_SPEED);
  stepper.setAcceleration(MotionConfig::ACCELERATION);
  stepper.setCurrentPosition(0);
  stepper.disableOutputs();
  outputsEnabled = false;

  Dabble.begin(MotionConfig::BLE_NAME);

  printHelp();
  startupWiggle();
}

void loop() {
  Dabble.processInput();

  readButtons();
  handleButtonActions();
  handleManualDpad();
  updateMotion();

  prevBtn = curBtn;
}
