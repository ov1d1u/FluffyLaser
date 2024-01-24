
#ifndef ARDUINO_FLUFFYLASERMQTT_H
#define ARDUINO_FLUFFYLASERMQTT_H
#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <limits.h>
#include <LaserMotor.h>
#include <LaserSettings.h>
#include <PubSubClient.h>
#include "FluffyLaser.h"

#define MAX_RUN_TIME ULONG_MAX
#define DEFAULT_RUN_TIME 5 * 60 * 1000

class FluffyLaserMQTT: public FluffyLaser  
{
	private:
		PubSubClient mqttClient;

		bool _connect();

		char* getTopicName(const char* topicName);

		// MQTT Messages
		/* 
		Move motor to a specific coordinate. Message syntax:
			x,y,z
		where:
			x - int >= 0 && <= 180, the horizontal position
			y - int >= 0 && <= 180, the vertical position
			z - double > 0.0 && < 100.0, the movement speed

		Example MQTT message:
			60,90,2.0
		*/
		void motorMove(char *payload, unsigned int length);

		/*
		Turn on or off the laser light. Message syntax:
			x
		where:
			x - int 1 || 0, the desired laser light state

		Example MQTT message:
			1
		*/
		void laserControl(char *payload, unsigned int length);

		/*
		Configure the limits of the coordinates randomly generated when using program 0.
		Note that this limits does not apply to other program or to the `motorMove` command.
		Message syntax:
			minX,maxX,minY,maxY
		where:
			minX - int >= 0 && <= 180, the minimum horizontal position
			maxX - int >= 0 && <= 180, the maximum horizontal position
			minY - int >= 0 && <= 180, the minimum vertical position
			maxY - int >= 0 && <= 180, the maximum vertical position

		Example MQTT message:
			20,160,10,80
		*/
		void limitsControl(char *payload, unsigned int length);

		/*
		Start a stored program. Message syntax:
			x,y
		where:
			x - int >= 0 && <=9, the desired program number
			y - long > 0, the duration of the program in milliseconds

		Example MQTT message:
			0,30000
		*/
		void startProgram(char *payload, unsigned int length);

		/* 
		Play a sequence of coordinates. Maximum 50 pairs of commands. Message syntax:
			x,y,z,x,y,z,...
		where:
			x - int >= 0 && <= 180, the horizontal position
			y - int >= 0 && <= 180, the vertical position
			z - int >= 0 && <= 180, the speed

		Example MQTT message:
			10,60,2.0,30,10,1.5,80,120,1.5,60,90,2.0
		*/
		void playProgram(char *payload, unsigned int length);

		/*
		Turns on or off the device. When turning the device on, the selected program will be 0
		and the laser light will be automatically turned on. Message syntax:
			x
		where:
			x - int 0 || 1, the state of power
		*/
		void power(char *payload, unsigned int length);

		/*
		Stop the device at the current position. The laser light will be turned off.
		Message is empty.
		*/
		void stop(char *payload, unsigned int length);
	public:
		FluffyLaserMQTT(WiFiClient &client, LaserMotor &_laserMotor, LaserSettings &_laserSettings);
		~FluffyLaserMQTT();
		void setup(Configuration configuration, uint8_t mac[6]);
		boolean connect(String server, uint16_t port, String user, String pass);
		void mqttCallback(char *topic, byte *payload, unsigned int length);
		void loop();
};
#endif
