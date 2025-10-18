#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ---------------- WIFI ----------------
const char* WIFI_SSID = "Mihir";
const char* WIFI_PASS = "Vivek@1970";
const char* SERVER_URL = "http://192.168.1.102:3000/api/presence";  // Your computer's IP address

// ---------------- ESP DEVICE ID ----------------
const char* ESP_ID = "ESP32_MOCK_001";

// Mock tokens for testing
const char* mockTokens[] = {"EMP001", "EMP002", "EMP003"};
const int numTokens = 3;
int currentTokenIndex = 0;

// ---------------- SEND TEST DATA TO SERVER ----------------
void sendMockData(const String& token, const String& action) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ WiFi disconnected. Cannot send.");
        return;
    }

    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);
    
    // Create JSON payload
    StaticJsonDocument<256> doc;
    doc["token"] = token;
    doc["esp_id"] = ESP_ID;
    doc["action"] = action;
    doc["timestamp"] = millis();
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    Serial.println("\n📤 Sending mock data...");
    Serial.println("Payload: " + jsonPayload);
    
    int code = http.POST(jsonPayload);
    
    if (code > 0) {
        Serial.printf("✅ Response Code: %d\n", code);
        String response = http.getString();
        Serial.println("Response: " + response);
    } else {
        Serial.printf("❌ Error: %s\n", http.errorToString(code).c_str());
    }
    
    http.end();
    Serial.println("─────────────────────────────────────");
}

// ---------------- SETUP ----------------
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔═══════════════════════════════════════╗");
    Serial.println("║  ESP32 Mock Server Test               ║");
    Serial.println("╚═══════════════════════════════════════╝\n");
    Serial.printf("ESP Device ID: %s\n", ESP_ID);
    Serial.printf("Server URL: %s\n\n", SERVER_URL);

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to Wi-Fi");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ Wi-Fi Connected!");
        Serial.printf("📡 IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("📶 Signal Strength: %d dBm\n\n", WiFi.RSSI());
        
        Serial.println("🎬 Starting mock data transmission in 3 seconds...\n");
        delay(3000);
    } else {
        Serial.println("❌ Wi-Fi Connection Failed!");
        Serial.println("Please check your SSID and password.");
    }
}

// ---------------- LOOP ----------------
void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ Wi-Fi disconnected. Reconnecting...");
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        delay(5000);
        return;
    }
    
    // Cycle through mock tokens
    String token = mockTokens[currentTokenIndex];
    
    // Alternate between IN and OUT actions
    static bool isIn = true;
    String action = isIn ? "IN" : "OUT";
    
    Serial.printf("🎭 Mock Event #%d: %s - %s\n", 
                  (currentTokenIndex + 1), 
                  token.c_str(), 
                  action.c_str());
    
    sendMockData(token, action);
    
    // Toggle action
    isIn = !isIn;
    
    // Move to next token if we completed both IN and OUT
    if (isIn) {
        currentTokenIndex = (currentTokenIndex + 1) % numTokens;
    }
    
    // Wait before next transmission
    Serial.println("⏳ Waiting 10 seconds before next event...\n");
    delay(10000);  // Send mock data every 10 seconds
}
