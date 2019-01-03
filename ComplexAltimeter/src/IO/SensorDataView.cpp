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

void SensorDataView::setData(SensorData &data)
{
    String heading = data.heading.toString();
    String dataStr = data.toString();
    String accStr  = data.acc_vec.toString();
    String gyroStr = data.gyro_vec.toString();
    setText(gyroStr, 3, false);
    setText(dataStr, 0, false);
    setText(heading, 1, false);
    setText(accStr, 2, false);
    setText(gyroStr, 3, false);
    update();
}

void SensorDataView::setWaiting()
{
    setText(F("No telemetry"),0,false);
    setText(F("Unit in landed mode"),1,false);
    setText(F(""),2,false);
    setText(F(""),3,false);
    setText(F(""),4,false);
    update();
}
