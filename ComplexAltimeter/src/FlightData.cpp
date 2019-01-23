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

#include "FlightData.hpp"

const bool FlightData::isValid()
{
  return apogee || ejectionAltitude || drogueEjectionAltitude ||
         maxAcceleration || burnoutAltitude;
}

const String FlightData::toString(int index)
{
  return String("index : " + String(index) + "," +
                "apogee:" + String(apogee) + "," +
                "main_alt : " + String(ejectionAltitude) + "," +
                "drogue_alt : " + String(drogueEjectionAltitude) + "," +
                "max_acc : " + String(maxAcceleration) + "," +
                "apogee_time : " + String(apogeeTime) + "," +
                "burnout_alt : " + String(burnoutAltitude) + "," +
                "burnout_time : " + String(burnoutTime) + "," +
                "acc_trigger_time :" + String(accTriggerTime) + "," +
                "alt_trigger_time :  " + String(altTriggerTime));
}

void FlightData::reset()
{
  apogee                 = 0;
  ejectionAltitude       = 0;
  drogueEjectionAltitude = 0;
  maxAcceleration        = 0;
  burnoutAltitude        = 0;
  accTriggerTime         = 0;
  altTriggerTime         = 0;
  apogeeTime             = 0;
  burnoutTime            = 0;
}
