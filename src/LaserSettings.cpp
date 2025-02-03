#include "LaserSettings.h"

LaserSettings::LaserSettings() {
    prefs.begin("fluffylaser");
    loadLimits();
    prefs.end();
}

LaserSettings::~LaserSettings() {
}

void LaserSettings::setLimits(movlimits newLimits) {
    prefs.begin("fluffylaser");

    newLimits.minX = max(0, min(newLimits.minX, 180));
    newLimits.maxX = max(0, min(newLimits.maxX, 180));
    newLimits.minY = max(0, min(newLimits.minY, 180));
    newLimits.maxY = max(0, min(newLimits.maxY, 180));

    prefs.putInt("minX", newLimits.minX);
    prefs.putInt("maxX", newLimits.maxX);
    prefs.putInt("minY", newLimits.minY);
    prefs.putInt("maxY", newLimits.maxY);

    limits = newLimits;

    prefs.end();
}

movlimits LaserSettings::getLimits() {
    return limits;
}

void LaserSettings::loadLimits() {
    prefs.begin("fluffylaser");

    limits.minX = prefs.getInt("minX", 0);
    limits.maxX = prefs.getInt("maxX", 180);
    limits.minY = prefs.getInt("minY", 0);
    limits.maxY = prefs.getInt("maxY", 180);
    
    Serial.print("Loaded limits settings: ");
    Serial.print("  x: "); Serial.print(limits.minX); Serial.print(","); Serial.print(limits.maxX);
    Serial.print("  y: "); Serial.print(limits.minY); Serial.print(","); Serial.println(limits.maxY);

    prefs.end();
}