#ifndef flightdata_h
#define flightdata_h

#include <Arduino.h>

typedef struct  {
  double apogee = 0;
  double ejectionAltitude = 0;
  double drogueEjectionAltitude = 0;
  double maxAcceleration = 0;
  double burnoutAltitude = 0;

  int    apogeeTime;
  int    accTriggerTime;
  int    altTriggerTime;   
} FlightData;


String dataToString(int index, FlightData *d);
bool isFdValid(FlightData *d);
void resetFlightData(FlightData *d);

#endif
