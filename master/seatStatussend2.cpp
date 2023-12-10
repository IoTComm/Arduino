#include "seatStatusSender.h"

const char *ssid = "SangWon";
const char *password = "11111111";
const char *server = "144.24.90.181";
const String resourcePath = "/iot-comm/ws/fallback/seats";

void setupWiFi() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

String sendSeatStatus(String &payload) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Construct the full URL
    String url = "http://" + String(server) + resourcePath;
    WiFiClient wifiClient;

    // Start connection with WiFi client
    http.begin(wifiClient, url);

    // Set HTTP method to PUT
    http.addHeader("Content-Type", "application/json"); 
    int httpCode = http.sendRequest("PUT", payload);

    if (httpCode > 0) {
      Serial.printf("[HTTP] PUT... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        String responsePayload = http.getString();
        Serial.println(responsePayload);
      }
    } else {
      Serial.printf("[HTTP] PUT... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    // End the connection
    http.end();
  }

  return responsePayload;  //서버로부터 받아온 변경된 좌석 상태
}
