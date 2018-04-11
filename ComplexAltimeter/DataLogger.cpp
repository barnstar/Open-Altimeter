#include <Arduino.h>
#include <EEPROM.h>

#include "DataLogger.hpp"
#include "types.h"

DataLogger::DataLogger() {}

DataLogger::~DataLogger() {}

DataLogger& DataLogger::sharedLogger()
{
  static DataLogger sharedInstance;
  return sharedInstance;
}

void DataLogger::log(String msg)
{
  Serial.println(msg);
}

bool isFdValid(FlightData *d);
String dataToString(int index, FlightData *d);


String DataLogger::getFlightList()
{
  String flightList;
  size_t maxProgs = EEPROM.length() / sizeof(FlightData);
  FlightData d;
  for (int i = 0; i < maxProgs; i++) {
    EEPROM.get(i * sizeof(FlightData), d);
    if (!isFdValid(&d)) {
      return flightList;
    }  
    flightList+=dataToString(i, &d);
  }
  return String("No Recorded Flights");
}


