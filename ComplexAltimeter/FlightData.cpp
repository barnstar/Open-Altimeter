#include "FlightData.hpp"

const bool FlightData::isValid()
{
  return apogee ||
         ejectionAltitude ||
         drogueEjectionAltitude ||
         maxAcceleration ||
         burnoutAltitude;
}


const String FlightData::toString(int index)
{
   return String(\
       "{\"flight_idx\" : " + String(index) +
       " , \"data\" : {" +
         " \"apogee\":" + String(apogee) +"," +
         " \"main\" : " + String(ejectionAltitude) + "," +
         " \"drogue_alt\" : " + String(drogueEjectionAltitude) + "," +
         " \"max_acc\" : " + String(maxAcceleration) + "," +
         " \"apogee_time\" : " + String(apogeeTime) + "," +
         " \"acc_trigger_time\" :" + String(accTriggerTime) + "," +
         " \"alt_trigger_time\" :  " + String(altTriggerTime) + "}}");
}

void FlightData::reset()
{
  apogee = 0;
  ejectionAltitude = 0;
  drogueEjectionAltitude = 0;
  maxAcceleration = 0;
  burnoutAltitude = 0;
  accTriggerTime = 0;
  altTriggerTime = 0;
  apogeeTime = 0;
}
