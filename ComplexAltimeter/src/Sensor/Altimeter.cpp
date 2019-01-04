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
  bool ready = false;
  if (ready = barometer.begin()) {  
#ifdef STATUS_PIN_LEVEL
    analogWrite(STATUS_PIN, STATUS_PIN_LEVEL);
#else
    digitalWrite(STATUS_PIN, HIGH);
#endif
    digitalWrite(MESSAGE_PIN, LOW);
    DataLogger::log("Barometer Started");
    barometerReady = true;
    reset();
  } else {
    // If the unit starts with the status pin off and the message pin on,
    // the barometer failed to initialize
    DataLogger::log("Barometer Init Fail");
  }
  return ready;
}

void Altimeter::reset()
{
  filter.reset(1, 1, 0.001);
  velocityFilter.reset(1, 1, 0.001);
  for (int i = 0; i < 4; i++) {
    filter.step(0);
    velocityFilter.step(0);
  }

  baselinePressure = pressure();
  lastRefreshTime  = 0;
  DataLogger::log(String("Barometer reset: ") + String(baselinePressure));
}

double Altimeter::referenceAltitude() { return refAltitude; }

double Altimeter::altitude() { return filter.lastEstimate(); }

double Altimeter::verticalVelocity() { velocityFilter.lastEstimate(); }

void Altimeter::update()
{
  double p = pressure();
  if (p == 0) {
    return;
  }
  double relativeAlt = barometer.altitude(p, baselinePressure);

  long time = millis();
  if (lastRefreshTime) {
    long delta      = time - lastRefreshTime;
    double velocity = (relativeAlt - filter.lastEstimate()) / delta * 1000;
    velocityFilter.step(velocity);
  }
  lastRefreshTime = time;

  filter.step(relativeAlt);
}

double Altimeter::pressure()
{
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
  return 0;
}

bool Altimeter::isReady() { return barometerReady; }


