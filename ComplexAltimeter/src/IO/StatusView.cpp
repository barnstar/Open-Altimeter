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

#include "StatusView.hpp"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "FlightController.hpp"

void StatusView::refresh()
{
  // This will only change after a complete flight.  Do it once... If we're on
  // the ground
  if (FlightController::shared().flightState == kOnGround && needsRefresh) {
    setInfo(FlightController::shared().getStatusData());
    needsRefresh = false;
  } else {
    // Update once we're on the ground
    needsRefresh = true;
  }
}

void StatusView::setInfo(StatusData const &data)
{
  setText(F("==:::: Status ::::=="), 0, false);
  setText(flightStateString(data.status), 1, false);
  String sensorStatus =
      (data.baroReady ? String("Baro OK") : String("Baro Fail")) + String(":") +
      (data.mpuReady ? String("IMU OK") : String("IMU Fail"));
  setText(sensorStatus, 2, false);
  setText((String("Depl:") + String(data.deploymentAlt) + String("m")), 3,
          false);
  setText((String("Last:") + String(data.deploymentAlt) + String("m")), 4,
          false);
  setText(WiFi.localIP().toString(), 5, false);
  update();
}
