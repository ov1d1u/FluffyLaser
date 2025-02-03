#include <NimBLEDevice.h>
#include "FluffyLaserBT.h"

#define COMMAND_MOTOR_MOVE   0x01
#define COMMAND_LASER        0x02
#define COMMAND_LIMITS       0x03
#define COMMAND_PROGRAM_RUN  0x04
#define COMMAND_PROGRAM_PLAY 0x05
#define COMMAND_STOP         0x06
#define COMMAND_POWER        0x07
#define COMMAND_REBOOT       0x09

class FluffyLaserBTCallbacks : public NimBLECharacteristicCallbacks {
public:
    FluffyLaserBTCallbacks(FluffyLaserBT* parent) {
        parent_ = parent;
    }

    void onRead(NimBLECharacteristic* pCharacteristic) {
        // Handle read operation here
        // You can use pCharacteristic->setValue() to set the value to be read
    }

    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        Serial.print("Received "); Serial.print(value.length()); Serial.println(" bytes");
        Serial.print("Received data: ");
        for (int i = 0; i < value.length(); i++) {
            Serial.print("0x");
            Serial.print(value[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        int command = value[0];
        if (command == COMMAND_MOTOR_MOVE && value.length() == 4) {
            int x = value[1];
            int y = value[2];
            double speed = value[3];
            parent_->motorMove(x, y, speed);
        } else if (command == COMMAND_LASER && value.length() == 2) {
            int state = value[1];
            parent_->laserControl(state);
        } else if (command == COMMAND_LIMITS && value.length() == 5) {
            int xMin = value[1];
            int xMax = value[2];
            int yMin = value[3];
            int yMax = value[4];
            parent_->limitsControl(xMin, xMax, yMin, yMax);
        } else if (command == COMMAND_PROGRAM_RUN && value.length() == 6) {
            int progNum = value[1];
            unsigned long duration = 0;
            duration |= value[2] << 24;
            duration |= value[3] << 16;
            duration |= value[4] << 8;
            duration |= value[5];
            parent_->startProgram(progNum, duration);
        } else if (command == COMMAND_PROGRAM_PLAY) {
            parent_->playProgram(value.c_str(), value.length());
        } else if (command == COMMAND_STOP) {
            parent_->stop();
        } else if (command == COMMAND_POWER) {
            int state = value[1];
            parent_->power(state);
        } else if (command == COMMAND_REBOOT) {
            ESP.restart();
        }
    }

private:
    FluffyLaserBT* parent_;
};