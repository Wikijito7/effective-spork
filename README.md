# effective-spork
Custom Arduino scripts for ESP32S3.

## Requirements
- ESP32 with network capabilities (preferred wifi).
- DHT22 sensor
- The following libs:
  - Adafruit's DHT
  - WiFi (built in with ESP32 boards)
  - HTTPClient
  - ArduinoJson by Benoit Blanchon, version 7+.
  - u8g2 library for screen.
 
## What's included
On this repo, you'll find mainly two scripts: 
1. Master script, which will read sensor data from a DHT22 sensor, display it on a screen and, if configured, will pull data from a server to display all satellite data on the screen. This script will only pull the last uploaded data from each satellite, with their name and battery percentage.
2. Satellite script, which will read sensor data from a DHT22 sensor and uploaded to a server in order to be pulled by master script or other services.

## Good to use it with
### Server
This server will pull data from each satellite and make it available. You'll be able to make data from each sensor public or keep it private. Server repo: SOON™.

This server will come with a website, where you'll be able to modify the visibility of each sensor and see the historic data of each sensor on a graph. Website repo: SOON™.
