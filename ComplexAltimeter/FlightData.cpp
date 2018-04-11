#include "FlightData.hpp"

bool isFdValid(FlightData *d) {
  return d->apogee || 
         d->ejectionAltitude ||
         d->drogueEjectionAltitude || 
         d->maxAcceleration ||
         d->burnoutAltitude;
}


String dataToString(int index, FlightData *d)
{
   return String("Flight " + String(index) + 
       " : [Apogee " + String(d->apogee) + 
       "m] : [Main " + String(d->ejectionAltitude) + 
       "m] : [Drogue " + String(d->drogueEjectionAltitude) + 
       "m] : [Acc " + String(d->maxAcceleration) + 
       "ms] : [Apogee Time  " + String(d->apogeeTime) +
       "ms]: [Acc Trigger Time" + String(d->accTriggerTime) + 
       "ms] : [Alt Trigger Time " + String(d->altTriggerTime) + "s]<br>");
}

void resetFlightData(FlightData *d) {
  d->apogee = 0;
  d->ejectionAltitude = 0;
  d->drogueEjectionAltitude = 0;
  d->maxAcceleration = 0;
  d->burnoutAltitude = 0; 
  d->accTriggerTime = 0;
  d->altTriggerTime = 0;
  d->apogeeTime = 0;

}
