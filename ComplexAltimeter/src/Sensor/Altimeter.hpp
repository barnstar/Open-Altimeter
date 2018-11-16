#ifndef altimeter_h
#define alitmeter_h

#include "Sensor/KalmanFilter.h"

//Sensor libraries
#if USE_BMP280
#include <Adafruit_BMP280.h>             //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h       
typedef  Adafruit_BMP280  Barometer;
const double SEA_LEVEL_PRESSURE = 1013.7;
#endif

#if USE_BMP085
#include <Adafruit_BMP085.h>             //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP085.h       
typedef  Adafruit_BMP085 Barometer ;
const double SEA_LEVEL_PRESSURE = 101370;
#endif


class Altimeter
{
   public:
   Altimeter();
   ~Altimeter();
   
   void start();
   bool isReady();
   void update()

   double getAltitude();
   double referenceAltitude();
   double verticalVelocity();

   void scanI2cBus();
      
   private:
   Barometer       barometer;
   bool            barometerReady;
   double          refAltitude;
   KalmanFilter    filter;
   KalmanFilter    velocityFilter;

   long lastRefreshTime;
   double lastRecordedAltitude;

};

#endif
