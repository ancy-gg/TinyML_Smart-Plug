#include "CloudHandler.h"

void CloudHandler::begin(const char* apiKey, const char* dbUrl) {
    config.database_url = dbUrl;
    
    // We use the Database Secret as the signer
    config.signer.test_mode = true; 

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("[Cloud] Firebase Initialized with Secret.");
}

void CloudHandler::update(float v, float c, float t, float zcv, float thd, float entropy, String state) {
    // 1. Check Timer (5 seconds = 5000ms)
    if (millis() - _lastSend < 5000) return;
    _lastSend = millis();

    if (!Firebase.ready()) {
        Serial.println("[Cloud] Firebase not ready yet...");
        return;
    }
    // 3. Prepare JSON
    FirebaseJson json;
    json.set("voltage", v);
    json.set("current", c);
    json.set("temp", t);
    json.set("zcv", zcv);
    json.set("thd", thd);
    json.set("entropy", entropy);
    json.set("status", state);
    
    // Add a timestamp (optional but useful)
    json.set("timestamp", millis());

    // 4. Send
    Serial.print("[Cloud] Sending data... ");
    if (Firebase.RTDB.updateNode(&fbdo, "/live_data", &json)) {
        Serial.println("OK");
    } else {
        Serial.print("Error: ");
        Serial.println(fbdo.errorReason());
    }



    Serial.print("[Cloud] Attempting Send... ");
    if (Firebase.RTDB.updateNode(&fbdo, "/live_data", &json)) {
        Serial.println("SUCCESS!");
    } else {
        Serial.print("FAILED. Reason: ");
        Serial.println(fbdo.errorReason());
    }
}