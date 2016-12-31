#include <LWiFi.h>
#include <LWiFiServer.h>

#include "GPS.h"
// Global vars

GPSTracker* tracker;

void setup(){
  tracker = new GPSTracker();
  tracker->initModules();
  Serial.begin(9600);
}

void loop(){
  tracker->run() ;
}
