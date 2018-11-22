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

#ifndef types_h
#define types_h

#include <Servo.h>
#include "DataLogger.hpp"
#include "FlightData.hpp"

#define NO_PIN -1

typedef enum { kNoEjection, kPyro, kServo } DeploymentType;

typedef enum { OFF = 0, ON = 1 } OnOffState;

typedef OnOffState RelayState;
typedef OnOffState BlinkerState;

struct Vector {
  float XAxis = 0;
  float YAxis = 0;
  float ZAxis = 0;

  Vector() {}

  Vector(float x, float y, float z)
  {
    XAxis = x;
    YAxis = y;
    ZAxis = z;
  }

  double length()
  {
    return sqrt((XAxis * XAxis) + (YAxis * YAxis) + (ZAxis * ZAxis));
  }

  Vector operator+(Vector rhs)
  {
    return Vector(XAxis + rhs.XAxis, YAxis + rhs.YAxis, ZAxis + rhs.ZAxis);
  }

  Vector operator/(float rhs)
  {
    return Vector(XAxis / rhs, YAxis / rhs, ZAxis / rhs);
  }

  Vector operator*(float rhs)
  {
    return Vector(XAxis * rhs, YAxis * rhs, ZAxis * rhs);
  }

  String toString()
  {
    return String("(" + String(XAxis) + "," + String(YAxis) + "," +
                  String(ZAxis) + ")");
  }
};

typedef struct {
  double roll;
  double pitch;
  double yaw;

  String toString()
  {
    return String("R: " + String(roll) + "  " + "P:" + String(pitch) + "  " +
                  "Y:" + String(yaw) + "  ");
  }
} Heading;

typedef struct {
  double altitude     = 0;
  double acceleration = 0;
  Vector acc_vec;
  Heading heading;

  String toString()
  {
    return String("Alt " + String(altitude) +
                  "    acc: " + String(acceleration));
  }
} SensorData;

typedef enum {
  kReadyToFly,
  kInFlight,
  kAscending,
  kDescending,
  kOnGround
} FlightState;

typedef enum { kNone, kActive, kPassive } PeizoStyle;

#endif  // types_h
