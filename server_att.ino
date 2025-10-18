#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <map>
#include <ArduinoJson.h>

// ---------------- WIFI ----------------
const char* WIFI_SSID = "Zoo_Studio_2.4";
const char* WIFI_PASS = "Trh@1234";
const char* SERVER_URL = "http://192.168.1.100:3000/api/presence";  // Update with your server IP

// ---------------- ESP DEVICE ID ----------------
const char* ESP_ID = "ESP32_001";  // Unique identifier for this ESP device

// ---------------- BLE ----------------
#define SERVICE_UUID32 0xD7E1A3F4
int scanTime = 5; // seconds

std::map<String, bool> devicePresence;
std::map<String, unsigned long> lastSeenTime;
const unsigned long TIMEOUT_MS = 15000; // 15 seconds

BLEScan* pBLEScan;

// ---------------- SERVER LOG ----------------
void logToServer(const String& token, const String& action) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ WiFi disconnected. Cannot log.");
        return;
    }

    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);
    
    // Create JSON payload according to BRD specification
    StaticJsonDocument<256> doc;
    doc["token"] = token;
    doc["esp_id"] = ESP_ID;
    doc["action"] = action;
    doc["timestamp"] = millis();  // In production, use NTP time for ISO_8601 format
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    int code = http.POST(jsonPayload);
    
    if (code > 0) {
        Serial.printf("✅ Logged %s (%s): %d\n", token.c_str(), action.c_str(), code);
        if (code == 200 || code == 201) {
            String response = http.getString();
            Serial.printf("   Response: %s\n", response.c_str());
        }
    } else {
        Serial.printf("❌ Log error %s (%s): %s\n", token.c_str(), action.c_str(), http.errorToString(code).c_str());
    }
    
    http.end();
}

// ---------------- SCAN CALLBACK ----------------
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        if (!advertisedDevice.haveServiceData()) return;

        String data = advertisedDevice.getServiceData(); 
        BLEUUID sUUID = advertisedDevice.getServiceDataUUID();

        if (!sUUID.equals(BLEUUID((uint32_t)SERVICE_UUID32))) return;

        // Extract readable token
        String token = "";
        for (size_t i = 0; i < data.length(); i++) {
            char c = data[i];
            if (c >= 32 && c <= 126) token += c;
        }
        if (token.isEmpty()) return;

        String mac = advertisedDevice.getAddress().toString().c_str();
        unsigned long now = millis();
        lastSeenTime[token] = now;

        if (!devicePresence[token]) {
            Serial.printf("👣 Walk-IN: %s [%s]\n", token.c_str(), mac.c_str());
            logToServer(token, "IN");
            devicePresence[token] = true;
        }
    }
};

// ---------------- SETUP ----------------
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n===== ESP32 BLE Attendance (Server Integration) =====");
    Serial.printf("ESP Device ID: %s\n", ESP_ID);

    // Wi-Fi connect
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting Wi-Fi");
    for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
        Serial.printf("\n✅ Wi-Fi connected! IP: %s\n", WiFi.localIP().toString().c_str());
    else
        Serial.println("\n❌ Wi-Fi failed, continuing offline");

    // BLE init
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(80);
    Serial.println("✅ BLE ready\n");
}

// ---------------- LOOP ----------------
void loop() {
    Serial.println("🔍 Scanning...");
    pBLEScan->start(scanTime, false);
    pBLEScan->clearResults();

    unsigned long now = millis();
    for (auto& it : devicePresence) {
        String token = it.first;
        bool present = it.second;
        if (present && now - lastSeenTime[token] > TIMEOUT_MS) {
            Serial.printf("🚪 Walk-OUT: %s (timeout %lu ms)\n", token.c_str(), now - lastSeenTime[token]);
            logToServer(token, "OUT");
            devicePresence[token] = false;
        }
    }
    delay(2000);
}
