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

#include "AttitudeControl.hpp"

#define kACGain 4.0f

void AttitudeControl::calibrate()
{
    //gravityVec = imu.getAcceleration();
    gravityVec = Vector(0,0,-10);
}

//The rocket should always travel upwards, which is direclty oppostie to the gravity Vector
//The z component of the gravity vector is not relevant, what we want to do is to keep the x and
//y components as close to zero as possible.  Any deviation from zero means the rocket is leaning
//in that direction.  We can calculate a correction factor (servo angle) based on the magnitude
//of the vector along that axis where 0 is straight up and 10.0mss is lying sideways

void AttitudeControl::start()
{
    running = true;
}

void AttitudeControl::stop()
{
    running = false;
    yawServo->write(kGimbalCenterAngle);
    pitchServo->write(kGimbalCenterAngle);
}

double gimbalClamp(double val)
{
    if(val < kMinGimbalOffset) return kMinGimbalOffset;
    if(val > kMaxGimbalOffset) return kMaxGimbalOffset;
    return val;
}

void AttitudeControl::update()
{
    Vector accVec = imu.getAcceleration();

    //Simple control where the gimbal angle is proportional to the gravity vector
    //component for the respective axis.

    float yaw = yawFilter.step(accVec.YAxis);
    float pitch = pitchFilter.step(accVec.XAxis);
    int pitchAngle = kGimbalCenterAngle + gimbalClamp( pitch * kACGain);
    int yawAngle = kGimbalCenterAngle + gimbalClamp( yaw * kACGain);

    if(running) {
        yawServo->write(yawAngle);
        pitchServo->write(pitchAngle);
    }
}