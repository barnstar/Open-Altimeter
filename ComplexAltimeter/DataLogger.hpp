#ifndef datalogger_h
#define datalogger_h

#include <Arduino.h>
#include "FlightData.hpp"

typedef void (*PrintCallback)(const String& line);

void logLine(const String &s);

#define FLIGHTS_DIR "/flights"


class FlightDataPoint
{
  public:
    FlightDataPoint() {}

    FlightDataPoint(long ltime, double altitude, double acelleration) :
        ltime(ltime),
        altitude(altitude),
        acelleration(acelleration)
        {}

    long ltime = 0;
    double altitude = 0;
    double acelleration = 0;

    String toJson();

    void reset() {
      ltime = 0;
      altitude = 0;
      acelleration = 0;
    }
};


class DataLogger
{
public:
    static DataLogger& sharedLogger();

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

    DataLogger(DataLogger const&)      = delete;
    void operator=(DataLogger const&)  = delete;

private:
    FlightDataPoint *dataBuffer;
    int dataIndex = 0;
    int dataBufferLen = 0;
    int triggerIndex = -1;
    int dataPointsLogged = 0;
};

#endif
