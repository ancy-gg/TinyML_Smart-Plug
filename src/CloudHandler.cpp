#include "CloudHandler.h"
#include "TimeSync.h"

void CloudHandler::begin(const char* apiKey, const char* dbUrl) {
    config.database_url = dbUrl;

    // Database Secret style signer (test mode)
    config.signer.test_mode = true;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("[Cloud] Firebase Initialized with Secret.");
}

void CloudHandler::update(float v, float c, float t, float zcv, float thd, float entropy,
                          const String& state, TimeSync* time) {
    if (millis() - _lastSend < _interval) return;
    _lastSend = millis();

    if (!Firebase.ready()) {
        Serial.println("[Cloud] Firebase not ready yet...");
        return;
    }

    uint64_t epochMs = 0;
    String iso = "";
    if (time) {
        epochMs = time->nowEpochMs();      // 0 until SNTP sync
        iso = time->nowISO8601Ms();        // "" until SNTP sync
    }

    // ----- live_data -----
    FirebaseJson json;
    json.set("voltage", v);
    json.set("current", c);
    json.set("temp", t);
    json.set("zcv", zcv);
    json.set("thd", thd);
    json.set("entropy", entropy);
    json.set("status", state);

    // internet time (ms) + friendly string
    json.set("ts_epoch_ms", (double)epochMs);
    json.set("ts_iso", iso);

    // always-available monotonic time
    json.set("uptime_ms", (int)millis());

    // server-side timestamp (always valid even if device time isn't)
    json.set("server_ts/.sv", "timestamp");

    Serial.print("[Cloud] Sending live_data... ");
    if (Firebase.RTDB.updateNode(&fbdo, "/live_data", &json)) {
        Serial.println("OK");
    } else {
        Serial.print("FAILED. Reason: ");
        Serial.println(fbdo.errorReason());
    }

    // ----- history (push-id) -----
    FirebaseJson hist = json; // reuse same fields
    Serial.print("[Cloud] Appending history... ");
    if (Firebase.RTDB.pushJSON(&fbdo, "/history", &hist)) {
        Serial.println("OK");
    } else {
        Serial.print("FAILED. Reason: ");
        Serial.println(fbdo.errorReason());
    }
}
