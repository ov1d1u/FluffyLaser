#include "LaserMotor.h"  

LaserMotor::LaserMotor(short _ledPin, short _motorXPin, short _motorYPin) {
    ledPin = _ledPin;
    motorXPin = _motorXPin;
    motorYPin = _motorYPin;
}

LaserMotor::~LaserMotor() {}

void LaserMotor::setup() {
    pinMode(ledPin, OUTPUT);
    servoX.attach(motorXPin);
    servoY.attach(motorYPin);
    move(INIT_X, INIT_Y, 0.5);
}

void LaserMotor::loop() {
    unsigned long progress = millis() - currentMovement.moveStartTime;

    if (progress <= currentMovement.speed) {
        if (currentMovement.speed > 0) {
            if (currentMovement.startX != currentMovement.endX) {
                long angle = map(progress, 0, currentMovement.speed, currentMovement.startX, currentMovement.endX);
                servoX.write(angle); 
            }
            if (currentMovement.startY != currentMovement.endY) {
                long angle = map(progress, 0, currentMovement.speed, currentMovement.startY, currentMovement.endY);
                servoY.write(angle);
            }
        }
    } else {
        currentMovement = getNextMoveRequest();
    }
}

void LaserMotor::move(int x, int y, double speed) {
    if (x >= 0 && x <= 180 && y >= 0 && y <= 180 && speed > 0.0 && speed <= 100.0) {
        currentMovement.moveStartTime = millis();
        currentMovement.startX = servoX.read();
        currentMovement.endX = x;
        currentMovement.startY = servoY.read();
        currentMovement.endY = y;
        currentMovement.speed = speed * 1000;
    }
}

bool LaserMotor::isNullMovement(movement move) {
    return move.speed == 0.0;
}

moverequest_t LaserMotor::getNextMoveRequest() {
    moverequest_t request;

    if (currentMovementIndex >= MAX_MOVES) {
        currentMovementIndex = 0;
    }

    movement nextMovement = movements[currentMovementIndex];
    if (!isNullMovement(nextMovement)) {
        request.moveStartTime = millis();
        request.startX = servoX.read();
        request.startY = servoY.read();
        request.endX = nextMovement.point.x;
        request.endY = nextMovement.point.y;
        request.speed = nextMovement.speed * 1000;

        Serial.print("Next movement:  X: "); Serial.print(nextMovement.point.x); Serial.print("  Y: "); Serial.println(nextMovement.point.y);

        currentMovementIndex++;
    } else {
        currentMovementIndex = 0;
    }
    
    return request;
}

void LaserMotor::laserControl(bool power) {
    digitalWrite(ledPin, power ? HIGH : LOW);
}

void LaserMotor::clearMovements() {
    for (int i = 0; i < MAX_MOVES; i++) {
        movement nullMovement;
        movements[i] = nullMovement;
    }
}

void LaserMotor::move(movement_t move) {
    clearMovements();
    this->move(move.point.x, move.point.y, move.speed);
}

void LaserMotor::startProgram(playprogram_t program) {
    clearMovements();
    currentMovementIndex = 0;
    for (int i = 0; i < MAX_MOVES; i++) {
        movements[i] = program.moves[i];
    }
}

void LaserMotor::stop() {
    clearMovements();
    move(INIT_X, INIT_Y, 1.0);
}