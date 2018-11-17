#ifndef IMU_H
#define IMU_H

#include "MPU9250.h"
#include "MahonyAHRS.h"
#include <Wire.h>

typedef MPU9250 ImuSensor;
typedef Mahony SensorFusion;

struct Heading
{
	double roll;
	double pitch;
	double yaw;
};

struct Vec3
{
	double x;
	double y;
	double z;

	double len() {
		return sqrt(x*x + y*y + z*z);
	}
};

class Imu
{
public:
	Imu(int frequency) {
	    //imuSenstor = new ImuSensor(Wire,0x68);
  		//mpuReady = !(imuSensor.begin() < 0);
  		//log(mpuReady? "IMU OK" : "IMU failed");
  		//imuSensor.frequency = frequency;
	}

	~Imu();

	Heading const& getHeading() {
		return heading;
	}

	Vec3 const& getAccelleration() {
		return accelleration;
	}


private:
	bool mpuReady;
	int frequency;
	Heading heading;
	Vec3 accelleration;

	//ImuSensor         imuSensor(Wire,0x68);
	Mahony            sensorFusion;

};

#endif