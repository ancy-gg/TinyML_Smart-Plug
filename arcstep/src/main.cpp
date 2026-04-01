#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#include "Pins.h"
#include "Secrets.h"
#include "MotionConfig.h"

struct Segment {
  int32_t steps = 0;
  int8_t dir = 1;            // +1 forward, -1 backward
  uint32_t stepUs = 800;
  const char* label = "";
};

static const uint8_t HALFSTEP_SEQ[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);
WiFiManager wm;

String topicCmd;
String topicSetStroke;
String topicSetArcUs;
String topicSetNudgeUs;
String topicSetJogUs;
String topicSetNudgeSteps;
String topicStatus;
String topicDebug;
String topicPos;
String topicCfg;

Segment plan[4];
uint8_t planCount = 0;
uint8_t planIndex = 0;
int32_t segmentStepsRemaining = 0;
uint32_t currentStepUs = DEFAULT_ARC_STEP_US;
int8_t currentDir = 1;
bool releaseAfterPlan = true;
bool motorEnabled = false;
uint8_t phaseIndex = 0;
int32_t currentPos = 0;
int32_t homePos = 0;

int32_t arcStrokeSteps = DEFAULT_ARC_STROKE_STEPS;
int32_t nudgeSteps = DEFAULT_NUDGE_STEPS;
uint32_t arcStepUs = DEFAULT_ARC_STEP_US;
uint32_t nudgeStepUs = DEFAULT_NUDGE_STEP_US;
uint32_t homeStepUs = DEFAULT_HOME_STEP_US;
uint32_t jogStepUs = DEFAULT_JOG_STEP_US;

unsigned long lastStepMicros = 0;
unsigned long lastWiFiAttemptMs = 0;
unsigned long lastMqttAttemptMs = 0;
unsigned long lastStatusMs = 0;

void publishDebug(const String& msg) {
  Serial.println(msg);
  if (mqtt.connected()) mqtt.publish(topicDebug.c_str(), msg.c_str(), false);
}

void publishStatus(const String& msg, bool retained = false) {
  Serial.println(msg);
  if (mqtt.connected()) mqtt.publish(topicStatus.c_str(), msg.c_str(), retained);
}

void publishPos() {
  char buf[24];
  snprintf(buf, sizeof(buf), "%ld", static_cast<long>(currentPos));
  if (mqtt.connected()) mqtt.publish(topicPos.c_str(), buf, false);
}

void publishConfig() {
  char buf[128];
  snprintf(buf, sizeof(buf),
           "stroke=%ld,arc_us=%lu,nudge_steps=%ld,nudge_us=%lu,jog_us=%lu,home=%ld,pos=%ld",
           static_cast<long>(arcStrokeSteps),
           static_cast<unsigned long>(arcStepUs),
           static_cast<long>(nudgeSteps),
           static_cast<unsigned long>(nudgeStepUs),
           static_cast<unsigned long>(jogStepUs),
           static_cast<long>(homePos),
           static_cast<long>(currentPos));
  if (mqtt.connected()) mqtt.publish(topicCfg.c_str(), buf, true);
}

void setOutputs(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  digitalWrite(IN1_PIN, a);
  digitalWrite(IN2_PIN, b);
  digitalWrite(IN3_PIN, c);
  digitalWrite(IN4_PIN, d);
}

void applyPhase(uint8_t idx) {
  setOutputs(HALFSTEP_SEQ[idx][0], HALFSTEP_SEQ[idx][1], HALFSTEP_SEQ[idx][2], HALFSTEP_SEQ[idx][3]);
}

void enableMotor() {
  if (!motorEnabled) {
    motorEnabled = true;
    applyPhase(phaseIndex);
  }
}

void releaseMotor() {
  setOutputs(0, 0, 0, 0);
  motorEnabled = false;
}

void stepOnce(int8_t dir) {
  enableMotor();
  phaseIndex = static_cast<uint8_t>((phaseIndex + (dir > 0 ? 1 : 7)) & 0x07);
  applyPhase(phaseIndex);
  currentPos += (dir > 0 ? 1 : -1);
}

void clearPlan(bool releaseMotorNow) {
  planCount = 0;
  planIndex = 0;
  segmentStepsRemaining = 0;
  if (releaseMotorNow) releaseMotor();
}

bool loadPlan(const Segment* segments, uint8_t count, bool releaseAfter, const char* name) {
  if (count == 0 || count > 4) return false;
  for (uint8_t i = 0; i < count; ++i) plan[i] = segments[i];
  planCount = count;
  planIndex = 0;
  segmentStepsRemaining = plan[0].steps;
  currentDir = plan[0].dir;
  currentStepUs = plan[0].stepUs;
  releaseAfterPlan = releaseAfter;
  lastStepMicros = micros();
  publishDebug(String("PLAN ") + name);
  return true;
}

bool startArcForward() {
  Segment segs[2] = {
    {arcStrokeSteps, +1, arcStepUs, "out_fwd"},
    {arcStrokeSteps, -1, arcStepUs, "back_rev"}
  };
  return loadPlan(segs, 2, true, "SHOT_FWD");
}

bool startArcReverse() {
  Segment segs[2] = {
    {arcStrokeSteps, -1, arcStepUs, "out_rev"},
    {arcStrokeSteps, +1, arcStepUs, "back_fwd"}
  };
  return loadPlan(segs, 2, true, "SHOT_REV");
}

bool startNudge(int8_t dir) {
  Segment segs[1] = {
    {nudgeSteps, dir, nudgeStepUs, dir > 0 ? "nudge_fwd" : "nudge_rev"}
  };
  return loadPlan(segs, 1, true, dir > 0 ? "NUDGE_FWD" : "NUDGE_REV");
}

bool startJog(int8_t dir, int32_t steps) {
  Segment segs[1] = {
    {steps, dir, jogStepUs, dir > 0 ? "jog_fwd" : "jog_rev"}
  };
  return loadPlan(segs, 1, true, dir > 0 ? "JOG_FWD" : "JOG_REV");
}

bool startGoHome() {
  int32_t delta = homePos - currentPos;
  if (delta == 0) {
    publishDebug("Already at HOME");
    return true;
  }
  Segment segs[1] = {
    {abs(delta), delta > 0 ? +1 : -1, homeStepUs, "go_home"}
  };
  return loadPlan(segs, 1, true, "GO_HOME");
}

void startupWiggle() {
  publishDebug("Startup wiggle...");
  Segment segs[3] = {
    {STARTUP_WIGGLE_STEPS, +1, STARTUP_STEP_US, "wiggle_fwd"},
    {STARTUP_WIGGLE_STEPS * 2, -1, STARTUP_STEP_US, "wiggle_back"},
    {STARTUP_WIGGLE_STEPS, +1, STARTUP_STEP_US, "wiggle_center"}
  };
  loadPlan(segs, 3, true, "SELFTEST");
}

void updateMotion() {
  if (planCount == 0 || segmentStepsRemaining <= 0) return;

  unsigned long nowUs = micros();
  if (static_cast<uint32_t>(nowUs - lastStepMicros) < currentStepUs) return;
  lastStepMicros = nowUs;

  stepOnce(currentDir);
  segmentStepsRemaining--;

  if (segmentStepsRemaining > 0) return;

  planIndex++;
  if (planIndex >= planCount) {
    publishDebug("PLAN DONE");
    planCount = 0;
    planIndex = 0;
    segmentStepsRemaining = 0;
    publishPos();
    publishConfig();
    if (releaseAfterPlan) releaseMotor();
    return;
  }

  currentDir = plan[planIndex].dir;
  currentStepUs = plan[planIndex].stepUs;
  segmentStepsRemaining = plan[planIndex].steps;
}

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  unsigned long now = millis();
  if (now - lastWiFiAttemptMs < WIFI_RECONNECT_MS) return;
  lastWiFiAttemptMs = now;

  publishDebug("Reconnecting Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.reconnect();
}

void setupWiFiManager() {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(true);
  WiFi.setAutoReconnect(true);

  wm.setConfigPortalTimeout(WM_PORTAL_TIMEOUT_S);
  wm.setConnectTimeout(20);
  wm.setBreakAfterConfig(true);

  publishDebug("Starting WiFiManager...");
  bool ok = wm.autoConnect(WM_AP_NAME, WM_AP_PASS);
  if (ok) {
    publishDebug(String("Wi-Fi connected: ") + WiFi.SSID() + " | IP=" + WiFi.localIP().toString());
  } else {
    publishDebug("WiFiManager portal timeout or connect failed");
  }
}

String makeClientId() {
  uint64_t chipid = ESP.getEfuseMac();
  char buf[32];
  snprintf(buf, sizeof(buf), "arcgen-%04X%08lX",
           static_cast<unsigned>((chipid >> 32) & 0xFFFF),
           static_cast<unsigned long)(chipid & 0xFFFFFFFF));
  return String(buf);
}

void ensureMqtt() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqtt.connected()) return;

  unsigned long now = millis();
  if (now - lastMqttAttemptMs < MQTT_RECONNECT_MS) return;
  lastMqttAttemptMs = now;

  String clientId = makeClientId();
  publishDebug("Connecting MQTT...");

  if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS,
                   topicStatus.c_str(), 0, true, "offline")) {
    mqtt.subscribe(topicCmd.c_str());
    mqtt.subscribe(topicSetStroke.c_str());
    mqtt.subscribe(topicSetArcUs.c_str());
    mqtt.subscribe(topicSetNudgeUs.c_str());
    mqtt.subscribe(topicSetJogUs.c_str());
    mqtt.subscribe(topicSetNudgeSteps.c_str());
    publishStatus("online", true);
    publishDebug("MQTT connected");
    publishConfig();
    publishPos();
  } else {
    publishDebug(String("MQTT failed rc=") + mqtt.state());
  }
}

void handleCommand(const String& cmd) {
  String c = cmd;
  c.trim();
  c.toUpperCase();

  publishDebug(String("CMD ") + c);

  if (c == "SHOT_FWD") {
    startArcForward();
  } else if (c == "SHOT_REV") {
    startArcReverse();
  } else if (c == "NUDGE_FWD") {
    startNudge(+1);
  } else if (c == "NUDGE_REV") {
    startNudge(-1);
  } else if (c == "JOG_FWD") {
    startJog(+1, arcStrokeSteps);
  } else if (c == "JOG_REV") {
    startJog(-1, arcStrokeSteps);
  } else if (c == "STOP") {
    clearPlan(false);
    publishDebug("STOP hold");
  } else if (c == "RELEASE") {
    clearPlan(true);
    publishDebug("RELEASE");
  } else if (c == "SAVE_HOME") {
    homePos = currentPos;
    publishDebug(String("HOME SAVED ") + homePos);
    publishConfig();
  } else if (c == "GO_HOME") {
    startGoHome();
  } else if (c == "SELFTEST") {
    startupWiggle();
  } else {
    publishDebug(String("Unknown cmd: ") + c);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; ++i) msg += static_cast<char>(payload[i]);

  Serial.print("TOPIC: ");
  Serial.print(t);
  Serial.print(" | PAYLOAD: ");
  Serial.println(msg);

  if (t == topicCmd) {
    handleCommand(msg);
    return;
  }

  long v = msg.toInt();
  if (t == topicSetStroke) {
    arcStrokeSteps = constrain(static_cast<int32_t>(v), 20, 1200);
    publishDebug(String("stroke=") + arcStrokeSteps);
    publishConfig();
  } else if (t == topicSetArcUs) {
    arcStepUs = constrain(static_cast<uint32_t>(v), 250, 5000);
    publishDebug(String("arc_us=") + arcStepUs);
    publishConfig();
  } else if (t == topicSetNudgeUs) {
    nudgeStepUs = constrain(static_cast<uint32_t>(v), 250, 5000);
    publishDebug(String("nudge_us=") + nudgeStepUs);
    publishConfig();
  } else if (t == topicSetJogUs) {
    jogStepUs = constrain(static_cast<uint32_t>(v), 250, 5000);
    homeStepUs = jogStepUs;
    publishDebug(String("jog_us=") + jogStepUs);
    publishConfig();
  } else if (t == topicSetNudgeSteps) {
    nudgeSteps = constrain(static_cast<int32_t>(v), 1, 500);
    publishDebug(String("nudge_steps=") + nudgeSteps);
    publishConfig();
  }
}

void initTopics() {
  topicCmd          = String(MQTT_BASE) + "/cmd";
  topicSetStroke    = String(MQTT_BASE) + "/set/stroke";
  topicSetArcUs     = String(MQTT_BASE) + "/set/arc_us";
  topicSetNudgeUs   = String(MQTT_BASE) + "/set/nudge_us";
  topicSetJogUs     = String(MQTT_BASE) + "/set/jog_us";
  topicSetNudgeSteps= String(MQTT_BASE) + "/set/nudge_steps";
  topicStatus       = String(MQTT_BASE) + "/status";
  topicDebug        = String(MQTT_BASE) + "/debug";
  topicPos          = String(MQTT_BASE) + "/pos";
  topicCfg          = String(MQTT_BASE) + "/cfg";
}

void setup() {
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  releaseMotor();

  Serial.begin(115200);
  delay(500);

  initTopics();

  Serial.println();
  Serial.println("=== ESP32-C3 MQTT Arc Generator ===");
  Serial.println("Topics:");
  Serial.println(topicCmd);
  Serial.println(topicSetStroke);
  Serial.println(topicSetArcUs);
  Serial.println(topicSetNudgeUs);
  Serial.println(topicSetJogUs);
  Serial.println(topicSetNudgeSteps);
  Serial.println(topicStatus);
  Serial.println(topicDebug);
  Serial.println(topicPos);
  Serial.println(topicCfg);
  Serial.println("===================================");
  Serial.println("WiFiManager AP: ArcGenerator-Setup");
  Serial.println("Portal password: 12345678");
  Serial.println("If no saved Wi-Fi works, connect to the AP and set your Wi-Fi.");
  Serial.println();
  secureClient.setInsecure();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(512);

  startupWiggle();
  setupWiFiManager();
}

void loop() {
  ensureWiFi();
  ensureMqtt();
  mqtt.loop();
  updateMotion();

  unsigned long now = millis();
  if (mqtt.connected() && now - lastStatusMs >= STATUS_PUBLISH_MS) {
    lastStatusMs = now;
    publishPos();
  }
}
