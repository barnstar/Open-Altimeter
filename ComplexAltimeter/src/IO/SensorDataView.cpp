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

#include "SensorDataView.hpp"
#include "FlightController.hpp"

void SensorDataView::refresh()
{
  if (FlightController::shared().flightState == kOnGround && needsRefresh) {
    setWaiting();
    //If we're in the waiting mode, then the image is static.
    needsRefresh = false;
  } else {  //flying or ready to fly
    needsRefresh = true;
    setData(FlightController::shared().flightData);
  }
}

void SensorDataView::setData(SensorData &data)
{
  setText(data.toString(), 0,  false);           // acceleration, vertical velocity and altitude
  setText(data.heading.toString(), 1, false);    // roll pitch yaw
  setText(data.acc_vec.toString(, 2, false);     // raw accelerometer values
  setText(data.gyro_vec.toString(), 3, false);   // raw gyro values 
  update();
}

void SensorDataView::setWaiting()
{
  setText(F("No telemetry"), 0, false);
  setText(F("Unit in landed mode"), 1, false);
  setText(F(""), 2, false);
  setText(F(""), 3, false);
  update();
}
