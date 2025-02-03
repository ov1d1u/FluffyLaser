#if defined(ESP32)
#include <ESPTrueRandom.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <NimBLEDevice.h>
#include "FluffyLaserBT.h"
#include "FileReader.h"
#include "FluffyLaserBTCallbacks.h"

#define SERVICE_UUID        "e2bdddf6-7686-431c-a474-000000000001"
#define CHARACTERISTIC_UUID "e2bdddf6-7686-431c-a474-000000000002"

FluffyLaserBT::FluffyLaserBT(WiFiClient &client, LaserMotor &_laserMotor, LaserSettings &_laserSettings)
    : FluffyLaserMQTT(client, _laserMotor, _laserSettings)
{
    callbacks = new FluffyLaserBTCallbacks(this);
}

FluffyLaserBT::~FluffyLaserBT() {}

void FluffyLaserBT::setup(Configuration configuration, uint8_t mac[6]) {
    FluffyLaserMQTT::setup(configuration, mac);

    Serial.print("Initializing Bluetooth with name: "); Serial.println(getClientId().c_str());

    NimBLEDevice::init(getClientId().c_str());
    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService(SERVICE_UUID);

    NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::NOTIFY
    );

    pCharacteristic->setCallbacks(callbacks);

    pService->start();
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    Serial.println("Bluetooth initialized.");
}

void FluffyLaserBT::loop() {
    long now = millis();
    FluffyLaserMQTT::loop();
}
#endif