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

#include "../../Configuration.h"
#include "Filters.hpp"

#define USE_BMP085 1

// Sensor libraries
#if USE_BMP280
#include "lib/Adafruit_BMP280.h"  
typedef Adafruit_BMP280 Barometer;
#define SEA_LEVEL_PRESSURE 1013.7
#endif

#if USE_BMP085
#include "lib/SFE_BMP180.h"  
typedef SFE_BMP180 Barometer;  //180 and 085 use the same interface
#define SEA_LEVEL_PRESSURE 101370
#endif

class Altimeter
{
 public:
  Altimeter() : velocityFilter(0,0.5) {
    altitudeFilter.reset(0);
    velocityFilter.reset(0);
  };

  ~Altimeter(){};

  bool start();
  bool isReady();
  void update();
  void reset();

  double altitude();            //meters above the reference altitude
  double referenceAltitude();   //Altitude when start() was called
  double verticalVelocity();    //in meters per second based on rate of barometric pressure change
  double pressure();

 private:
  Barometer barometer;
  bool barometerReady = false;
  double refAltitude  = 0;

  KalmanFilter altitudeFilter;
  LowPassFilter velocityFilter;
  
  double baselinePressure = 0;

  long lastRefreshTime        = 0;
  double lastRecordedAltitude = 0;
};

#endif
