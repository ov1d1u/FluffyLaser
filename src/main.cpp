#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <ArduinoOTA.h>
#include <Defines.h>
#include <LaserMotor.h>
#include <FluffyLaserMQTT.h>
#include <LaserSettings.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "FileReader.h"

WiFiClient wifiClient;
LaserSettings laserSettings;
LaserMotor laserMotor(LED_PIN, MOTOR_X, MOTOR_Y);
FluffyLaserMQTT fluffyLaser(wifiClient, laserMotor, laserSettings);

bool isConfigured = false;
Configuration configuration;
uint8_t mac[6];

bool loadConfiguration() {
  LittleFS.begin();
  // Do some checks...
  if (!LittleFS.exists("/config.json")) {
    Serial.println("CONFIGURATION FILE DOES NOT EXIST, ABORTING.");
    LittleFS.end();
    return false;
  }
  
  int length;
  std::unique_ptr<char[]> buf;
  FileReader configReader = FileReader("/config.json");
  configReader.read(&buf, &length);

  // Parse file
  DynamicJsonDocument doc(512);
  DeserializationError jsonError = deserializeJson(doc, buf.get());
  if (jsonError) {
    Serial.println("FAILED TO PARSE CONFIGURATION FILE, ABORTING.");
    LittleFS.end();
    return false;
  }

  // Load config values from files
  JsonObject json = doc.as<JsonObject>();
  configuration.ssid = json["wifi_name"].as<String>();
  configuration.password = json["wifi_pass"].as<String>();
  configuration.mqtt_server = json["mqtt_server"].as<String>();
  configuration.mqtt_port = json["mqtt_port"];
  configuration.mqtt_user = json["mqtt_user"].as<String>();
  configuration.mqtt_password = json["mqtt_pass"].as<String>();
  LittleFS.end();

  return true;
}

void ensureWiFiConnection() {
  // Check if WiFi is connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to "); Serial.print(configuration.ssid); Serial.print("... ");
    WiFi.begin(configuration.ssid.c_str(), configuration.password.c_str());
    int ledState = HIGH;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      digitalWrite(LED_PIN, ledState);
      ledState = ledState == HIGH ? LOW : HIGH;
    }
    digitalWrite(LED_PIN, LOW);
    Serial.println("done.");
    Serial.print("Local IP address: "); Serial.println(WiFi.localIP());
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Booting system");

  pinMode(LED_PIN, OUTPUT);

  Serial.println("Configuration details:");
  Serial.print("  LED_PIN: "); Serial.println(LED_PIN);
  Serial.print("  MOTOR_X: "); Serial.println(MOTOR_X);
  Serial.print("  MOTOR_Y: "); Serial.println(MOTOR_Y);

  Serial.print("Loading configuration... ");
  isConfigured = loadConfiguration();
  if (!isConfigured) {
    Serial.println("fail.");
    return;
  }
  Serial.println("done.");

  WiFi.macAddress(mac);
  ensureWiFiConnection();

  Serial.println("Configuring laser system... ");
  fluffyLaser.setup(configuration, mac);
  Serial.println("All done.");

  Serial.print("Configuring OTA... ");
  ArduinoOTA.begin();
  Serial.println("done.");
}

void loop() {
  ensureWiFiConnection();
  fluffyLaser.loop();
  ArduinoOTA.handle();
}