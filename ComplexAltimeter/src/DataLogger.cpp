/*********************************************************************************
 * Open Altimeter
 *
 * Mid power rocket avionics software for altitude recording and dual deployment
 *
 * Copyright 2018, Jonathan Nobels
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **********************************************************************************/

#include <Arduino.h>

#include "DataLogger.hpp"
#include "FlightData.hpp"
#include "types.h"

DataLogger::DataLogger()
{
  log(F(":: Data Logger Initialized ::"));

  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    Serial.print(dir.fileName() + "  size:");
    File f = dir.openFile("r");
    Serial.println(f.size());
  }
  printFlightData();
  clearBuffer();
}

void DataLogger::resetAll()
{
  log(F("Erasing all flight data"));
  File f = SPIFFS.open("/flights.txt", "w");
  log(F("Erasing all flight data"));
  f.print("");
  f.close();

  f = SPIFFS.open("/apogeeHistory.txt", "w");
  log(F("Erasing all flight history"));
  f.print("");
  f.close();

  f = SPIFFS.open("/flightCount.txt", "w");
  log(F("Setting flight count to zero"));
  f.print("");
  f.close();

  Dir dir = SPIFFS.openDir(FLIGHTS_DIR);
  while (dir.next()) {
    Serial.print("Deleting " + dir.fileName());
    SPIFFS.remove(dir.fileName());
  }
}

DataLogger::~DataLogger() {}

DataLogger &DataLogger::sharedLogger()
{
  static DataLogger sharedInstance;
  return sharedInstance;
}

void DataLogger::logDataPoint(FlightDataPoint &p, bool isTriggerPoint)
{
  if (isTriggerPoint) {
    triggerIndex = dataIndex;
    int idx      = dataIndex;
    int logCount = MIN(dataPointsLogged, dataBufferLen);
    for (int i = 0; i < logCount; i++) {
      if(i) {
        dataFile.print(",");
      }
      dataFile.println(dataBuffer[idx].toJson());
      idx = (idx == dataBufferLen) ? 0 : idx + 1;
    }
  } else if (triggerIndex != -1) {
    dataFile.print(",");
    dataFile.println(p.toJson());
  } else {
    dataBuffer[dataIndex] = p;
    dataIndex = (dataIndex == dataBufferLen - 1) ? 0 : dataIndex + 1;
    dataPointsLogged++;
  }
}

void DataLogger::readFlightDetails(int index, PrintCallback callback)
{
  String path = String(FLIGHTS_DIR) + String("/") + String(index);
  File f      = SPIFFS.open(path, "r");
  if (f) {
    while (f.available()) {
      String line = f.readStringUntil('\n');
      callback(line);
    }
    f.close();
  }
}

void DataLogger::openFlightDataFileWithIndex(int index)
{
  DataLogger::log(F("Opening flight data file.."));
  String path = String(FLIGHTS_DIR) + String("/") + String(index);
  dataFile    = SPIFFS.open(path, "w");
  dataFile.print("var flightData = { \"data\":[");
}

bool DataLogger::closeFlightDataFile()
{
  DataLogger::log(F("Closing flight data file.."));
  dataFile.println("]}\n\n");
  dataFile.close();
  clearBuffer();
}

void DataLogger::clearBuffer()
{
  triggerIndex     = -1;
  dataPointsLogged = 0;
  dataIndex        = 0;
  for (int i = 0; i < dataBufferLen; i++) {
    dataBuffer[i].reset();
  }
}

FlightDataPoint *DataLogger::getDataBuffer() { return dataBuffer; }

int DataLogger::dataBufferLength() { return dataPointsLogged; }

void DataLogger::printFlightData() { readFlightData(logLine); }

void DataLogger::readFlightData(PrintCallback callback)
{
  File f = SPIFFS.open("/flights.txt", "r");
  if (!f.available()) {
    callback("file open failed");
  } else {
    while (f.available()) {
      String line = f.readStringUntil('\n');
      callback(line);
    }
  }
  f.close();
}

void DataLogger::printBufferData() { readBufferData(logLine); }

void DataLogger::readBufferData(PrintCallback callback)
{
  if (triggerIndex == -1) {
    callback(F("No Data to Log"));
    return;
  }

  int idx = triggerIndex;
  for (int i = 0; i < dataBufferLen; i++) {
    callback(dataBuffer[idx].toJson());
    idx = (idx == dataBufferLen) ? 0 : idx + 1;
  }
}

void logLine(const String &s) { DataLogger::log(s); }

void DataLogger::log(const String &msg) { Serial.println(msg); }

String DataLogger::apogeeHistory()
{
  File f        = SPIFFS.open("/apogeeHistory.txt", "r");
  String retVal = "Flight History\n";
  int lineCount = 0;
  while (f.available()) {
    f.readStringUntil('\n');
    lineCount++;
  }

  f.seek(0);
  const byte maxLines = 7;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    if (lineCount <= maxLines) {
      retVal = retVal + line + String("\n");
    }
    lineCount--;
  }
  f.close();
  return retVal;
}

void DataLogger::endDataRecording(FlightData &d, int index)
{
  closeFlightDataFile();

  File f;

  f = SPIFFS.open("/flights.txt", "a");
  f.seek(0, SeekEnd);
  f.println(d.toString(index));
  f.close();
  log(F("Saved Flight"));

  f            = SPIFFS.open("/apogeeHistory.txt", "a");
  String entry = String(d.apogee) + String(" | ") + String(d.maxAcceleration);
  f.seek(0, SeekEnd);
  f.println(entry);
  f.close();

  f               = SPIFFS.open("/flightCount.txt", "w");
  String indexStr = String(index);
  f.println(indexStr);
  f.close();
  log("Updated Flight Count: " + indexStr);
}

int DataLogger::nextFlightIndex()
{
  int index = 0;
  File f    = SPIFFS.open("/flightCount.txt", "r");
  if (f.available()) {
    String line = f.readStringUntil('\n');
    index       = line.toInt();
    log("Flight Count: " + String(index));
    index++;
  }
  f.close();
  return index;
}

String FlightDataPoint::toJson()
{
  return String("{\"x\":" + String(ltime) + "," + "\"y\":" + String(altitude) +
                "," + "\"g\":" + String(acelleration) + "}");
}
