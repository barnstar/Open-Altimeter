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

#ifndef RECOVERYDEVICE_H
#define RECOVERYDEVICE_H

#include <Servo.h>

#ifdef IS_SIMPLE_ALT
#include "Configuration.h"
#else
#include "../Configuration.h"
#include "Settings.hpp"
#endif



class RecoveryDevice
{
 public:
  RecoveryDevice() {};
  ~RecoveryDevice(){};

  static int offAngle;
  static int onAngle;  

  static void setOnAngle(int angle, bool save)
  {
    RecoveryDevice::onAngle = angle;
    DataLogger::log("On Angle set to " + String(angle));
#ifndef IS_SIMPLE_ALT
    if (save) {
      Settings s;
      s.writeIntValue(angle, "servoOnAngle");
    }
#endif
  }

  static void setOffAngle(int angle, bool save)
  {
    RecoveryDevice::offAngle = angle;
    DataLogger::log("Off Angle set to " + String(angle));
#ifndef IS_SIMPLE_ALT
    if (save) {
      Settings s;
      s.writeIntValue(angle, "servoOffAngle");
    }
#endif
  }

  bool deployed      = false;  // True if the the chute has been deplyed
  int deploymentTime = 0;      // Time at which the chute was deployed
  bool timedReset    = false;
  RecoveryDeviceState deviceState = OFF;
  RecoveryDeviceType type         = kServo;

 public:
  void init(byte id, byte gpioPin, RecoveryDeviceType type);
  void enable();
  void disable();
  void reset();
  byte id      = 0;
  byte gpioPin = 0;

 private:
  Servo *servo = nullptr;

  void setServoAngle(int angle);
};

#endif  // RECOVERYDEVICE_H
