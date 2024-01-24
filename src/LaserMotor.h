
#ifndef ARDUINO_LASERMOTOR_H
#define ARDUINO_LASERMOTOR_H
#include <Arduino.h>
#if defined(ESP8266)
#include <Servo.h>
#elif defined(ESP32)
#include <ESP32Servo.h>
#endif
#include <Defines.h>


// Internal use only
typedef struct moverequest {
	int startX = 0;
	int endX = 0;
	int startY = 0;
	int endY = 0;
	double speed = 0.0;
	unsigned long moveStartTime;
} moverequest_t;

typedef struct point {
	int x;
	int y;
} point_t;

typedef struct movement {
	point_t point;
	double speed = 0;
} movement_t;

typedef struct playprogram {
	movement_t moves[MAX_MOVES];
} playprogram_t;

class LaserMotor  
{
	private:
		Servo servoX;
		Servo servoY;

		short ledPin;
		short motorXPin;
		short motorYPin;

		moverequest_t currentMovement;
		movement_t movements[MAX_MOVES];
		int currentMovementIndex;

		moverequest_t getNextMoveRequest();
		void move(int x, int y, double speed = 1.0);
		// void runProgram(playprogram_t program);
		bool isNullMovement(movement move);
		void clearMovements();
	public:
		LaserMotor(short _ledPin, short _motorXPin, short _motorYPin);
		void setup();
		
		void loop();
		void laserControl(bool power);

		void startProgram(playprogram_t program);
		void move(movement move);
		void stop();
		~LaserMotor();

};
#endif
