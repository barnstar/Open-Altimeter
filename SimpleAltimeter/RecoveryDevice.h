/*********************************************************************************
 * Simple Alitimeter 
 * 
 * Mid power rocket avionics software for alitidue recording and dual deployment
 * 
 * Copyright 2018, Jonathan Nobels
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of 
 * this software and associated documentation files (the "Software"), to deal in the 
 * Software without restriction, including without limitation the rights to use, copy, 
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the 
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **********************************************************************************/

#ifndef RECOVERYDEVICE_H
#define RECOVERYDEVICE_H

#include "types.h"
#include <Servo.h>

class RecoveryDevice
{

public:
  RecoveryDevice() {
    this->reset();
  };
  
  ~RecoveryDevice() {};
  
  bool           deployed = false;         //True if the the chute has been deplyed
  int            deploymentTime = 0;   //Time at which the chute was deployed
  bool           timedReset = false;       //True if we've reset the chute relay due to a timeout
  RelayState     relayState = OFF;  //State of the parachute relay pin.  Recorded separately.  To avoid fire.
  DeploymentType type = kServo;
  Servo          servo;

  int    relayPin=0;
  char   id = 0; 

public:
  void init(int id, int pin, DeploymentType type);
  void enable();
  void disable();
  void reset();
  
};

#endif //RECOVERYDEVICE_H
