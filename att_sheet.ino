#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <map>

// ---------------- WIFI ----------------
const char* WIFI_SSID = "Zoo_Studio_2.4";
const char* WIFI_PASS = "Trh@1234";
const char* GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbyzkxCLyA7MfOrPnXirxHPIT7XOWx8LQJG_Bade2zxOSSZJ--0eePnHCIMRhP2zbz_V/exec";

// ---------------- BLE ----------------
#define SERVICE_UUID32 0xD7E1A3F4
int scanTime = 5; // seconds

std::map<String, bool> devicePresence;
std::map<String, unsigned long> lastSeenTime;
const unsigned long TIMEOUT_MS = 15000; // 15 seconds

BLEScan* pBLEScan;

// ---------------- GOOGLE SHEETS LOG ----------------
void logToGoogleSheet(const String& token, const String& action) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ WiFi disconnected. Cannot log.");
        return;
    }

    HTTPClient http;
    String url = String(GOOGLE_SCRIPT_URL) + "?device=" + token + "&action=" + action;
    http.begin(url);
    http.setTimeout(5000);
    int code = http.GET();
    if (code > 0)
        Serial.printf("✅ Logged %s (%s): %d\n", token.c_str(), action.c_str(), code);
    else
        Serial.printf("❌ Log error %s (%s)\n", token.c_str(), action.c_str());
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
            logToGoogleSheet(token, "IN");
            devicePresence[token] = true;
        }
    }
};

// ---------------- SETUP ----------------
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n===== ESP32 BLE Attendance (Classic BLE, 32-bit UUID) =====");

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
            logToGoogleSheet(token, "OUT");
            devicePresence[token] = false;
        }
    }
    delay(2000);
}
