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

#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

#define NO_PIN 0

void log(String msg);

typedef enum {
  kNoEjection,
  kPyro,
  kServo  
} DeploymentType;


typedef struct  {
  double apogee = 0;
  double ejectionAltitude = 0;
  double drogueEjectionAltitude = 0;
  double maxAcceleration = 0;
  double burnoutAltitude = 0;

  int    apogeeTime;
  int    accTriggerTime;
  int    altTriggerTime;   
} FlightData;


typedef struct {
  double altitude =0;
  double acceleration =0;
}SensorData;


typedef enum {
  kReadyToFly,
  kAscending,
  kDescending,
  kOnGround
} FlightState;


typedef enum {
  kNone,
  kActive,
  kPassive
}PeizoStyle;


typedef enum {
  OFF = 0,
  ON =1 
}OnOffState;


typedef OnOffState RelayState;
typedef OnOffState BlinkerState;

#endif //TYPES_H
