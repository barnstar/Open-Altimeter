
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
 * 
 * 
 * Components Required:
 * 
 * An Arduino.. Nano preferably
 * A few LEDs and some current limiting resistors
 * A momentary switch
 * A BMP280 or BMP085 compatible altimeter (Adafruit works)
 * Optional: An active peizo buzzer
 * Optional: Relay breakout boards for dual deployment
 * A battery and power switch - 9V (lipo, alk, or nihm) or even 2 cr2025 button cells 
 * Wire/perfboard, etc to connect all this together
 * 
 * 
 * - Records apogee, deployment altitude for hundreds of flights (but who's kidding who,
 *   you'll lose this long before it makes it 100 flights)
 * - Trigger drogue depoloyment at apogee
 * - Triggers main deployment relay at a predefined altitude
 * - Plays an audible buzzer on landing to aid in locating
 * - Blinks out the last recorded apogee
 * - All flight data will be recored to EEPROM anddumped to the serial port 
 *   on startup.
 *  
 * For deployment, the depoloyment pins should be connected to a relay which
 * fires the deployment charge(s).
 * 
 * All signal PINs should be connected to LEDs via an 330ohm (or larger) resistor
 * The reset pin should be connected to ground via a momentary switch
 * 
 * This is designed for an Adafruit-compatible BMP280 board connected via
 * i2c.  On a nano that's Clock->A5 and Data->A4.  Other barometer sensors 
 * should work just as well.  Cheap MBP280s require you to add the 0x76 paramter
 * to correct the i2c addresss.
 * 
 * An optional MPU6050 can be attached which will recored the maximum accelleration
 * and a triggered acceleration initiation event time.
 * 
 * See comments on the various pin-outs for operation 
 *
 *********************************************************************************/
#define VERSION 3

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

#if ENABLE_MPU
#include "MPU9250.h"
typedef MPU9250 ImuSensor;
#endif

#include <Wire.h>

#include "types.h"
#include "SimpleTimer.h"
#include "KalmanFilter.h"
#include "RecoveryDevice.h"
#include "Blinker.hpp"
#include "MahonyAHRS.h"


void log(String msg)
{
#if LOG_TO_SERIAL
  Serial.println(msg);
#endif
}

//Prototypes
void playReadyTone();
bool isValid(FlightData *d);
void reset(FlightData *d);
void logData(int index, FlightData *d);
int readDeploymentAltitude();
void configureEeprom();
void readSensorData(SensorData *d);
void flightControl(SensorData *d);
bool checkResetPin();




/////////////////////////////////////////////////////////////////
// Global State

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
#if ENABLE_MPU
ImuSensor   imu(Wire,0x68);
#endif

KalmanFilter      filter;
Mahony            sensorFusion;



bool              barometerReady = false;        //True if the barometer/altimeter is ready
bool              mpuReady = false;              //True if the barometer/altimeter is ready
SimpleTimer       timer;
Blinker           blinker(timer, MESSAGE_PIN, BUZZER_PIN);


//////////////////////////////////////////////////////////////////
// main()

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  log("V " + String(VERSION));
  
  pinMode(RESET_PIN, INPUT_PULLUP);

  
  //All LED pins sset to outputs
  pinMode(MESSAGE_PIN, OUTPUT);
  pinMode(STATUS_PIN,  OUTPUT);
  pinMode(READY_PIN,   OUTPUT);
  pinMode(BUZZER_PIN,  OUTPUT);
  
  if(TEST_PIN) {
    pinMode(TEST_PIN, INPUT_PULLUP);
  }

  //Start in the "error" state.  Status pin should be high and message
  //pin should be low to indicate a good startup
  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(MESSAGE_PIN, HIGH);

  mainChute.init(2, MAIN_DEPL_RELAY_PIN, MAIN_TYPE);
  drogueChute.init(1, DROGUE_DEPL_RELAY_PIN, DROGUE_TYPE);

  if (barometer.begin(BARO_I2C_ADDR)) {  //Omit the parameter for adafruit
    #ifdef STATUS_PIN_LEVEL
    analogWrite(STATUS_PIN, STATUS_PIN_LEVEL);
    #else
    digitalWrite(STATUS_PIN, HIGH);
    #endif
    digitalWrite(MESSAGE_PIN, LOW);
    log("Baro Started");
    barometerReady = true;
  } else {
    log("Baro Fail");
  }

  mpuReady = !(imu.begin() < 0);
  log(mpuReady? "IMU OK" : "IMU failed");

#if ENABLE_MPU
  mpu.initialize();
  mpuReady = mpu.testConnection();
  log(mpuReady? "IMU OK" : "IMU failed");
  if(mpuReady) {
   mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
  }
#endif

  reset(&flightData);
  deploymentAltitude = readDeploymentAltitude();
  log("Depl Alt: " + String(deploymentAltitude));
  refAltitude = barometer.readAltitude(SEA_LEVEL_PRESSURE);
  log("Pad Alt:" + String(refAltitude));

  configureEeprom();
}

static unsigned long lastFireTime = 0;
int flightControlTimer = 0;
 SensorData data;

void loop()
{
  timer.run();
  if(!barometerReady && !blinker.isBlinking()) {
    blinker.blinkValue(3, BLINK_SPEED_MS, true);
  }
  if (flightState == kOnGround) {
      //Kill Timer
      timer.deleteTimer(flightControlTimer);
      flightControlTimer = -1;
      digitalWrite(READY_PIN, LOW);
      if(flightData.apogee && !blinker.isBlinking()) {
        blinker.blinkValue(flightData.apogee, BLINK_SPEED_MS, true);
      }
      checkResetPin();
    }
}


void flightControllInterrupt()
{
    if(flightState != kOnGround) {
      readSensorData(&data);
      flightControl(&data);
      digitalWrite(READY_PIN, HIGH); 
    }
}

int readDeploymentAltitude()
{
  pinMode(ALT_PIN_A, INPUT_PULLUP);
  pinMode(ALT_PIN_B, INPUT_PULLUP);

  int a = (digitalRead(ALT_PIN_A) == LOW) ? 1 : 0;
  int b = (digitalRead(ALT_PIN_B) == LOW) ? 1 : 0;

  static const int altitudes[] = { 100, 150, 150, 200 };

  int val = a + b*2;
  log("Alt Index:" + String(val) + String(a)+ String(b));
  if(val > 3)val = 0;
  return altitudes[val]; 
}


TimerProxy flightControlInterruptProxy(flightControllInterrupt);

bool checkResetPin()
{
  if (digitalRead(RESET_PIN) == LOW && flightState != kReadyToFly) {
    log("Resetting");
    blinker.cancelSequence();
    reset(&flightData); 
    resetTime = millis(); 
    setDeploymentRelay(OFF, &drogueChute); 
    drogueChute.reset(); 
    setDeploymentRelay(OFF, &mainChute); 
    mainChute.reset(); 
    testFlightTimeStep = 0;
    playReadyTone();
    flightState = kReadyToFly; 
    filter.reset(1,1,0.001);
    #if ENABLE_MPU
    sensorFusion.begin(1000/SENSOR_READ_DELAY_MS);
    #endif
    flightControlTimer = timer.setInterval(SENSOR_READ_DELAY_MS, &flightControlInterruptProxy);

    return true;
  }
  return false;
}


void playReadyTone()
{
  blinker.blinkValue(3, 100, false);
}



////////////////////////////////////////////////////////////////////////
// Sensors 
void readSensorData(SensorData *d)
{
  if(TEST_PIN && ((digitalRead(TEST_PIN) == LOW) || testFlightTimeStep)) {
    testFlightData(d);
    return;
  }
  
  //Our relative altitude... Relative to wherever we last reset the altimeter.
  if(barometerReady) {
    d->altitude = barometer.readAltitude(SEA_LEVEL_PRESSURE) - refAltitude;
  }
  if(mpuReady) {
    #if ENABLE_MPU
    imu.readSensor();
    sensorFusion.update(
      imu.getGyroX_rads(),
      imu.getGyroY_rads(),
      imu.getGyroZ_rads(),
      imu.getAccelX_mss(),
      imu.getAccelY_mss(),
      imu.getAccelZ_mss(),
      imu.getMagX_uT(),
      imu.getMagY_uT(),
      imu.getMagZ_uT());
  }
  #endif
}



////////////////////////////////////////////////////////////////////////
// Flight Control 

void flightControl(SensorData *d)
{
  double acceleration = d->acceleration;
  double altitude = filter.step(d->altitude);

  if(PLOT_ALTITUDE) { 
    log(String(altitude)); 
    log(String(sensorFusion.getYaw()) +":"+String(sensorFusion.getPitch()) +":" + String(sensorFusion.getRoll()));
   }

  //Failsafe.. Nothing should happen while we're ready but the altitude is below our threshold
  if(flightState == kReadyToFly && altitude < FLIGHT_START_THRESHOLD_ALT){
    return;
  }
  
  //Keep track or our apogee and our max g load
  if( altitude > flightData.apogee ){
    flightData.apogee = altitude;
    flightData.apogeeTime = millis() - resetTime;
  }
  flightData.maxAcceleration = acceleration > flightData.maxAcceleration ? acceleration : flightData.maxAcceleration;
  
  //Experimental.  Log when we've hit some prescribed g load.  This might be more accurate than starting the
  //flight at some altitude 
  if(flightState == kReadyToFly && acceleration > FLIGHT_START_THRESHOLD_ACC && flightData.accTriggerTime == 0) {
    flightData.accTriggerTime = millis() - resetTime;
  }

  if (flightState == kReadyToFly && altitude >= FLIGHT_START_THRESHOLD_ALT) {
    //Transition to "InFlight" if we've exceeded the threshold altitude.
    log("Flight Started");
    flightState = kAscending;
    flightData.altTriggerTime = millis() - resetTime;
    //For testing - to indicate we're in the ascending mode
    digitalWrite(READY_PIN, LOW);
    digitalWrite(MESSAGE_PIN, HIGH);
  } 
  else if (flightState == kAscending && altitude < (flightData.apogee - DESCENT_THRESHOLD)) {
    //Transition to kDescendining if we've we're DESCENT_THRESHOLD meters below our apogee
    log("Desc");
    flightState = kDescending;

    //Deploy our drogue chute
    setDeploymentRelay(ON, &drogueChute);
    flightData.drogueEjectionAltitude = altitude;
  }
  else if (flightState == kDescending && altitude < FLIGHT_END_THRESHOLD_ALT) {
    flightState = kOnGround;
    log(String("Land"));
    
    logData(flightCount, &flightData);
    recordFlight(flightData);
    flightCount++;

    //Reset the pyro charges.  Leave chute releases open.  Start the locator
    //beeper and start blinking...
    resetChuteIfRequired(&drogueChute);
    resetChuteIfRequired(&mainChute);
  }

  //Main chute deployment at kDeployment Altitude.  
  //We deploy in the onGround state as well just in case an anomalous pressure spike has transitioned
  //us to that state while we're still in the air
  if ((flightState == kDescending  || flightState == kOnGround) && !mainChute.deployed && altitude <= deploymentAltitude) {
    //If we're descening and we're below our deployment altitude, deploy the chute!
    flightData.ejectionAltitude = altitude;
    setDeploymentRelay(ON, &mainChute);
  }
  
  //Safety measure in case we don't switch to the onGround state.  This will disable the igniter relay
  //after 5 seconds to avoid draining or damaging the battery.  This only applies for pyro type.
  checkChuteIgnitionTimeout(&mainChute, MAX_FIRE_TIME);
  checkChuteIgnitionTimeout(&drogueChute, MAX_FIRE_TIME);
}

void resetChuteIfRequired(RecoveryDevice *c)
{
  if(c->type == kPyro) {
     setDeploymentRelay(OFF, c);
     c->reset();
  }
}

void checkChuteIgnitionTimeout(RecoveryDevice *c, int maxIgnitionTime)
{
    if (!c->timedReset && c->deployed &&
        millis() - c->deploymentTime > maxIgnitionTime &&
        c->type == kPyro) 
    {
      int chuteId = c->id;
      setDeploymentRelay(OFF, c);
      c->timedReset = true;
    }
}


void setDeploymentRelay(RelayState relayState, RecoveryDevice *c)
{
  if(relayState == c->relayState)return;
  int chuteId = c->id;
  
   switch(relayState) {
    case ON:
      c->enable();
      break;
    case OFF:
      c->disable();
      break;
  }
}


////////////////////////////////////////////////////////////////////////
//EEPROM & Peristance

void recordFlight(FlightData d)
{
  int offset = flightCount * sizeof(FlightData);
  EEPROM.put(offset, d);
}


void configureEeprom()
{
  if (digitalRead(RESET_PIN) == LOW) {
    for (int i = 0 ; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0);
    }
    log("EEProm Wiped");
  }
  flightCount = getFlightCount();
}


int getFlightCount()
{
  size_t maxProgs = EEPROM.length() / sizeof(FlightData);
  FlightData d;
  for (int i = 0; i < maxProgs; i++) {
    EEPROM.get(i * sizeof(FlightData), d);
    if (!isValid(&d)) {
      return i;
    }  
    logData(i, &d);
    flightData = d;
  }
  return 0;
}


////////////////////////////////////////////////////////////////////////
//Flight Data Utilities

bool isValid(FlightData *d) {
  return d->apogee || 
         d->ejectionAltitude ||
         d->drogueEjectionAltitude || 
         d->maxAcceleration ||
         d->burnoutAltitude;
}

void reset(FlightData *d) {
  d->apogee = 0;
  d->ejectionAltitude = 0;
  d->drogueEjectionAltitude = 0;
  d->maxAcceleration = 0;
  d->burnoutAltitude = 0; 
  d->accTriggerTime = 0;
  d->altTriggerTime = 0;
  d->apogeeTime = 0;

}

void logData(int index, FlightData *d) {
   log("# " + String(index) + 
       ": Apo " + String(d->apogee) + 
       "m:Main " + String(d->ejectionAltitude));
//       "m:Dro " + String(d->drogueEjectionAltitude) + 
//       "m:Acc " + String(d->maxAcceleration) + 
//       "ms:ApT " + String(d->apogeeTime) +
//       "ms:AcT" + String(d->accTriggerTime) + 
//       "ms:AltTT " + String(d->altTriggerTime) + "s");
}


///////////////////////////////////////////////////////////////////
// Test Flight Generator
// When you ground the TEST_PIN, the unit will initate a test flight
// 

SensorData fakeData;
double testApogee = 400;
bool isTestAscending;

void testFlightData(SensorData *d)
{
  if(0 == TEST_PIN)return;
  
  if (testFlightTimeStep == 0) {
    testFlightTimeStep = 1;
    fakeData.altitude = 0;
    fakeData.acceleration = 4;
    d->altitude = fakeData.altitude;
    d->acceleration = fakeData.acceleration;
    isTestAscending = true;
    return;
  }

  if(fakeData.altitude > testApogee) {
    isTestAscending = false;
  }

  double increment = isTestAscending ? 5.0 : -2.0;
  fakeData.altitude += increment;
 
  testFlightTimeStep++;

  d->altitude = fakeData.altitude;
  d->acceleration = fakeData.acceleration;
}




