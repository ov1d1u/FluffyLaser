#if defined(ESP32)
#ifndef ARDUINO_FLUFFYLASERBT_H
#define ARDUINO_FLUFFYLASERBT_H
#include <Arduino.h>
#include <limits.h>
#include <LaserMotor.h>
#include <LaserSettings.h>
#include <PubSubClient.h>
#include "FluffyLaserMQTT.h"

#define MAX_RUN_TIME ULONG_MAX
#define DEFAULT_RUN_TIME 5 * 60 * 1000

class FluffyLaserBTCallbacks;

class FluffyLaserBT: public FluffyLaserMQTT
{
	private:
		FluffyLaserBTCallbacks *callbacks;
		
	public:
		FluffyLaserBT(WiFiClient &client, LaserMotor &_laserMotor, LaserSettings &_laserSettings);
		~FluffyLaserBT();
		void setup(Configuration configuration, uint8_t mac[6]);
		void loop();
};
#endif
#endif