#ifndef flightdata_h
#define flightdata_h

#include <Arduino.h>

class FlightData  {
  public:
  double apogee = 0;
  double ejectionAltitude = 0;
  double drogueEjectionAltitude = 0;
  double maxAcceleration = 0;
  double burnoutAltitude = 0;

  int    apogeeTime;
  int    accTriggerTime;
  int    altTriggerTime;

  const String toString(int index);
  const bool isValid();

  void reset();
};




#endif
