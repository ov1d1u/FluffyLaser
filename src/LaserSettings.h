
#ifndef ARDUINO_LASERSETTINGS_H
#define ARDUINO_LASERSETTINGS_H
#include <Arduino.h>
#include <Preferences.h>
#include <LaserMotor.h>

struct movlimits {
	int minX;
	int maxX;
	int minY;
	int maxY;
};

struct storedProgram {
	movement moves[MAX_MOVES];
};

class LaserSettings  
{
	private:
		Preferences prefs;

	public:
		LaserSettings();
		~LaserSettings();
		void setLimits(movlimits limits);
		movlimits getLimits();
};
#endif
