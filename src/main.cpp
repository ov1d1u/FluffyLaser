#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <ArduinoOTA.h>
#include <Defines.h>
#include <LaserMotor.h>
#if defined(ESP32)
#include <FluffyLaserBT.h>
#endif
#include <FluffyLaserMQTT.h>
#include <LaserSettings.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "FileReader.h"

WiFiClient wifiClient;
LaserSettings laserSettings;
LaserMotor laserMotor(LED_PIN, MOTOR_X, MOTOR_Y);
#if defined(ESP32)
FluffyLaserBT fluffyLaser(wifiClient, laserMotor, laserSettings);
#else
FluffyLaserMQTT fluffyLaser(wifiClient, laserMotor, laserSettings);
#endif

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
  configuration.static_ip = json["static_ip"].isNull() ? "" : json["static_ip"].as<String>();
  configuration.gateway = json["gateway"].isNull() ? "" : json["gateway"].as<String>();
  configuration.subnet = json["subnet"].isNull() ? "" : json["subnet"].as<String>();
  configuration.primary_dns = json["primary_dns"].isNull() ? "" : json["primary_dns"].as<String>();
  configuration.secondary_dns = json["secondary_dns"].isNull() ? "" : json["secondary_dns"].as<String>();
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
    Serial.print("Start looking for networks...");

    int n = WiFi.scanNetworks();
    Serial.println(" done.");
    if (n == 0) {
        Serial.println("ERROR: No networks found.");
    } else {
      Serial.print(n);
      Serial.println(" networks found:");
      for (int i = 0; i < n; ++i) {
        // Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
        delay(10);
      }
    }
    Serial.println("");
    
    if (!configuration.static_ip.isEmpty()) {
      IPAddress staticIP;
      IPAddress gateway;
      IPAddress subnet;
      IPAddress primaryDNS;
      IPAddress secondaryDNS;
      Serial.print("Setting static IP address: "); Serial.println(configuration.static_ip.isEmpty());
      staticIP.fromString(configuration.static_ip);
      gateway.fromString(configuration.gateway);
      subnet.fromString(configuration.subnet);
      primaryDNS.fromString(configuration.primary_dns);
      secondaryDNS.fromString(configuration.secondary_dns);
      WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS);
    }
    
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

  WiFi.mode(WIFI_STA);
  WiFi.macAddress(mac);
  WiFi.hostname("fluffy-laser-" + String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX));

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