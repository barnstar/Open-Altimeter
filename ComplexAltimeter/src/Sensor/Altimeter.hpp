#ifndef altimeter_h
#define alitmeter_h

#include "../../config.h"
#include "KalmanFilter.h"

#define USE_BMP085 1

// Sensor libraries
#if USE_BMP280
#include <Adafruit_BMP280.h>  //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h
typedef Adafruit_BMP280 Barometer;
#define SEA_LEVEL_PRESSURE 1013.7
#endif

#if USE_BMP085
#include "SFE_BMP180.h"  //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP085.h
typedef SFE_BMP180 Barometer;
#define SEA_LEVEL_PRESSURE 101370
#endif

class Altimeter
{
 public:
  Altimeter(){};
  ~Altimeter(){};

  void start();
  bool isReady();
  void update();
  void reset();

  double getAltitude();
  double referenceAltitude();
  double verticalVelocity();
  double getPressure();

  void scanI2cBus();

 private:
  Barometer barometer;
  bool barometerReady = false;
  double refAltitude  = 0;
  KalmanFilter filter;
  KalmanFilter velocityFilter;
  double baselinePressure = 0;

  long lastRefreshTime        = 0;
  double lastRecordedAltitude = 0;
};

#endif
