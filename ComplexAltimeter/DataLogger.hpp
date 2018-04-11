#ifndef datalogger_h
#define datalogger_h

#include <Arduino.h>


class FlightDataPoint
{
  public:
    FlightDataPoint() {}

    FlightDataPoint(long millis, double altitude, double acelleration) :
        millis(millis)
        altitude(altitude),
        acelleration(acelleration)
        {}

    long millis = 0;
    double altitude = 0;
    double acelleration = 0;

    String toJson();
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

   static void log(String msg);
   static String getFlightList();

   void logDataPoint(FlightDataPoint p, bool isTriggerPoint);
   void writeBufferToFile(FlightData &ddata, const String& path);


   DataLogger(DataLogger const&)      = delete;
   void operator=(DataLogger const&)  = delete;

   private:
   FlightDataPoint *dataBuffer;
   int dataIndex = 0;
   int dataBufferLen = 0;
   int triggerIndex = 0;
   int dataPointsLogged = 0;

};

#endif
