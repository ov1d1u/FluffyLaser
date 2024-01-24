
#ifndef ARDUINO_FLUFFYLASER_H
#define ARDUINO_FLUFFYLASER_H
#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <limits.h>
#include <LaserMotor.h>
#include <LaserSettings.h>

#define MAX_RUN_TIME ULONG_MAX
#define DEFAULT_RUN_TIME 5 * 60 * 1000

class FluffyLaser  
{
	private:
		LaserMotor *laserMotor;
		LaserSettings *laserSettings;
		
		long programStartTime;
		long programDuration;
	public:
		uint8_t mac[6];

		FluffyLaser(LaserMotor &_laserMotor, LaserSettings &_laserSettings);
		~FluffyLaser();

		String getClientId();
		void setup(Configuration configuration, uint8_t mac[6]);
		void motorMove(int x, int y, double speed);
		void laserControl(bool power);
		void limitsControl(int minX, int maxX, int minY, int maxY);
		void startProgram(int progNum, unsigned long duration);
		void playProgram(char *payload, unsigned int length);
		void setLaserPower(bool power);
		void runProgram(int progNum, unsigned long duration);
		void power(bool power);
		void stop();
		void loop();
};
#endif
