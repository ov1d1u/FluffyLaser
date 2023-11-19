#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <Defines.h>
#include <LaserMotor.h>
#include <FluffyLaser.h>
#include <LaserSettings.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "FileReader.h"

WiFiClient wifiClient;
LaserSettings laserSettings;
LaserMotor laserMotor(LED_PIN, MOTOR_X, MOTOR_Y);
FluffyLaser fluffyLaser(wifiClient, laserMotor, laserSettings);

bool isConfigured = false;
struct {
  String ssid;
  String password;
  String mqtt_server;
  String mqtt_user;
  String mqtt_password;
  int mqtt_port;
} configuration;

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
    WiFi.hostname("Fluffy-Laser");
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

  Serial.print("Configuring motors... ");
  laserMotor.setup();
  Serial.println("done.");

  Serial.print("Loading configuration... ");
  isConfigured = loadConfiguration();
  if (!isConfigured) {
    Serial.println("fail.");
    return;
  }
  Serial.println("done.");

  ensureWiFiConnection();

  Serial.print("Configuring MQTT... ");
  if (!fluffyLaser.connect(configuration.mqtt_server, configuration.mqtt_port, configuration.mqtt_user, configuration.mqtt_password)) {
    Serial.println("fail.");
    return;
  }
  Serial.println("done.");

  Serial.print("Configuring OTA... ");
  ArduinoOTA.begin();
  Serial.println("done.");
}

void loop() {
  ensureWiFiConnection();
  fluffyLaser.loop();
  ArduinoOTA.handle();
}