#include <Arduino.h>
#include <EEPROM.h>

#include "DataLogger.hpp"
#include "FlightData.hpp"
#include "types.h"

static const size_t buffer_size = 16*1024;
static const int sample_rate_hz = 20;

DataLogger::DataLogger() 
{
  log("Data Logger Initialized");
  dataBufferLen = buffer_size / sizeof(DataLogger);
  log("Buffer Size: " + String(buffer_size / sample_rate_hz));

  //This should give us 204 seconds of data... 2.5 minutes...
  dataBuffer = (FlightDataPoint *)malloc(buffer_size);
}

DataLogger::~DataLogger() 
{
  free(dataBuffer);
}

DataLogger& DataLogger::sharedLogger()
{
  static DataLogger sharedInstance;
  return sharedInstance;
}

void DataLogger::logDataPoint(FlightDataPoint &p, bool isTriggerPoint)
{
  if(dataPointsLogged == dataBufferLen && triggerIndex) {
    return;
  }

  if(isTriggerPoint) {
    triggerIndex = dataIndex;
    if(dataPointsLogged > 100){
      dataPointsLogged = 100;
    }
  }

  dataBuffer[dataIndex] = p;
  dataIndex++;
  dataPointsLogged++;

  if(dataIndex == dataBufferLen) {
    dataIndex = 0;
  }
}

void DataLogger::writeBufferToFile(FlightData &ddata, const String& path)
{


}

void DataLogger::clearBuffer()
{
  triggerIndex = 0;
  dataPointsLogged = 0;
  dataIndex = 0;
  FlightDataPoint d;
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
  return dataPointsLogged;
}

void DataLogger::log(String msg)
{
  Serial.println(msg);
}

String DataLogger::getFlightList()
{
  String flightList;
  size_t maxProgs = EEPROM.length() / sizeof(FlightData);
  FlightData d;
  for (int i = 0; i < maxProgs; i++) {
    EEPROM.get(i * sizeof(FlightData), d);
    if (!d.isValid()) {
      return flightList;
    }
    flightList+=d.toString(i);
  }
  return String("No Recorded Flights");
}


String FlightDataPoint::toJson()
{
  return String("{\"time\":" +  String(ltime) + "," +
                "\"alt\":" + String(altitude) + "," +
                "\"acc\":" + String(acelleration) + "}");
}
