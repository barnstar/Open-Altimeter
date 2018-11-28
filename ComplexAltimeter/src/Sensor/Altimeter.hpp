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

#ifndef altimeter_h
#define alitmeter_h

#include "../../config.h"
#include "KalmanFilter.h"

#define USE_BMP085 1

// Sensor libraries
#if USE_BMP280
#include "lib/Adafruit_BMP280.h"  //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h
typedef Adafruit_BMP280 Barometer;
#define SEA_LEVEL_PRESSURE 1013.7
#endif

#if USE_BMP085
#include "lib/SFE_BMP180.h"  //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP085.h
typedef SFE_BMP180 Barometer;
#define SEA_LEVEL_PRESSURE 101370
#endif

class Altimeter
{
 public:
  Altimeter(){};
  ~Altimeter(){};

  bool start();
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
