#include <Arduino.h>
#include <EEPROM.h>

#include "DataLogger.hpp"
#include "FlightData.hpp"
#include "types.h"
#include "FS.h"

static const size_t buffer_size = 16*1024;
static const int sample_rate_hz = 20;

DataLogger::DataLogger() 
{
  log("Data Logger Initialized");
  dataBufferLen = buffer_size / sizeof(FlightDataPoint);
  log("Buffer Size: " + String(buffer_size / sample_rate_hz));

  //This should give us 204 seconds of data... 2.5 minutes...
  dataBuffer = new FlightDataPoint[dataBufferLen];

  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
      Serial.print(dir.fileName());
      File f = dir.openFile("r");
      Serial.println(f.size());
  }
  printFlightData();

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
  if((dataPointsLogged == dataBufferLen) && triggerIndex != -1) {
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
  triggerIndex = -1;
  dataPointsLogged = 0;
  dataIndex = 0;
  for(int i=0; i<dataBufferLen; i++) {
    dataBuffer[i].reset();
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



 void DataLogger::printFlightData()
 {
    readFlightData(logLine);
 }
 
 void DataLogger::readFlightData(PrintCallback callback)
 {
   File f = SPIFFS.open("/flights.txt", "r");
  if (!f) {
    callback("file open failed");
    return;
  }
  
  while (f.available()){
    String line = f.readStringUntil('\n');
    callback(line);
  }
  f.close();
}
 
 
void DataLogger::printBufferData()
{
  readBufferData(logLine);
}
 
 void DataLogger::readBufferData(PrintCallback callback)
 {  
    if(triggerIndex == -1) {
      callback(String("No Data to Log"));
      return;    
    }
    
    int idx=triggerIndex;
    for(int i=0; i<dataBufferLen; i++) {
      callback(dataBuffer[idx].toJson());
      idx = (idx == dataBufferLen) ? 0 : idx+1;
    }
 }


void logLine(const String &s) {
  DataLogger::log(s);
}

void DataLogger::log(const String &msg)
{
  Serial.println(msg);
}


void DataLogger::saveFlight(FlightData &d, int index)
{
  File f = SPIFFS.open("/flights.txt", "a");
  if (!f) {
    log("Unalble to save flight. No File");
    return;
  }
  f.seek(0, SeekEnd);
  f.println(d.toString(index));
  f.close();
  log("Saved Flight");


  f = SPIFFS.open("/flightCount.txt", "w");
  String indexStr = String(index+1);
  f.println(indexStr);
  f.close();
  log("Updtaed Flight Count: " + indexStr);
}

int DataLogger::nextFlightIndex()
{
  File f = SPIFFS.open("/flightCount.txt", "r");
  if (!f) { 
    return 0;
  }
  String line = f.readStringUntil('\n');
  int index = line.toInt(); 
  f.close();
  log("Flight Count: " + String(index));
  return index++;
}



String FlightDataPoint::toJson()
{
  return String("{\"time\":" +  String(ltime) + "," +
                "\"alt\":" + String(altitude) + "," +
                "\"acc\":" + String(acelleration) + "}");
}
