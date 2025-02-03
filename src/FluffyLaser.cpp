#include <ESPTrueRandom.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "FluffyLaser.h"
#include "FileReader.h"

FluffyLaser::FluffyLaser(LaserMotor &_laserMotor, LaserSettings &_laserSettings)
{
    laserMotor = &_laserMotor;
    laserSettings = &_laserSettings;
}

FluffyLaser::~FluffyLaser() {}

void FluffyLaser::loop() {
    long now = millis();

    laserMotor->loop();

    if ((programStartTime != 0 && programDuration != 0) && now > programStartTime + programDuration) {
        programStartTime = 0;
        programDuration = 0;
        stop();
    }
}

void FluffyLaser::setup(Configuration configuration, uint8_t mac[6]) {
    memcpy(this->mac, mac, sizeof(this->mac));
    laserMotor->setup();
}

String FluffyLaser::getClientId() {
    char macStr[18];
    sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return String("FluffyLaser-") + macStr;
}

void FluffyLaser::motorMove(int x, int y, double speed) {
    Serial.print("motorMove: "); Serial.print("  x: "); Serial.print(x); Serial.print("  y:"); Serial.print(y); Serial.print("  speed:"); Serial.println(speed);
    movement move;
    move.point.x = x;
    move.point.y = y;
    move.speed = speed;
    laserMotor->move(move);
}

void FluffyLaser::laserControl(bool power) {
    Serial.print("laserControl: "); Serial.println(power);
    setLaserPower(power);
}

void FluffyLaser::limitsControl(int minX, int maxX, int minY, int maxY) {
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

void FluffyLaser::playProgram(const char *payload, unsigned int length) {
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

void FluffyLaser::startProgram(int progNum, unsigned long duration) {
    Serial.print("startProgram: ");
    Serial.print(progNum);

    Serial.print(", Duration: ");
    Serial.println(duration);
    
    runProgram(progNum, duration);
}

void FluffyLaser::runProgram(int progNum, unsigned long duration) {
    playprogram_t program;

    programStartTime = millis();
    programDuration = duration;

    if (progNum == 0) {
        // Generate program
        movlimits limits = laserSettings->getLimits();
        Serial.print("Limits: "); Serial.print(limits.minX); Serial.print(","); Serial.print(limits.maxX); Serial.print(","); Serial.print(limits.minY); Serial.print(","); Serial.println(limits.maxY);
        
        point_t startPoint = { .x = INIT_X, .y = INIT_Y };

        for (int i = 0; i < MAX_MOVES; i++) {
            movement move;
            point_t point;
            // move.speed = ESPTrueRandom.random(20, 40) / 10.0;
            point.x = ESPTrueRandom.random(limits.minX, limits.maxX);
            point.y = ESPTrueRandom.random(limits.minY, limits.maxY);
            move.point = point;
            
            int distance = sqrt(pow(move.point.x - startPoint.x, 2) + pow(move.point.y - startPoint.y, 2) * 1.0);
            int min_speed = map(distance * 10, 0, 1800, 5, 14);  // Multiply everything with 10
            int max_speed = map(distance * 10, 0, 1800, 15, 30);  // Multiply everything with 10
            move.speed = ESPTrueRandom.random(min_speed, max_speed) / 10.0;

            program.moves[i] = move;
            startPoint = move.point;
        }
    } else {
        // Read program from LittleFS
        Serial.print("Program "); Serial.print(progNum); Serial.println(" selected");
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

void FluffyLaser::power(bool power) {
    Serial.print("power: "); Serial.println(power);

    if (power == false) {
        setLaserPower(false);
        laserMotor->stop();
    } else {
        setLaserPower(true);
        runProgram(0, DEFAULT_RUN_TIME);
    }
}

void FluffyLaser::stop() {
    Serial.println("stop");
    setLaserPower(false);
    laserMotor->stop();
}

void FluffyLaser::setLaserPower(bool power) {
    Serial.print("setLaserPower: "); Serial.println(power);
    laserMotor->laserControl(power);
}