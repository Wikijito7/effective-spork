#include "DHT.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define uS_TO_S_FACTOR 1000000ULL

const int DHT_SENSOR_PIN = D7;
const int DHT_SENSOR_TYPE = DHT22;

const char* WIFI_SSID = "REPLACE_WITH_YOUR_SSID";
const char* WIFI_PASSWORD = "REPLACE_WITH_YOUR_PASSWORD";
const char* GET_DATA_SERVER = "REPLACE_WITH_YOUR_SERVER/api/sensor/last";
const char* POST_DATA_SERVER = "REPLACE_WITH_YOUR_SERVER/api/sensor";
const char* SERVER_TOKEN = "REPLACE_WITH_TOKEN";
const char* SENSOR_NAME = "Master Sensor";

const int SLEEP_TIMER = 20;  // in seconds

int loopCount = 0;

DHT dht(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
HTTPClient https;

void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.println(WiFi.localIP());
}

char* readSensorValues() {
  JsonDocument jsonDoc;
  char serializedJson[256];
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

    serializeJson(jsonDoc, serializedJson);
    Serial.println(serializedJson);
  }

  return serializedJson;
}

void sendRequest(char* body) {
  if (https.begin(SERVER)) {
    https.addHeader("Content-Type", "application/json");
    String token_key = String("Bearer ") + SERVER_TOKEN;
    https.addHeader("Authorization", token_key);
    int result = https.POST(body);

    if (result == 200) {
      Serial.println(https.getString());
    }
    https.end();
  }
}

char* getSatelliteDataFromServer() {
  if (https.begin()) {

  }
}

void displayDataOnScreen(char* data) {
  JsonDocument doc;
  deserializeJson(doc, json);
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  WiFi.mode(WIFI_STA);

  // We forcelly disconnect wifi, just in case it was connected from a previous session.
  WiFi.disconnect();
  connectToWifi();
}

void loop() {

  if (loopCount >= 5) {
    char* sensorData = readSensorValues();
    sendRequest(sensorData);
  }

  loopCount++;
  sleep(1000);
}
