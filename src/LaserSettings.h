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

class LaserSettings  
{
    private:
        Preferences prefs;
        movlimits limits;

    public:
        LaserSettings();
        ~LaserSettings();
        void setLimits(movlimits limits);
        movlimits getLimits();
        void loadLimits();  // New method declaration
};
#endif