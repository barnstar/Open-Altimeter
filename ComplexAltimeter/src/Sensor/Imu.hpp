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

#ifndef IMU_H
#define IMU_H

#include <Wire.h>
#include "../../Configuration.h"
#include "../DataLogger.hpp"
#include "../types.h"
#include "lib/MadgwickAHRS.h"
#include "lib/MahonyAHRS.h"

#if USE_MPU9250
#include "lib/MPU9250.h"
typedef MPU9250 ImuSensor;
#endif

#if USE_MPU6050
#include "lib/MPU6050.h"
typedef MPU6050 ImuSensor;
#endif

typedef Madgwick SensorFusion;

class Imu
{
 public:
  #if USE_MPU9250
  Imu(int frequency) : imuSensor(Wire, IMU_I2C_ADDR), frequency(frequency) {}
  #endif
  #if USE_MPU6050
  Imu(int frequency) : imuSensor(), frequency(frequency) {}
  #endif

  ~Imu() {}

  bool start()
  {

  #if USE_MPU9250
    mpuReady = !(imuSensor.begin() < 0);
  #endif
  #if USE_MPU6050
    mpuReady = imuSensor.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G, IMU_I2C_ADDR);
  #endif
    DataLogger::log(mpuReady ? "IMU OK" : "IMU failed");
    return mpuReady;
  }

  void reset();
  void update();

  // Heading - roll, pitch, yaw
  Heading const &getHeading() { return heading; }

  // Linear accelleration
  Vector const &getAcceleration() { return acceleration; }

  // Rotational acceleration
  Vector const &getGyro() { return gyro; }

  // Reference heading on startup
  Heading const &getReferenceHeading() { return referenceHeading; }

  // Euler angles relative to our reference heading
  Heading getRelativeHeading();

 private:
  bool mpuReady;
  int frequency;
  Heading heading;
  Vector acceleration;
  Vector gyro;

  Heading referenceHeading;
  Vector gravityReference;

  ImuSensor imuSensor;
  Mahony sensorFusion;

  void calibrate();
};

#endif