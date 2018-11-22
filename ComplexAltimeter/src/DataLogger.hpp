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


#ifndef datalogger_h
#define datalogger_h

#include <Arduino.h>
#include "FlightData.hpp"

typedef void (*PrintCallback)(const String &line);

class PrintFnc
{
 public:
  String operator()(String x)
  {
    ret += x;
    return ret;
  }
  String operator()(void) { return ret; }

 private:
  String ret;
};

void logLine(const String &s);

#define FLIGHTS_DIR "/flights"

class FlightDataPoint
{
 public:
  FlightDataPoint() {}

  FlightDataPoint(long ltime, double altitude, double acelleration)
      : ltime(ltime), altitude(altitude), acelleration(acelleration)
  {
  }

  long ltime          = 0;
  double altitude     = 0;
  double acelleration = 0;

  String toJson();

  void reset()
  {
    ltime        = 0;
    altitude     = 0;
    acelleration = 0;
  }
};

class DataLogger
{
 public:
  static DataLogger &sharedLogger();

  DataLogger();
  ~DataLogger();

  void initializeBuffer(int sampleRate, size_t bufferSize);

  FlightDataPoint *getDataBuffer();
  int dataBufferLength();

  static void log(const String &msg);
  static String getFlightList();

  void saveFlight(FlightData &d, int index);
  void logDataPoint(FlightDataPoint &p, bool isTriggerPoint);
  void writeFlightDataFileWithIndex(FlightData &ddata, int index);

  void clearBuffer();
  void printFlightData();
  void printBufferData();

  void readFlightData(PrintCallback callback);
  void readFlightDetails(int index, PrintCallback callback);

  void readBufferData(PrintCallback callback);

  int nextFlightIndex();

  static void resetAll();

  DataLogger(DataLogger const &) = delete;
  void operator=(DataLogger const &) = delete;

 private:
  FlightDataPoint *dataBuffer;
  int dataIndex        = 0;
  int dataBufferLen    = 0;
  int triggerIndex     = -1;
  int dataPointsLogged = 0;
};

#endif
