#include "DHT.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define uS_TO_S_FACTOR 1000000ULL

const int DHT_SENSOR_PIN = D7;
const int DHT_SENSOR_TYPE = DHT22;

const char* WIFI_SSID = "REPLACE_WITH_YOUR_SSID";
const char* WIFI_PASSWORD = "REPLACE_WITH_YOUR_PASSWORD";
const char* POST_DATA_SERVER = "REPLACE_WITH_YOUR_SERVER/api/sensor/simple";
const char* SERVER_TOKEN = "REPLACE_WITH_TOKEN";
const char* SENSOR_NAME = "Outside Sensor";

const int SLEEP_TIMER = 20;  // in seconds

DHT dht(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
HTTPClient https;

void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  long connectionTry = millis();
  while (WiFi.status() != WL_CONNECTED) {
    long now = millis();
    if (now - connectionTry > 10000) {
      Serial.println();
      Serial.println("Cannot connect to given Wifi");
      break;
    }
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.println(WiFi.localIP());
}

String readSensorValues() {
  JsonDocument jsonDoc;
  String serializedJson;
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  jsonDoc["name"] = SENSOR_NAME;

  if (isnan(h) || isnan(t)) {
    char* error = "Failed to read from DHT sensor!";
    Serial.println(error);
    jsonDoc["error"] = error;
  } else {
    jsonDoc["temp"] = t;
    jsonDoc["hum"] = h;
  }
  serializeJson(jsonDoc, serializedJson);
  Serial.println(serializedJson);

  return serializedJson;
}

void sendRequest(String body) {
  if (https.begin(POST_DATA_SERVER)) {
    https.addHeader("Content-Type", "application/json");
    String tokenKey = String("Bearer ") + SERVER_TOKEN;
    https.addHeader("Authorization", tokenKey);
    int result = https.POST(body);

    if (result == 200) {
      Serial.println(https.getString());
    }
    https.end();
  }
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  WiFi.mode(WIFI_STA);
  esp_sleep_enable_timer_wakeup(SLEEP_TIMER * uS_TO_S_FACTOR);

  // We forcelly disconnect wifi, just in case it was connected from a previous session. If it came back from deep sleep, it will always be disconnected.
  WiFi.disconnect();
  connectToWifi();

  String sensorData = readSensorValues();
  sendRequest(sensorData); 

  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {}
