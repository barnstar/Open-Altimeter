#ifndef IMU_H
#define IMU_H

#include "../DataLogger.hpp"
#include "../types.h"
#include "MPU9250.h"
#include "MahonyAHRS.h"
#include <Wire.h>

typedef MPU9250 ImuSensor;
typedef Mahony SensorFusion;

class Imu
{
public:
  Imu(int frequency) : imuSensor(Wire, 0x68), frequency(frequency * 0.5) {}

  ~Imu() {}

  void start()
  {
    mpuReady = !(imuSensor.begin() < 0);
    DataLogger::log(mpuReady ? "IMU OK" : "IMU failed");
  }

  void reset();
  void update();

  Heading getHeading() { return heading; }

  Vector const &getAccelleration() { return accelleration; }

private:
  bool mpuReady;
  int frequency;
  Heading heading;
  Vector accelleration;

  ImuSensor imuSensor;
  Mahony sensorFusion;
};

#endif