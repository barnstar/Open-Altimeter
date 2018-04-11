#include <Arduino.h>
#include <EEPROM.h>

#include "DataLogger.hpp"
#include "types.h"

static const size_t buffer_size = 16*1024;
static const int sample_rate_hz = 20;

DataLogger::DataLogger() {
  log("Data Logger Initialized");
  dataBufferLen = buffer_size / sizeof(DataLogger);
  log("Buffer Size: " + String(sampleCount / sample_rate_hz));

  //This should give us 204 seconds of data... 2.5 minutes...
  dataBuffer = (FlightDataPoint *)malloc(buffer_size);
}

DataLogger::~DataLogger() {
  free dataBuffer;  
}

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


