
#ifndef ARDUINO_FLUFFYLASER_H
#define ARDUINO_FLUFFYLASER_H
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LaserMotor.h>
#include <LaserSettings.h>
#include <PubSubClient.h>

class FluffyLaser  
{
	private:
		PubSubClient mqttClient;
		LaserMotor *laserMotor;
		LaserSettings *laserSettings;

		bool _connect();

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
			x
		where:
			x - int >= 0 && <=9, the desired program number

		Example MQTT message:
			0
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
		FluffyLaser(WiFiClient &client, LaserMotor &_laserMotor, LaserSettings &_laserSettings);
		~FluffyLaser();
		void setLaserPower(bool power);
		void runProgram(int progNum);
		void loop();
		boolean connect(String server, uint16_t port, String user, String pass);
		void mqttCallback(char *topic, byte *payload, unsigned int length);
};
#endif
