
#include "Configuration.h"
#include <EEPROM.h>
#include <Servo.h>

//Sensor libraries
#if USE_BMP280
#include <Adafruit_BMP280.h>             //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h       
typedef  Adafruit_BMP280  Barometer;
const double SEA_LEVEL_PRESSURE = 1013.7;
#endif

#if USE_BMP085
#include <Adafruit_BMP085.h>             //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP085.h       
typedef  Adafruit_BMP085 Barometer ;
const double SEA_LEVEL_PRESSURE = 101370;
#endif

//#include "I2Cdev.h"                      //https://diyhacking.com/arduino-mpu-6050-imu-sensor-tutorial/
//#include "MPU6050_6Axis_MotionApps20.h"  //https://diyhacking.com/arduino-mpu-6050-imu-sensor-tutorial/ 
#include <Wire.h>

#include "types.h"
#include "SimpleTimer.h"
#include "KalmanFilter.h"
#include "RecoveryDevice.h"
#include "Blinker.hpp"


class FlightControl
{
public:
	FlightControl();
	~FlightControl() {}

	FlightData flightData;
	FlightState flightState = kOnGround;  //The flight state

	RecoveryDevice mainChute;
	RecoveryDevice drogueChute;

	double refAltitude = 0;               //The reference altitude (altitude of the launch pad)
	int    flightCount = 0;               //The number of flights recorded in EEPROM
	int    resetTime = 0;                 //millis() after starting the current flight
	double deploymentAltitude = 100;      //Deployment altitude in m.
	int    testFlightTimeStep = 0;

	Barometer   barometer;
	SensorData  sensorData;

	bool              barometerReady = false;        //True if the barometer/altimeter is ready
	bool              mpuReady = false;              //True if the barometer/altimeter is ready
	KalmanFilter      filter;

	double apogee();
	

	int  readDeploymentAltitude()
	void readSensorData(SensorData &d)
	void flightControl(SensorData &d)
	void resetChuteIfRequired(RecoveryDevice &c)
	void setDeploymentRelay(RelayState relayState, RecoveryDevice &c)
	void readSensorData(SensorData &d)

}