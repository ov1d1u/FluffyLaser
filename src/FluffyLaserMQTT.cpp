#include <ESPTrueRandom.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "FluffyLaserMQTT.h"
#include "FileReader.h"

const char move_topic[] = "move";
const char laser_topic[] = "light";
const char limits_topic[] = "limits";
const char run_program_topic[] = "program/run";
const char play_program_topic[] = "program/play";
const char stop_topic[] = "stop";

const char power_topic[] = "power";
const char status_topic[] = "status";
const char reboot_topic[] = "reboot";

String mqttServer;
uint16_t mqttPort;
String mqttUser;
String mqttPass;

FluffyLaserMQTT::FluffyLaserMQTT(WiFiClient &client, LaserMotor &_laserMotor, LaserSettings &_laserSettings)
    : FluffyLaser(_laserMotor, _laserSettings)
{
    mqttClient.setClient(client);
    mqttClient.setCallback(
        [&](char *topic, uint8_t *payload, unsigned int length) {
            if (mqttClient.isRetained()) {
                return;
            }
            
            mqttCallback(topic, payload, length);
        }
    );
}

FluffyLaserMQTT::~FluffyLaserMQTT() {}

char* FluffyLaserMQTT::getTopicName(const char *topic) {
    char *fullTopic = new char[strlen(topic) + strlen(getClientId().c_str()) + 2];
    strcpy(fullTopic, getClientId().c_str());
    strcat(fullTopic, "/");
    strcat(fullTopic, topic);

    return fullTopic;
}

void FluffyLaserMQTT::loop() {
    if (!mqttClient.connected()) {
        _connect();
    } else {
        mqttClient.loop();
    }

    FluffyLaser::loop();
}

void FluffyLaserMQTT::setup(Configuration configuration, uint8_t mac[6]) {
    FluffyLaser::setup(configuration, mac);

    Serial.print("Configuring MQTT... ");
    if (!connect(configuration.mqtt_server, configuration.mqtt_port, configuration.mqtt_user, configuration.mqtt_password)) {
        Serial.println("fail.");
        return;
    }
    Serial.println("done.");
}

bool FluffyLaserMQTT::connect(String server, uint16_t port, String user, String pass) {
    mqttServer = server;
    mqttPort = port;
    mqttUser = user;
    mqttPass = pass;

    mqttClient.setServer(mqttServer.c_str(), mqttPort);

    return _connect();
}

bool FluffyLaserMQTT::_connect() {
    bool success = mqttClient.connect(getClientId().c_str(), mqttUser.c_str(), mqttPass.c_str());
    if (success) {
        Serial.println("Connected to MQTT server");
        Serial.print("Subscribing to topic: "); Serial.println(getTopicName("#"));
        mqttClient.subscribe(getTopicName("#"));
    }

    return success;
}

void FluffyLaserMQTT::mqttCallback(char *topic, byte *data, unsigned int length) {
    char payload[length];
    strncpy(payload, (char*)data, length);
    payload[length] = '\0';
    Serial.print("Got MQTT message of length "); Serial.print(length); Serial.println(": ");
    Serial.println(payload);

    if (strcmp(topic, getTopicName(move_topic)) == 0) {
        mqttMotorMove(payload, length);
    } else if (strcmp(topic, getTopicName(laser_topic)) == 0) {
        mqttLaserControl(payload, length);
    } else if (strcmp(topic, getTopicName(limits_topic)) == 0) {
        mqttLimitsControl(payload, length);
    } else if (strcmp(topic, getTopicName(run_program_topic)) == 0) {
        mqttStartProgram(payload, length);
    } else if (strcmp(topic, getTopicName(play_program_topic)) == 0) {
        mqttPlayProgram(payload, length);
    } else if (strcmp(topic, getTopicName(stop_topic)) == 0) {
        mqttCallStop(payload, length);
    } else if (strcmp(topic, getTopicName(power_topic)) == 0) {
        mqttSetPower(payload, length);
    } else if (strcmp(topic, getTopicName(reboot_topic)) == 0) {
        ESP.restart();
    }
}

void FluffyLaserMQTT::mqttMotorMove(char *payload, unsigned int length) {
    int posX, posY;
    double speed;
    sscanf(payload, "%d,%d,%lf", &posX, &posY, &speed);
    Serial.print("motorMove: "); Serial.print("  x: "); Serial.print(posX); Serial.print("  y:"); Serial.print(posY); Serial.print("  speed:"); Serial.println(speed);
    motorMove(posX, posY, speed);
}

void FluffyLaserMQTT::mqttLaserControl(char *payload, unsigned int length) {
    int power;
    sscanf(payload, "%d", &power);
    Serial.print("laserControl: "); Serial.println(power);
    setLaserPower(power);
}

void FluffyLaserMQTT::mqttLimitsControl(char *payload, unsigned int length) {
    int minX, maxX, minY, maxY;
    sscanf(payload, "%d,%d,%d,%d", &minX, &maxX, &minY, &maxY);
    limitsControl(minX, maxX, minY, maxY);
}

/*void FluffyLaserMQTT::setProgram(char *payload, unsigned int length) {
    char accumulator[7] = {'\0'};

    int currentMove = 0;
    movement moves[MAX_MOVES];

    int progNum = -1;
    int moveX = -1;
    int moveY = -1;
    double moveSpeed = -1;

    for (unsigned int i = 0; i < length; i++) {
        char c = payload[i];

        if (currentMove >= MAX_MOVES) {
            return;
        } 

        if (c == ',' || i == length - 1) { 
            if (progNum == -1) {
                long num = strtol(accumulator, nullptr, 10);
                progNum = num;
                Serial.println("Prog: "); Serial.print(progNum); Serial.println(": "); 
            } else if (moveX == -1) {
                long num = strtol(accumulator, nullptr, 10);
                moveX = num;
            } else if (moveY == -1) {
                long num = strtol(accumulator, nullptr, 10);
                moveY = num;
            } else if (moveSpeed == -1) {
                double num = strtod(accumulator, nullptr);
                moveSpeed = num;
            }

            if (moveX != -1 && moveY != -1 && moveSpeed != -1) {
                movement move;
                move.point.x = moveX;
                move.point.y = moveY;
                move.speed = moveSpeed;
                moves[currentMove] = move;
                
                Serial.print("  "); Serial.print(currentMove); Serial.print(": "); Serial.print(move.point.x); Serial.print(","); Serial.print(move.point.y); Serial.print(","); Serial.print(move.speed); 

                currentMove += 1;
                moveX = -1;
                moveY = -1;
                moveSpeed = -1;
            }

            memset(&accumulator[0], '\0', sizeof(accumulator));
        } else {
            long cl = strlen(accumulator);  // Current Length
            accumulator[cl] = c;
        }
    }

    if (progNum > 0 && progNum <= 9) {
        storedProgram program;
        memcpy(program.moves, moves, sizeof(program.moves));
        laserSettings->storeProgram(progNum, program);
        Serial.print("setProgram: "); Serial.println(progNum);
    }
}*/

void FluffyLaserMQTT::mqttPlayProgram(char *payload, unsigned int length) {
    playProgram(payload, length);
}

void FluffyLaserMQTT::mqttStartProgram(char *payload, unsigned int length) {
    int progNum;
    unsigned long progDuration = MAX_RUN_TIME;
    
    sscanf(payload, "%d,%lu", &progNum, &progDuration);
    startProgram(progNum, progDuration);
}

void FluffyLaserMQTT::mqttSetPower(char *payload, unsigned int length) {
    int onoff;
    sscanf(payload, "%d", &onoff);
    power(onoff);
}

void FluffyLaserMQTT::mqttCallStop(char *payload, unsigned int length) {
    stop();
}

void FluffyLaserMQTT::power(bool power) {
    FluffyLaser::power(power);
    mqttClient.publish(getTopicName(status_topic), power ? "1" : "0");
}