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

#include "Altimeter.hpp"
#include "../../Configuration.h"
#include "../DataLogger.hpp"

bool Altimeter::start()
{
  #if USE_BMP280
  bool ready = barometer.begin(BARO_I2C_ADDR);
  #endif
  #if USE_BMP085
  bool ready = barometer.begin();
  #endif

  if (ready) {
#ifdef STATUS_PIN_LEVEL
    analogWrite(STATUS_PIN, STATUS_PIN_LEVEL);
#else
    digitalWrite(STATUS_PIN, HIGH);
#endif
    digitalWrite(MESSAGE_PIN, LOW);
    DataLogger::log(F("Barometer Started"));
    barometerReady = true;
    reset();
  } else {
    // If the unit starts with the status pin off and the message pin on,
    // the barometer failed to initialize
    DataLogger::log(F("Barometer Init Fail"));
  }
  return ready;
}

void Altimeter::reset()
{
  altitudeFilter.reset(0);
  velocityFilter.reset(0);

  baselinePressure = pressure();
  #if USE_BMP280
  refAltitude = barometer.readAltitude(); 
  #endif
  lastRefreshTime  = 0;
  DataLogger::log(String(F("Barometer reset: ")) + String(baselinePressure));
}

double Altimeter::referenceAltitude() { return refAltitude; }

double Altimeter::altitude() { return altitudeFilter.getCurrentValue(); }

double Altimeter::verticalVelocity()
{
  return velocityFilter.getCurrentValue();
}

void Altimeter::update()
{
  #if USE_BMP085
  double p = pressure();
  if (p == 0) {
    return;
  }
  double relativeAlt = barometer.altitude(p, baselinePressure);
  #endif
  #if USE_BMP280
  double relativeAlt = barometer.readAltitude() - refAltitude;
  #endif

  double lastAlt = altitudeFilter.getCurrentValue();

  altitudeFilter.step(relativeAlt);

  long t = micros();
  if (lastRefreshTime) {
    long dt         = t - lastRefreshTime;
    double velocity = ((relativeAlt - lastAlt) / dt) * 1000000;
    velocityFilter.step(velocity);
  }
  lastRefreshTime = t;
}

double Altimeter::pressure()
{
  #if USE_BMP280
  return barometer.readPressure();
  #endif
  #if USE_BMP085
  char status;
  double t, p, p0, a;
  status = barometer.startTemperature();
  if (status != 0) {
    delay(status);
    status = barometer.getTemperature(t);
    if (status != 0) {
      status = barometer.startPressure(3);
      if (status != 0) {
        delay(status);
        status = barometer.getPressure(p, t);
        if (status != 0) {
          return (p);
        }
      }
    }
  }
  #endif
  return 0;
}

bool Altimeter::isReady() { return barometerReady; }
