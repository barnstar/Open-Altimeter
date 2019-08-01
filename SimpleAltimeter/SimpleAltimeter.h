#ifndef SIMPLE_ALT_H
#define SIMPLE_ALT_H


#include <EEPROM.h>
#include <Servo.h>
#include "Configuration.h"

// Sensor libraries
#if USE_BMP280
#include "Adafruit_BMP280.h"  //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h
typedef Adafruit_BMP280 Barometer;
const double SEA_LEVEL_PRESSURE = 1013.7;
#endif

#if USE_BMP085
#include "Adafruit_BMP085.h"  //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP085.h
typedef Adafruit_BMP085 Barometer;
const double SEA_LEVEL_PRESSURE = 101370;
#endif

#if ENABLE_MPU
#include "MPU9250.h"
#include "MahonyAHRS.h"
typedef MPU9250 ImuSensor;
#endif

#include "Filters.hpp"
#include "SimpleTimer.h"

#include "Blinker.hpp"
#include "RecoveryDevice.h"
#include "types.h"

void playReadyTone();
void reset(FlightData *d);
bool checkResetPin();
int  readDeploymentAltitude();

void flightControllInterrupt();
void readSensorData(SensorData *d);
void flightControl(SensorData *d);

void setRecoveryDeviceState(RecoveryDeviceState deviceState, RecoveryDevice *c);

void testFlightData(SensorData *d);

void logData(int index, FlightData *d);
bool isValid(FlightData *d);
void configureEeprom();


#endif