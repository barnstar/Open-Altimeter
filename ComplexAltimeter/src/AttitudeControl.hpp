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

#ifndef ATT_CTL_H
#define ATT_CTL_H

#include "Sensor/Imu.hpp"
#include <Servo.h>
#include "Sensor/Filters.hpp"

#define kMaxGimbalOffset 20
#define kMinGimbalOffset -20

//This should be configurable
#define kGimbalCenterAngle 90

class AttitudeControl
{
    AttitudeControl(Imu imu) : imu(imu), pitchFilter(0), yawFilter(0) {
        pitchServo = new Servo();
        yawServo = new Servo();
        pitchServo->attach(PITCH_CONTROL_PIN);
        yawServo->attach(YAW_CONTROL_PIN);
        pitchServo->write(pitchServoCenterAngle);
        yawServo->write(yawServoCenterAngle);

        //Use slightly higher gain than the default for faster response here.
        pitchFilter.configure(1,1,0.05);
        yawFilter.configure(1,1,0.05);
        pitchFilter.reset(0);
        yawFilter.reset(0);
    };


    ~AttitudeControl() {
        delete pitchServo;
        delete yawServo;
    };

    void calibrate();
    void start();
    void stop();
    void update();

private:
    bool running = false;

    Imu &imu;
    Vector gravityVec;
    Vector accVec;

    KalmanFilter pitchFilter;
    KalmanFilter yawFilter;

    Servo *pitchServo;
    Servo *yawServo;

    int pitchServoCenterAngle = kGimbalCenterAngle;
    int yawServoCenterAngle = kGimbalCenterAngle;
};

#endif