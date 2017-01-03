#include "GPS.h"


GPSTracker* tracker;

void setup(){
  tracker = new GPSTracker();
  tracker->initModules();
  Serial.begin(9600);
}

void loop(){
  tracker->run() ;
}
