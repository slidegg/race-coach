#include <WiFi.h>
#include <HTTPClient.h>

// TODO: change these
const char* ssid        = "ALHN-B884";
const char* password    = "HHMMQh97qs";

// n8n webhook URL (non-test one if you want it live)
const char* webhookUrl  = "https://n8n.srv1134803.hstgr.cloud/webhook/racebox";

// Basic config
const unsigned long LAP_INTERVAL_MS = 15000;  // send a dummy lap every 15s

unsigned long lastLapMs = 0;
int lapCounter = 1;

void connectWifi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());
}

bool postJson(const String& jsonPayload) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] WiFi not connected, reconnecting...");
    connectWifi();
  }

  HTTPClient http;
  Serial.print("[HTTP] POST ");
  Serial.println(webhookUrl);

  http.begin(webhookUrl);                 // for HTTPS, ESP32's built-in cert store is usually enough
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(jsonPayload);
  Serial.printf("[HTTP] Response code: %d\n", httpCode);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("[HTTP] Response body:");
    Serial.println(response);
  } else {
    Serial.printf("[HTTP] POST failed: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  return httpCode > 0 && httpCode < 400;
}

String buildDummyLapJson() {
  // Just for testing – later we’ll replace these with RaceBox data
  float lapTime     = 85.0 + (rand() % 40) / 100.0f;   // 85.00–85.39s
  float theoretical = 84.8;
  float t3Time      = 18.3;
  int   t3MinKmh    = 52;
  int   tempC       = 26;

  String json = "{";
  json += "\"track\":\"Serres\",";
  json += "\"car\":\"bmw e46\",";
  json += "\"lap\":" + String(lapCounter) + ",";
  json += "\"lap_time_s\":" + String(lapTime, 2) + ",";
  json += "\"theoretical_s\":" + String(theoretical, 2) + ",";
  json += "\"sectors\":[";
  json += "  {\"id\":\"T3\",\"t_s\":" + String(t3Time, 2) + ",\"min_kmh\":" + String(t3MinKmh) + "}";
  json += "],";
  json += "\"env\":{";
  json += "  \"temp_c\":" + String(tempC) + ",";
  json += "  \"dry\":true";
  json += "}";
  json += "}";

  return json;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  connectWifi();
  lastLapMs = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - lastLapMs >= LAP_INTERVAL_MS) {
    lastLapMs = now;

    String payload = buildDummyLapJson();
    Serial.println("[LAP] Sending payload:");
    Serial.println(payload);

    bool ok = postJson(payload);
    if (ok) {
      Serial.println("[LAP] Sent successfully.");
      lapCounter++;
    } else {
      Serial.println("[LAP] Send failed.");
    }
  }

  // Do other stuff here later (RaceBox reading, etc.)
}
