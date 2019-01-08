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

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef enum { kNoEjection, kPyro, kServo, kServoChannel } RecoveryDeviceType;

typedef enum { OFF = 0, ON = 1 } OnOffState;

typedef OnOffState RecoveryDeviceState;
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
    return String(String(XAxis) + ":" + String(YAxis) + ":" + String(ZAxis));
  }
};

struct Heading {
  Heading() {}

  Heading(float r, float p, float y)
  {
    roll  = r;
    pitch = p;
    yaw   = y;
  }

  double roll  = 0;
  double pitch = 0;
  double yaw   = 0;

  Heading operator-(Heading rhs)
  {
    return Heading(roll - rhs.roll, pitch - rhs.pitch, yaw - rhs.yaw);
  }

  String toString()
  {
    return String(String(roll) + ":" + String(pitch) + ":" + String(yaw));
  }
};

typedef struct {
  double altitude         = 0;
  double acceleration     = 0;
  double verticalVelocity = 0;
  Vector acc_vec;
  Vector gyro_vec;
  Heading heading;

  String toString()
  {
    return String("A:" + String(altitude) + " C:" + String(acceleration) +
                  " V:" + String(verticalVelocity));
  }
} SensorData;

typedef enum {
  kReadyToFly,
  kInFlight,
  kAscending,
  kDescending,
  kOnGround
} FlightState;

inline String flightStateString(FlightState s)
{
  switch (s) {
    case kReadyToFly:
      return String("Ready");
      break;
    case kInFlight:
      return String("In Flight");
      break;
    case kAscending:
      return String("Ascending");
      break;
    case kDescending:
      return String("Descending");
      break;
    case kOnGround:
      return String("On Ground");
      break;
  }
}

typedef enum { kNone, kActive, kPassive } PeizoStyle;

struct StatusData {
  uint8_t deploymentAlt;
  FlightState status;
  bool baroReady;
  bool mpuReady;

  double padAltitude;
  double lastApogee;
  double referencePressure;

  boolean isEqual(const StatusData &data)
  {
    return status == data.status && deploymentAlt == data.deploymentAlt &&
           lastApogee == data.lastApogee && baroReady == data.baroReady &&
           mpuReady == data.mpuReady;
  }
};

typedef enum { PrimaryButton = 1, SecondaryButton = 2 } ButtonId;

typedef enum {
  ControlChannel1 = 1,
  ControlChannel2,
  ControlChannel3,
  ControlChannel4
} ControlChannel;

#endif  // types_h
