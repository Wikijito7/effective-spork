#include "DHT.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

const int DHT_SENSOR_PIN = D7;
const int DHT_SENSOR_TYPE = DHT22;
const int SCREEN_CLOCK_PIN = SCL;
const int SCREEN_DATA_PIN = SDA;

const char* WIFI_SSID = "REPLACE_WITH_YOUR_SSID";
const char* WIFI_PASSWORD = "REPLACE_WITH_YOUR_PASSWORD";
const char* GET_DATA_SERVER = "https://REPLACE_WITH_YOUR_SERVER/api/sensor/last";
const char* POST_DATA_SERVER = "https://REPLACE_WITH_YOUR_SERVER/api/sensor/simple";
const char* SERVER_TOKEN = "REPLACE_WITH_TOKEN";
const char* SENSOR_NAME = "Master Sensor";

const long SLEEP_TIMER = 20000;  // in millis
long lastUpdate = 0;

DHT dht(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
HTTPClient https;
U8G2_SSD1309_128X64_NONAME0_F_SW_I2C u8g2(U8G2_R0, D5, D4);

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
    drawDataUploadIcon();
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

void updateWifiStatusIcon() {
  if (WiFi.status() == WL_CONNECTED) {
    u8g2.setFont(u8g2_font_open_iconic_www_1x_t);
    u8g2.drawGlyph(112, 10, 72);
    u8g2.sendBuffer();
  }
}

void drawDataUploadIcon() {
  u8g2.setFont(u8g2_font_open_iconic_www_1x_t);
  u8g2.drawGlyph(96, 10, 67);
  u8g2.sendBuffer();
}

void drawDataDownloadIcon() {
  drawDataDownloadIcon();
  u8g2.setFont(u8g2_font_open_iconic_www_1x_t);
  u8g2.drawGlyph(96, 10, 76);
  u8g2.sendBuffer();
}

String getSatelliteDataFromServer() {
  String response = "";
  if (https.begin(GET_DATA_SERVER)) {
    https.addHeader("Content-Type", "application/json");
    String tokenKey = String("Bearer ") + SERVER_TOKEN;
    https.addHeader("Authorization", tokenKey);
    int result = https.GET();

    if (result == 200) {
      response = https.getString();
    }
    https.end();
  }
  return response;
}

void displayRemoteDataOnScreen(String data) {
  JsonDocument doc;
  deserializeJson(doc, data);

  JsonArray deserializedData = doc["sensors"];

  for (JsonDocument item : deserializedData) {
    displayChildData(item);
  }
}

void deserializeLocalData(String data) {
  JsonDocument doc;
  deserializeJson(doc, data);
  displayChildData(doc);
}

void displayChildData(JsonDocument item) {
  String name = item["name"];
  float temp = item["temp"];
  float hum = item["hum"];
  displayCurrentDataOnScreen(name, temp, hum);
}

void displayCurrentDataOnScreen(String name, float temp, float hum) {
  String formattedTemp = String(temp, 1) + " C";
  String formattedHum = String(hum, 0) + " %";

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, name.c_str());
  u8g2.drawStr(0, 30, formattedTemp.c_str());
  u8g2.drawStr(0, 50, formattedHum.c_str());
  updateWifiStatusIcon();
  u8g2.sendBuffer();
  delay(4000);
}

void runConnectedWorkflow(String sensorData) {
  sendRequest(sensorData);
  String remoteData = getSatelliteDataFromServer();
  displayRemoteDataOnScreen(remoteData);
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  WiFi.mode(WIFI_STA);
  u8g2.begin();

  // We forcelly disconnect wifi, just in case it was connected from a previous session.
  WiFi.disconnect();
  connectToWifi();
  updateWifiStatusIcon();
}

void loop() {
  long currentTime = millis();
  if (currentTime - lastUpdate > SLEEP_TIMER || lastUpdate == 0) { // 20 secs has passed or first loop
    lastUpdate = millis();
    String sensorData = readSensorValues();
    if (WiFi.status() == WL_CONNECTED) {
      runConnectedWorkflow(sensorData);
    } else {
      deserializeLocalData(sensorData);
    }
  }

  delay(1000);
}
