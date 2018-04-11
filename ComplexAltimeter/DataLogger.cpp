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

void DataLogger::logDataPoint(FlightDataPoint p, bool isTriggerPoint)
{
  if(dataPointsLogged == dataBufferLen && triggerIndex) {
    return;
  }

  if(isTriggerPoint) {
    triggerIndex = dataIndex;
    dataPointsLogged = MIN(dataPointsLogged, 100);
  }

  dataBuffer[dataIndex] = p;
  dataIndex++;
  dataPointsLogged++;

  if(dataIndex == dataBufferLen) {
    dataIndex = 0;
  }
}

void DataLogger::writeBufferToFile(FlightData &data, int index)
{


}

void DataLogger::clearBuffer()
{
  triggerIndex = 0;
  dataPointsLogged = 0;
  dataIndex = 0;
  FlightData d;
  for(int i=0; i<dataBufferLen; i++) {
    dataBuffer[i] = d;
  }
}

FlightDataPoint* DataLogger::getDataBuffer()
{
  return dataBuffer;
}


int DataLogger::dataBufferLength()
{
  return dataBufferLength;
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


String FlightDataPoint::toJson()
{
  return String("{\"time\":" +  String(millis) + "," +
                "\"alt\":" + String(alt) + "," +
                "\"acc\":" + String(acc) + "}");
}
