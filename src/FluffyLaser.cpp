#include <ESP8266TrueRandom.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "FluffyLaser.h"
#include "FileReader.h"

const char move_topic[] = "fluffylaser/move";
const char laser_topicl[] = "fluffylaser/light";
const char limits_topic[] = "fluffylaser/limits";
const char run_program_topic[] = "fluffylaser/program/run";
const char play_program_topic[] = "fluffylaser/program/play";
const char stop_topic[] = "fluffylaser/stop";

const char power_topic[] = "fluffylaser/power";
const char status_topic[] = "fluffylaser/status";
const char reboot_topic[] = "fluffylaser/reboot";

String mqttServer;
uint16_t mqttPort;
String mqttUser;
String mqttPass;
long mqttLastReconnectAttempt = 0;

FluffyLaser::FluffyLaser(WiFiClient &client, LaserMotor &_laserMotor, LaserSettings &_laserSettings)
{
    laserMotor = &_laserMotor;
    laserSettings = &_laserSettings;
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

FluffyLaser::~FluffyLaser() {}

void FluffyLaser::loop() {
    if (!mqttClient.connected()) {
        long now = millis();
        if (now - mqttLastReconnectAttempt > 5000) {
            mqttLastReconnectAttempt = now;
            if (_connect()) {
                mqttLastReconnectAttempt = 0;
            }
        }
    } else {
        mqttClient.loop();
    }

    laserMotor->loop();
}

bool FluffyLaser::connect(String server, uint16_t port, String user, String pass) {
    mqttServer = server;
    mqttPort = port;
    mqttUser = user;
    mqttPass = pass;

    mqttClient.setServer(mqttServer.c_str(), mqttPort);

    return _connect();
}

bool FluffyLaser::_connect() {
    bool success = mqttClient.connect(DEVICE_NAME, mqttUser.c_str(), mqttPass.c_str());
    if (success) {
        mqttClient.subscribe(DEVICE_TOPIC);
    }

    return success;
}

void FluffyLaser::mqttCallback(char *topic, byte *data, unsigned int length) {
    char payload[length];
    strncpy(payload, (char*)data, length);
    payload[length] = '\0';
    Serial.print("Got MQTT message of length "); Serial.print(length); Serial.println(": ");
    Serial.println(payload);

    if (strcmp(topic, move_topic) == 0) {
        motorMove(payload, length);
    } else if (strcmp(topic, laser_topicl) == 0) {
        laserControl(payload, length);
    } else if (strcmp(topic, limits_topic) == 0) {
        limitsControl(payload, length);
    } else if (strcmp(topic, run_program_topic) == 0) {
        startProgram(payload, length);
    } else if (strcmp(topic, play_program_topic) == 0) {
        playProgram(payload, length);
    } else if (strcmp(topic, stop_topic) == 0) {
        stop(payload, length);
    } else if (strcmp(topic, power_topic) == 0) {
        power(payload, length);
    } else if (strcmp(topic, reboot_topic) == 0) {
        ESP.restart();
    }
}

void FluffyLaser::motorMove(char *payload, unsigned int length) {
    int posX, posY;
    double speed;
    sscanf(payload, "%d,%d,%lf", &posX, &posY, &speed);
    Serial.print("motorMove: "); Serial.print("  x: "); Serial.print(posX); Serial.print("  y:"); Serial.print(posY); Serial.print("  speed:"); Serial.println(speed);
    movement move;
    move.point.x = posX;
    move.point.y = posY;
    move.speed = speed;
    laserMotor->move(move);
}

void FluffyLaser::laserControl(char *payload, unsigned int length) {
    int power;
    sscanf(payload, "%d", &power);
    Serial.print("laserControl: "); Serial.println(power);
    setLaserPower(power);
}

void FluffyLaser::limitsControl(char *payload, unsigned int length) {
    int minX, maxX, minY, maxY;
    sscanf(payload, "%d,%d,%d,%d", &minX, &maxX, &minY, &maxY);
    Serial.print("limitsControl: "); 
    Serial.print("  x: "); Serial.print(minX); Serial.print(","); Serial.print(maxX);
    Serial.print("  y: "); Serial.print(minY); Serial.print(","); Serial.println(maxY);
    movlimits limits;
    limits.minX = minX;
    limits.maxX = maxX;
    limits.minY = minY;
    limits.maxY = maxY;
    laserSettings->setLimits(limits);
}

/*void FluffyLaser::setProgram(char *payload, unsigned int length) {
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

void FluffyLaser::playProgram(char *payload, unsigned int length) {
    movement moves[MAX_MOVES];
    char accumulator[7] = {'\0'};

    int currentMove = 0;
    int moveX = -1;
    int moveY = -1;
    double moveSpeed = -1;

    for (unsigned int i = 0; i < length; i++) {
        char c = payload[i];

        if (c == ',' || i == length - 1) {
            if (moveX == -1) {
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

    playprogram_t program;
    memcpy(program.moves, moves, sizeof(program.moves));
    setLaserPower(true);
    laserMotor->startProgram(program);
    Serial.println("playProgram");
}

void FluffyLaser::startProgram(char *payload, unsigned int length) {
    int progNum;
    sscanf(payload, "%d", &progNum);
    Serial.print("startProgram: "); Serial.println(progNum);

    runProgram(progNum);
}

void FluffyLaser::runProgram(int progNum) {
    playprogram_t program;
    if (progNum == 0) {
        // Generate program
        movlimits limits = laserSettings->getLimits();
        
        point_t startPoint = { .x = INIT_X, .y = INIT_Y };

        for (int i = 0; i < MAX_MOVES; i++) {
            movement move;
            point_t point;
            // move.speed = ESP8266TrueRandom.random(20, 40) / 10.0;
            point.x = ESP8266TrueRandom.random(limits.minX, limits.maxX);
            point.y = ESP8266TrueRandom.random(limits.minY, limits.maxY);
            move.point = point;
            
            int distance = sqrt(pow(move.point.x - startPoint.x, 2) + pow(move.point.y - startPoint.y, 2) * 1.0);
            int min_speed = map(distance * 10, 0, 1800, 5, 14);  // Multiply everything with 10
            int max_speed = map(distance * 10, 0, 1800, 15, 30);  // Multiply everything with 10
            move.speed = ESP8266TrueRandom.random(min_speed, max_speed) / 10.0;

            program.moves[i] = move;
            startPoint = move.point;
        }
    } else {
        // Read program from LittleFS
        char progFile[8];
        snprintf(progFile, 8, "/%d.json", progNum);
        FileReader programReader = FileReader(progFile);
        int length;
        std::unique_ptr<char[]> buf;
        programReader.read(&buf, &length);

        DynamicJsonDocument doc(4096);
        DeserializationError jsonError = deserializeJson(doc, buf.get());
        if (jsonError) {
            Serial.print("Program "); Serial.print(progFile); Serial.println(" could not be loaded");
            return;
        }

        JsonArray movesList = doc.as<JsonArray>();

        int index = 0;
        for (JsonArray::iterator it=movesList.begin(); it!=movesList.end(); ++it) {
            JsonArray coords = it->as<JsonArray>();
            if (coords.size() != 3) {
                continue;
            }

            movement_t movement;
            point_t point;
            point.x = coords[0].as<int>();
            point.y = coords[1].as<int>();
            movement.point = point;
            movement.speed = coords[2].as<double>();
            program.moves[index] = movement;
            index++;
        }

        Serial.print("Succesfully read program from "); Serial.println(progFile);
    }

    setLaserPower(true);
    laserMotor->startProgram(program);
}

void FluffyLaser::power(char *payload, unsigned int length) {
    int power;
    sscanf(payload, "%d", &power);
    Serial.print("power: "); Serial.println(power);

    if (power == 0) {
        setLaserPower(false);
        laserMotor->stop();
    } else {
        setLaserPower(true);
        runProgram(0);
    }
}

void FluffyLaser::stop(char *payload, unsigned int length) {
    Serial.println("stop");
    setLaserPower(false);
    laserMotor->stop();
}

void FluffyLaser::setLaserPower(bool power) {
    Serial.print("setLaserPower: "); Serial.println(power);
    laserMotor->laserControl(power);
    mqttClient.publish(status_topic, String(power).c_str());
}