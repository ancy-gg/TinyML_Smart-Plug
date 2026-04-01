#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <Update.h>
#include <ArduinoOTA.h>

#include "Pins.h"
#include "Secrets.h"
#include "MotionConfig.h"

namespace {

constexpr uint8_t HALFSTEP_SEQ[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1},
};

WiFiManager wm;
WiFiClientSecure net;
PubSubClient mqtt(net);
WebServer server(80);

int seqIndex = 0;
long logicalPosition = 0;
long homePosition = 0;

int32_t manualSteps = DEFAULT_MANUAL_STEPS;
int32_t testSteps = DEFAULT_TEST_STEPS;
uint32_t nudgeStepUs = DEFAULT_NUDGE_STEP_US;
uint32_t jogStepUs = DEFAULT_JOG_STEP_US;
uint32_t shotStepUs = DEFAULT_SHOT_STEP_US;
uint32_t testStepUs = DEFAULT_TEST_STEP_US;
uint32_t testPauseMs = DEFAULT_TEST_PAUSE_MS;

bool stopRequested = false;
bool releaseAfterStop = false;
bool motionActive = false;
String motionState = "idle";

bool portalRequested = false;
bool portalRunning = false;
bool portalStartedThisBoot = false;
uint32_t portalOpenedMs = 0;
uint32_t lastMqttReconnectMs = 0;
char deviceName[32] = {0};

String ipToString(const IPAddress& ip) {
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

void applyOutputs(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  digitalWrite(IN1_PIN, a);
  digitalWrite(IN2_PIN, b);
  digitalWrite(IN3_PIN, c);
  digitalWrite(IN4_PIN, d);
}

void releaseMotor() {
  applyOutputs(0, 0, 0, 0);
}

int appDirToMotorDir(int dir) {
  int out = (dir >= 0) ? +1 : -1;
  return INVERT_APPLICATION_DIR ? -out : out;
}

void stepOnceMotor(int motorDir) {
  seqIndex += (motorDir >= 0) ? 1 : -1;
  if (seqIndex < 0) seqIndex = 7;
  if (seqIndex > 7) seqIndex = 0;

  applyOutputs(
    HALFSTEP_SEQ[seqIndex][0],
    HALFSTEP_SEQ[seqIndex][1],
    HALFSTEP_SEQ[seqIndex][2],
    HALFSTEP_SEQ[seqIndex][3]
  );
}

void publishDebug(const String& msg) {
  String line = String("[") + String(millis()) + " ms] " + msg;
  Serial.println(line);
  if (mqtt.connected()) mqtt.publish(TOPIC_DEBUG, line.c_str(), false);
}

void publishDebug(const String& tag, const String& msg) {
  publishDebug(tag + " | " + msg);
}

void publishStatus(const String& state) {
  String s = "{";
  s += "\"state\":\"" + state + "\",";
  s += "\"ip\":\"" + ipToString(WiFi.localIP()) + "\",";
  s += "\"portal\":" + String(portalRunning ? 1 : 0) + ",";
  s += "\"saved\":" + String(wm.getWiFiIsSaved() ? 1 : 0) + ",";
  s += "\"pos\":" + String(logicalPosition) + ",";
  s += "\"home\":" + String(homePosition) + ",";
  s += "\"manual_steps\":" + String(manualSteps) + ",";
  s += "\"test_steps\":" + String(testSteps) + ",";
  s += "\"test_us\":" + String(testStepUs) + ",";
  s += "\"test_pause_ms\":" + String(testPauseMs) + ",";
  s += "\"moving\":" + String(motionActive ? 1 : 0) + ",";
  s += "\"motion\":\"" + motionState + "\"";
  s += "}";
  if (mqtt.connected()) mqtt.publish(TOPIC_STATUS, s.c_str(), true);
}

void pumpServices() {
  if (portalRunning) wm.process();
  server.handleClient();
  ArduinoOTA.handle();
  if (mqtt.connected()) mqtt.loop();
}

void setMotionState(const String& s) {
  motionState = s;
  publishStatus(s);
}

bool shouldAbortMotion() {
  if (stopRequested) {
    publishDebug("STOP", releaseAfterStop ? "Released" : "Stopped");
    motionActive = false;
    if (releaseAfterStop) releaseMotor();
    setMotionState("idle");
    stopRequested = false;
    releaseAfterStop = false;
    return true;
  }
  return false;
}

bool stepBlockingApp(int appDir, uint32_t stepUs, int32_t steps) {
  const int motorDir = appDirToMotorDir(appDir);
  motionActive = true;
  uint32_t lastServiceUs = micros();

  for (int32_t i = 0; i < steps; ++i) {
    if (shouldAbortMotion()) return false;

    stepOnceMotor(motorDir);
    logicalPosition += (appDir >= 0) ? 1 : -1;
    delayMicroseconds(stepUs);

    const uint32_t nowUs = micros();
    if ((uint32_t)(nowUs - lastServiceUs) >= 5000UL) {
      pumpServices();
      if (shouldAbortMotion()) return false;
      lastServiceUs = nowUs;
      yield();
    }
  }

  motionActive = false;
  return true;
}

void clampSettings() {
  if (manualSteps < MIN_STROKE_STEPS) manualSteps = MIN_STROKE_STEPS;
  if (manualSteps > MAX_STROKE_STEPS) manualSteps = MAX_STROKE_STEPS;
  if (testSteps < MIN_STROKE_STEPS) testSteps = MIN_STROKE_STEPS;
  if (testSteps > MAX_STROKE_STEPS) testSteps = MAX_STROKE_STEPS;

  if (nudgeStepUs < MIN_STEP_US) nudgeStepUs = MIN_STEP_US;
  if (nudgeStepUs > MAX_STEP_US) nudgeStepUs = MAX_STEP_US;
  if (jogStepUs < MIN_STEP_US) jogStepUs = MIN_STEP_US;
  if (jogStepUs > MAX_STEP_US) jogStepUs = MAX_STEP_US;
  if (shotStepUs < MIN_STEP_US) shotStepUs = MIN_STEP_US;
  if (shotStepUs > MAX_STEP_US) shotStepUs = MAX_STEP_US;
  if (testStepUs < MIN_STEP_US) testStepUs = MIN_STEP_US;
  if (testStepUs > MAX_STEP_US) testStepUs = MAX_STEP_US;

  if (testPauseMs > 2000) testPauseMs = 2000;
}

void doStartupWiggle() {
  publishDebug("BOOT", "Startup wiggle begin");
  setMotionState("startup");
  stepBlockingApp(+1, STARTUP_STEP_US, STARTUP_WIGGLE_STEPS);
  delay(80);
  stepBlockingApp(-1, STARTUP_STEP_US, STARTUP_WIGGLE_STEPS * 2);
  delay(80);
  stepBlockingApp(+1, STARTUP_STEP_US, STARTUP_WIGGLE_STEPS);
  releaseMotor();
  publishDebug("BOOT", "Startup wiggle end");
  setMotionState("idle");
}

void doSelfTest() {
  publishDebug("TEST", "Self-test wiggle");
  doStartupWiggle();
}

void doMove(const char* tag, int appDir, uint32_t stepUs, int32_t steps) {
  stopRequested = false;
  releaseAfterStop = false;
  publishDebug("MOVE", String(tag));
  setMotionState(tag);
  bool ok = stepBlockingApp(appDir, stepUs, steps);
  if (ok) {
    releaseMotor();
    setMotionState("idle");
  }
}

void doTestArc(int appDir) {
  stopRequested = false;
  releaseAfterStop = false;
  publishDebug("TEST", appDir > 0 ? "Arc forward-back" : "Arc reverse-back");
  setMotionState("test");
  bool ok = stepBlockingApp(appDir, testStepUs, testSteps);
  if (ok && testPauseMs > 0) {
    const uint32_t pauseStart = millis();
    while ((uint32_t)(millis() - pauseStart) < testPauseMs) {
      pumpServices();
      if (shouldAbortMotion()) {
        ok = false;
        break;
      }
      delay(1);
    }
  }
  if (ok) {
    ok = stepBlockingApp(-appDir, testStepUs, testSteps);
  }
  if (ok) {
    releaseMotor();
    setMotionState("idle");
  }
}

void goHome() {
  long delta = homePosition - logicalPosition;
  if (delta == 0) {
    publishDebug("HOME", "Already at home");
    return;
  }
  stopRequested = false;
  releaseAfterStop = false;
  publishDebug("HOME", "Going home");
  setMotionState("go_home");
  int dir = (delta > 0) ? +1 : -1;
  bool ok = stepBlockingApp(dir, jogStepUs, labs(delta));
  if (ok) {
    releaseMotor();
    setMotionState("idle");
  }
}

void requestStop(bool release) {
  stopRequested = true;
  releaseAfterStop = release;
  if (!motionActive) {
    if (release) releaseMotor();
    setMotionState("idle");
    stopRequested = false;
    releaseAfterStop = false;
  }
}

void onPortalTimeout() {
  publishDebug("PORTAL", "Timeout reached");
}

void startPortalWindow(const char* reason) {
  if (portalRunning) {
    publishDebug("PORTAL", String("Already open (") + reason + ")");
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  publishDebug("PORTAL", String("Opened (") + reason + ") SSID: " + CONFIG_AP_NAME);
  wm.startConfigPortal(CONFIG_AP_NAME, CONFIG_AP_PASS);
  portalRunning = true;
  portalOpenedMs = millis();
  publishStatus("portal");
}

void stopPortalWindow(const char* reason) {
  if (!portalRunning) return;
  publishDebug("PORTAL", String("Closed (") + reason + ")");
  wm.stopConfigPortal();
  portalRunning = false;
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
  }
  publishStatus(WiFi.status() == WL_CONNECTED ? "idle" : "offline");
}

void handlePortalLifecycle() {
  if (portalRunning) {
    wm.process();
    if ((uint32_t)(millis() - portalOpenedMs) >= (uint32_t)CONFIG_PORTAL_TIMEOUT_S * 1000UL) {
      stopPortalWindow("timeout");
    }
  }
}

void handleRoot() {
  String html;
  html += "<html><head><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>Arc Generator</title></head><body style='font-family:Arial,sans-serif;padding:20px'>";
  html += "<h2>Arc Generator</h2>";
  html += "<p>IP: " + ipToString(WiFi.localIP()) + "</p>";
  html += "<p>Position: " + String(logicalPosition) + "</p>";
  html += "<p>Home: " + String(homePosition) + "</p>";
  html += "<p>Manual steps: " + String(manualSteps) + "</p>";
  html += "<p>Test steps: " + String(testSteps) + "</p>";
  html += "<p>Test step us: " + String(testStepUs) + "</p>";
  html += "<p>Test pause ms: " + String(testPauseMs) + "</p>";
  html += "<p>Portal: " + String(portalRunning ? "OPEN" : "CLOSED") + "</p>";
  html += "<p><a href='/update'>Open OTA Update</a></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleUpdatePage() {
  String html;
  html += "<html><body style='font-family:Arial,sans-serif;padding:20px'>";
  html += "<h2>ESP32 OTA Upload</h2>";
  html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
  html += "<input type='file' name='update'><br><br>";
  html += "<input type='submit' value='Upload'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleUpdateUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    publishDebug("OTA", "Upload started");
    requestStop(true);
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      publishDebug("OTA", "Upload successful, rebooting");
    } else {
      Update.printError(Serial);
      publishDebug("OTA", "Upload failed");
    }
  }
}

void setupWebOta() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_GET, handleUpdatePage);
  server.on(
    "/update", HTTP_POST,
    []() {
      bool ok = !Update.hasError();
      server.send(200, "text/plain", ok ? "OK, rebooting" : "FAIL");
      delay(250);
      if (ok) ESP.restart();
    },
    handleUpdateUpload
  );
  server.begin();
}

void setupArduinoOta() {
  ArduinoOTA.setHostname(deviceName);
  ArduinoOTA.onStart([]() { publishDebug("OTA", "ArduinoOTA start"); });
  ArduinoOTA.onEnd([]() { publishDebug("OTA", "ArduinoOTA end"); });
  ArduinoOTA.onError([](ota_error_t error) {
    publishDebug("OTA", String("ArduinoOTA error ") + String((int)error));
  });
  ArduinoOTA.begin();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t(topic);
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; ++i) msg += (char)payload[i];
  msg.trim();

  publishDebug("MQTT", String("RX ") + t + " = " + msg);

  if (t == TOPIC_CMD) {
    if (msg == "NUDGE_FWD") doMove("nudge_fwd", +1, nudgeStepUs, manualSteps);
    else if (msg == "NUDGE_REV") doMove("nudge_rev", -1, nudgeStepUs, manualSteps);
    else if (msg == "JOG_FWD") doMove("jog_fwd", +1, jogStepUs, manualSteps);
    else if (msg == "JOG_REV") doMove("jog_rev", -1, jogStepUs, manualSteps);
    else if (msg == "SHOT_FWD") doMove("shot_fwd", +1, shotStepUs, manualSteps);
    else if (msg == "SHOT_REV") doMove("shot_rev", -1, shotStepUs, manualSteps);
    else if (msg == "TEST" || msg == "TEST_FWD") doTestArc(+1);
    else if (msg == "TEST_REV") doTestArc(-1);
    else if (msg == "STOP") requestStop(false);
    else if (msg == "RELEASE") requestStop(true);
    else if (msg == "SAVE_HOME") { homePosition = logicalPosition; publishDebug("HOME", "Saved home position"); setMotionState("idle"); }
    else if (msg == "GO_HOME") goHome();
    else if (msg == "SELFTEST") doSelfTest();
    else if (msg == "OPEN_AP") { portalRequested = true; publishDebug("PORTAL", "Open requested by MQTT"); }
    else if (msg.startsWith("SET_MANUAL_STEPS ")) { manualSteps = msg.substring(17).toInt(); clampSettings(); publishDebug("CFG", String("Manual steps = ") + String(manualSteps)); setMotionState("idle"); }
    else if (msg.startsWith("SET_TEST_STEPS ")) { testSteps = msg.substring(15).toInt(); clampSettings(); publishDebug("CFG", String("Test steps = ") + String(testSteps)); setMotionState("idle"); }
    else if (msg.startsWith("SET_TEST_US ")) { testStepUs = msg.substring(12).toInt(); clampSettings(); publishDebug("CFG", String("Test speed us = ") + String(testStepUs)); setMotionState("idle"); }
    else if (msg.startsWith("SET_TEST_PAUSE_MS ")) { testPauseMs = msg.substring(18).toInt(); clampSettings(); publishDebug("CFG", String("Test pause ms = ") + String(testPauseMs)); setMotionState("idle"); }
    else {
      publishDebug("MQTT", "Unknown command");
    }
    return;
  }

  if (t == TOPIC_SLIDER_MANUAL_STEPS) {
    manualSteps = msg.toInt();
    clampSettings();
    publishDebug("CFG", String("Manual steps = ") + String(manualSteps));
    setMotionState("idle");
  } else if (t == TOPIC_SLIDER_TEST_STEPS) {
    testSteps = msg.toInt();
    clampSettings();
    publishDebug("CFG", String("Test steps = ") + String(testSteps));
    setMotionState("idle");
  } else if (t == TOPIC_SLIDER_TEST_US) {
    testStepUs = msg.toInt();
    clampSettings();
    publishDebug("CFG", String("Test speed us = ") + String(testStepUs));
    setMotionState("idle");
  } else if (t == TOPIC_SLIDER_TEST_PAUSE_MS) {
    testPauseMs = msg.toInt();
    clampSettings();
    publishDebug("CFG", String("Test pause ms = ") + String(testPauseMs));
    setMotionState("idle");
  }
}

void connectMqtt() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(512);

  if (mqtt.connected()) return;
  if (WiFi.status() != WL_CONNECTED) return;

  const uint32_t now = millis();
  if (now - lastMqttReconnectMs < 2000) return;
  lastMqttReconnectMs = now;

  publishDebug("MQTT", "Connecting");
  if (mqtt.connect(deviceName, MQTT_USER, MQTT_PASS)) {
    mqtt.subscribe(TOPIC_CMD);
    mqtt.subscribe(TOPIC_SLIDER_MANUAL_STEPS);
    mqtt.subscribe(TOPIC_SLIDER_TEST_STEPS);
    mqtt.subscribe(TOPIC_SLIDER_TEST_US);
    mqtt.subscribe(TOPIC_SLIDER_TEST_PAUSE_MS);
    publishDebug("MQTT", "Connected");
    publishStatus("idle");
  } else {
    publishDebug("MQTT", String("Connect failed rc=") + String(mqtt.state()));
  }
}

void startStartupPortalWindow() {
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT_S);
  wm.setConfigPortalTimeoutCallback(onPortalTimeout);
  wm.setWiFiAutoReconnect(true);
  wm.setConnectTimeout(8);
  wm.setConnectRetries(1);
  wm.setBreakAfterConfig(false);

  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(true);
  startPortalWindow("boot");

  if (wm.getWiFiIsSaved()) {
    publishDebug("WIFI", String("Saved SSID: ") + wm.getWiFiSSID());
    WiFi.begin();
  } else {
    publishDebug("WIFI", "No saved Wi-Fi");
  }

  portalStartedThisBoot = true;
}

} // namespace

void setup() {
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  releaseMotor();

  Serial.begin(115200);
  delay(300);

  uint64_t chip = ESP.getEfuseMac();
  snprintf(deviceName, sizeof(deviceName), "ArcGen-%04X", (uint16_t)(chip & 0xFFFF));

  net.setInsecure();

  startStartupPortalWindow();
  setupWebOta();
  setupArduinoOta();
  doStartupWiggle();
}

void loop() {
  handlePortalLifecycle();
  server.handleClient();
  ArduinoOTA.handle();

  if (!mqtt.connected()) connectMqtt();
  mqtt.loop();

  if (portalRequested) {
    portalRequested = false;
    startPortalWindow("mqtt_cmd");
  }

  static wl_status_t lastWifi = WL_IDLE_STATUS;
  wl_status_t nowStatus = WiFi.status();
  if (nowStatus != lastWifi) {
    lastWifi = nowStatus;
    if (nowStatus == WL_CONNECTED) {
      publishDebug("WIFI", String("Connected to ") + WiFi.SSID() + " | IP: " + ipToString(WiFi.localIP()));
      publishStatus("idle");
    } else {
      publishDebug("WIFI", String("Status = ") + String((int)nowStatus));
      if (!portalRunning && portalStartedThisBoot) publishStatus("offline");
    }
  }
}
