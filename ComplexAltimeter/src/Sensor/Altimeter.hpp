#ifndef altimeter_h
#define alitmeter_h

#include "KalmanFilter.h"
#include "../../config.h"

#define USE_BMP085 1

//Sensor libraries
#if USE_BMP280
#include <Adafruit_BMP280.h>             //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h       
typedef  Adafruit_BMP280  Barometer;
#define SEA_LEVEL_PRESSURE 1013.7
#endif

#if USE_BMP085
#include <Adafruit_BMP085.h>             //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP085.h       
typedef  Adafruit_BMP085 Barometer ;
#define  SEA_LEVEL_PRESSURE  101370
#endif


class Altimeter
{
   public:
   Altimeter() {}
   ~Altimeter() {}
   
   void start();
   bool isReady();
   void update();

   double getAltitude();
   double referenceAltitude();
   double verticalVelocity();

   void scanI2cBus();
      
   private:
   Adafruit_BMP085       barometer;
   bool            barometerReady;
   double          refAltitude;
   KalmanFilter    filter;
   KalmanFilter    velocityFilter;

   long lastRefreshTime;
   double lastRecordedAltitude;

};

#endif
