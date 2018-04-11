#ifndef datalogger_h
#define datalogger_h

#include <Arduino.h>


struct FlightDataPoint
{
  long millis;
  double altitude;
  double acelleration;
};


class DataLogger
{
   public:
   static DataLogger& sharedLogger();
   
   DataLogger();
   ~DataLogger();

   void initializeBuffer(int sampleRate, size_t bufferSize);
   
   static void log(String msg);
   static String getFlightList();

   DataLogger(DataLogger const&)       = delete;
   void operator=(DataLogger const&)  = delete;
  
   private:
   FlightDataPoint *dataBuffer;
   FlightDataPoint dataIndex;
   int dataBufferLen;


};

#endif
