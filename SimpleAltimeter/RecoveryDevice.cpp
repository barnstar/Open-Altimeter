
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


#include "RecoveryDevice.h"
#ifdef IS_SIMPLE_ALT
#include "Configuration.h"
#else
#include "../Configuration.h"
#endif
#include "types.h"

void RecoveryDevice::init(byte id, byte pin, RecoveryDeviceType type)
{
  //log("Init RD " + String(id) + " p:" + String(pin));
  this->gpioPin = pin;
  this->id       = id;
  this->type     = type;
  switch (type) {
    case kPyro:
      pinMode(pin, OUTPUT);
      break;
    case kServo:
      servo.attach(pin);
      break;
    case kNoEjection:
      break;
  }
  reset();
};

void RecoveryDevice::enable()
{
  deployed       = true;
  deploymentTime = millis();
  deviceState     = ON;
  switch (type) {
    case kPyro:
      digitalWrite(gpioPin, HIGH);
      break;
    case kServo:
      servo.write(kChuteReleaseTriggeredAngle);
      break;
    case kNoEjection:
      break;
  }
  Serial.println("RD En" + String(id) );
};

void RecoveryDevice::disable()
{
  deployed   = false;
  deviceState = OFF;
  switch (type) {
    case kPyro:
      digitalWrite(gpioPin, LOW);
      break;
    case kServo:
      servo.write(kChuteReleaseArmedAngle);
      break;
    case kNoEjection:
      break;
  }
  Serial.println("RD Dis" + String(id) );
};

void RecoveryDevice::reset()
{
  deployed       = false;
  deploymentTime = 0;
  timedReset     = false;
  disable();
};
