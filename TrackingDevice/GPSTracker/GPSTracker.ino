#include "GPS.h"

GPSTracker* tracker;

void setup()
{
    Serial.begin(9600);
    tracker = new GPSTracker();
    tracker->initModules();
}

void loop()
{
    tracker->run();
}
