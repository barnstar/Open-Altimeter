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
   return String(\
       "{flight_idx : " + String(index) + 
       " data : {" +
         "apogee:" + String(d->apogee) + "," +
         "main : " + String(d->ejectionAltitude) + "," + 
         "drogue_alt : " + String(d->drogueEjectionAltitude) + "," + 
         "max_acc : " + String(d->maxAcceleration) + "," + 
         "apogee_time : " + String(d->apogeeTime) + "," +
         "acc_trigger_time :" + String(d->accTriggerTime) + "," + 
         "alt_trigger_time :  " + String(d->altTriggerTime) + "}}");
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
