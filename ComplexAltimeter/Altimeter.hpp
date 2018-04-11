#ifndef altimeter_h
#define alitmeter_h

#include <Adafruit_BMP280.h> 

class Altimeter
{
   public:
   Altimeter();
   ~Altimeter();
   
   void start();
   bool isReady();

   double getAltitude();
   
   private:
   Adafruit_BMP280 barometer;
   bool            barometerReady;
   double          refAltitude;
};

#endif
