#include "LaserSettings.h"  

LaserSettings::LaserSettings(){}

LaserSettings::~LaserSettings(){}

void LaserSettings::setLimits(movlimits limits) {
    prefs.begin("fluffylaser");
    prefs.putInt("minX", max(0, min(limits.minX, 180)));
    prefs.putInt("maxX", max(0, min(limits.maxX, 180)));
    prefs.putInt("minY", max(0, min(limits.minY, 180)));
    prefs.putInt("maxY", max(0, min(limits.maxY, 180)));
    prefs.end();
}

movlimits LaserSettings::getLimits() {
    movlimits limits;
    prefs.begin("fluffylaser");
    limits.minX = prefs.getInt("minX", 0);
    limits.maxX = prefs.getInt("maxX", 180);
    limits.minY = prefs.getInt("minY", 0);
    limits.maxY = prefs.getInt("maxY", 180);
    prefs.end();
    return limits;
}