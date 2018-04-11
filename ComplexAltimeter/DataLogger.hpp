#ifndef datalogger_h
#define datalogger_h

#include <Arduino.h>

class DataLogger
{
   public:
   static DataLogger& sharedLogger();
   
   DataLogger();
   ~DataLogger();
   
   static void log(String msg);
   static String getFlightList();


   public:
     DataLogger(DataLogger const&)       = delete;
      void operator=(DataLogger const&)  = delete;

};

#endif
