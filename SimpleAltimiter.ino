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
 * A BMP-280 compatible altimeter (Adafruit works)
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
#define VERSION 2
 
#include <EEPROM.h>

//Sensor libraries
#include <Adafruit_BMP280.h>             //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h       
#include "I2Cdev.h"                      //https://diyhacking.com/arduino-mpu-6050-imu-sensor-tutorial/
#include "MPU6050_6Axis_MotionApps20.h"  //https://diyhacking.com/arduino-mpu-6050-imu-sensor-tutorial/ 

#define LOG_TO_SERIAL 1   //Set to 0 to disable serial logging...

typedef Adafruit_BMP280 Barometer;
void log(String val);

//Today's pressure at sea level...
const double SEA_LEVEL_PRESSURE = 1013.7;

//Recording will start at FLIGHT_START_THRESHOLD_ALT m and we'll assume we're on the 
//ground at FLIGHT_END_THRESHOLD_ALT m.
//In theory, these could be lower, but we want to account for landing in a tree,
//on a hill, etc.  30m should be sufficient for most launch sites.
const double FLIGHT_START_THRESHOLD_ALT = 10;
const double FLIGHT_END_THRESHOLD_ALT = 30;
const double FLIGHT_START_THRESHOLD_ACC = 0.1;  //in G's

//100m deployment altitude (330 ft) 
double deploymentAltitude = 100;

//When the altitude is DESCENT_THRESHOLD meters less than the apogee, we'll assume we're 
//descending.  Hopefully, your rocket has a generally upwards trajectory....
const double DESCENT_THRESHOLD = 20;

//The deployment relay will be deactivated after this time.
const int MAX_FIRE_TIME = 5000;

//When grounded the reset pin will cancel the last apogee display and
//prepare the alitmiter for the next flight.  If it is grounded on boot
//the eeprom will be erased.

#define USE_PIN_CONFIG_1 1
#define USE_PIN_CONFIG_2 0

#if USE_PIN_CONFIG_1
//Configuration A: 1 1/2" PCB
const int STATUS_PIN            = 4;   //Unit status pin.  On if OK
const int MESSAGE_PIN           = 2;   //Blinks out the altitude
const int READY_PIN             = 13;  //Inicates the unit is ready for flight
const int BUZZER_PIN            = 8;   //Audible buzzer on landing
const int RESET_PIN             = 6;
const int MAIN_DEPL_RELAY_PIN   =  9;  //parachute deployment pin
const int DROGUE_DEPL_RELAY_PIN =  10;  //parachute deployment pin
const int ALT_PIN_A             =  11;  //Altitude Set Pin.
const int ALT_PIN_B             =  12;  //Altitude Set Pin
#elif USE_PIN_CONFIG_2
const int STATUS_PIN            = 2;  //Unit status pin.  On if OK
const int MESSAGE_PIN           = 3;  //Blinks out the altitude
const int READY_PIN             = 4;  //Inicates the unit is ready for flight
const int BUZZER_PIN            = 7;  //Audible buzzer on landing
const int RESET_PIN             = 6;
const int MAIN_DEPL_RELAY_PIN   =  8;  //parachute deployment pin
const int DROGUE_DEPL_RELAY_PIN =  9;  //parachute deployment pin
const int ALT_PIN_A             =  10;  //Altitude Set Pin.
const int ALT_PIN_B             =  11;  //Altitude Set Pin
#endif


const int SERIAL_BAUD_RATE     = 9600;

//The barometer can only refresh at about 50Hz. 
const int SENSOR_READ_DELAY_MS = 20;

//Delay between digit blinks.  Any faster is too quick to keep up with
const int BLINK_SPEED_MS       = 200;

//FlightData parameters.
typedef struct  {
  double apogee = 0;
  double ejectionAltitude = 0;
  double drogueEjectionAltitude = 0;
  double maxAcceleration = 0;
  double burnoutAltitude = 0;

  int    accTriggerTime;
  int    altTriggerTime;   
} FlightData;

bool isValid(FlightData *d);
void reset(FlightData *d);
void logData(int index, FlightData *d);


typedef enum {
  OFF = 0,
  ON =1 
}RelayState;

typedef struct {
  bool         deployed ;         //True if the the chute has been deplyed
  int          deploymentTime ;   //Time at which the chute was deployed
  bool         timedReset ;       //True if we've reset the chute relay due to a timeout
  RelayState   relayState = OFF;  //State of the parachute relay pin.  Recorded separately.  To avoid fire.
  
  int    relayPin=0;
  char   id = 0; 
  
  void init(int id, int pin) {
      log("Init Chute " + String(id) + " on pin " + String(pin));
      this->relayPin = pin;
      this->id = id;
      pinMode(pin, OUTPUT);
      reset();
  }

  void reset() {
    deployed = false;
    deploymentTime = 0;
    timedReset = false;
    relayState = OFF;
    digitalWrite(relayPin, LOW);
  }
}ChuteState;


typedef struct {
  double altitude =0;
  double acceleration =0;
}SensorData;

typedef enum {
  kInFlight,
  kAscending,
  kDescending,
  kOnGround
} FlightState;

FlightData flightData;
FlightState flightState = kOnGround;  //The flight state

ChuteState mainChute;
ChuteState drogueChute;

double refAltitude = 0;         //The reference altitude (altitude of the launch pad)
int    flightCount = 0;         //The number of flights recorded in EEPROM

bool   barometerReady = false;        //True if the barometer/altimeter is ready
bool   mpuReady = false;              //True if the barometer/altimeter is ready

int    resetTime = 0;                 //millis() after starting
bool   readyToFly = false;            //switches to false at the end of the flight.  Resets on reset.
bool   enableBuzzer = false;          //True if the buzzer should be sounding

//The Sensors
Barometer   barometer;
MPU6050     mpu;


void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  log("Initializing. Version " + String(VERSION));
  
  pinMode(RESET_PIN, INPUT_PULLUP);

  //All LED pins sset to outputs
  pinMode(MESSAGE_PIN, OUTPUT);
  pinMode(STATUS_PIN,  OUTPUT);
  pinMode(READY_PIN,   OUTPUT);
  pinMode(BUZZER_PIN,  OUTPUT);

  //Start in the "error" state.  Status pin should be high and message
  //pin should be low to indicate a good startup
  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(MESSAGE_PIN, HIGH);

  mainChute.init(2, MAIN_DEPL_RELAY_PIN);
  drogueChute.init(1, DROGUE_DEPL_RELAY_PIN);

  delay(100);   //The barometer doesn't like being queried immediately
  if (barometer.begin()) {  //Omit the parameter for adafruit
    digitalWrite(STATUS_PIN, HIGH);
    digitalWrite(MESSAGE_PIN, LOW);
    log("Barometer Started");
    barometerReady = true;
  } else {
    //If the unit starts with the status pin off and the message pin on,
    //the barometer failed to initialize
    log("Barometer Init Fail");
  }

  mpu.initialize();
  mpuReady = mpu.testConnection();
  log(mpuReady? "MPU6050 connection successful" : "MPU6050 connection failed");
  if(mpuReady) {
    //Don't really care if it's not ready or not present
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
  }

  reset(&flightData);
  deploymentAltitude = readDeploymentAltitude();
  log("Deployment Altitude: " + String(deploymentAltitude));
  refAltitude = barometer.readAltitude(SEA_LEVEL_PRESSURE);
  log("Pad Altitude:" + String(refAltitude));

  configureEeprom();

  //We don't want to sound the buzzer on first boot, only after a flight
  enableBuzzer = false;
}

int readDeploymentAltitude()
{
  pinMode(ALT_PIN_A, INPUT_PULLUP);
  pinMode(ALT_PIN_B, INPUT_PULLUP);

  int a = (digitalRead(RESET_PIN) == LOW) ? 1 : 0;
  int b = (digitalRead(RESET_PIN) == LOW) ? 1 : 0;

  static const int altitudes[] = { 100, 70, 150, 200 };

  int val = a & b<<1;
  if(val > 3)val = 0;
  return altitudes[val]; 
}


void loop()
{
  static SensorData data;
  if(readyToFly && barometerReady) {
    readSensorData(&data);
    flightControl(&data);
    delay(SENSOR_READ_DELAY_MS);
    digitalWrite(READY_PIN, HIGH);
  }

  //Blink out the last recorded apogee on the message pin
  if (!readyToFly && flightState == kOnGround) {
        digitalWrite(READY_PIN, LOW);
        blinkLastAltitude();
  }
}

//If the rest pin is hit while we're blinking out an altitude,
//reset the previous apogee and the buzzer state and get ready
//for the next flight
#define CHECK_RESETPIN_AND_RETURN() \
   do{ \
      if (digitalRead(RESET_PIN) == LOW) {\
        reset(&flightData); \
        resetTime = millis(); \
        enableBuzzer = false;\
        readyToFly = true; \
        return;\
      }\
   }while(0); \



void blinkWithDelay(int onTime, int offTime)
{
  digitalWrite(MESSAGE_PIN, HIGH); 
  if(enableBuzzer)digitalWrite(BUZZER_PIN, HIGH); 
  delay(onTime);
  digitalWrite(MESSAGE_PIN, LOW);
  if(enableBuzzer)digitalWrite(BUZZER_PIN, LOW); 
  delay(offTime);
}


//Not particularily pretty... We could use timers or interrupts here
//but it's good enough.  You'll need to hold down the reset button for
//a couple of seconds at worst.  
void blinkLastAltitude()
{
  int tempApogee = flightData.apogee;
  bool foundDigit = false;         //Don't blink leading 0s
  for(int m=10000; m>0; m=m/10) {  //If we make it past 99km, we're in trouble :)
    int digit = tempApogee / m;
    if (digit || foundDigit){
      foundDigit = true;
      for (int i = 0; i < (digit ?: 10); i++) {
        blinkWithDelay(BLINK_SPEED_MS, BLINK_SPEED_MS);
        CHECK_RESETPIN_AND_RETURN();
      }
      delay(BLINK_SPEED_MS * 2);
      tempApogee = tempApogee - digit * m;
    }
  }
  CHECK_RESETPIN_AND_RETURN();
  blinkWithDelay(BLINK_SPEED_MS * 10, BLINK_SPEED_MS);
  CHECK_RESETPIN_AND_RETURN();
}



////////////////////////////////////////////////////////////////////////
// Sensors 

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t grx, gry, grz;
const double onegee = 65536.0 / 16.0; //units/g for scale of +/- 8g

double getacceleration()
{
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  double axd = ax / onegee ;
  double ayd = ay / onegee ;
  double azd = az / onegee ;

  //Remove gravity.. We now have the absolute acceleration
  double acc = sqrt( axd * axd + ayd * ayd + azd * azd) - 1.0;
  return acc;
}

void readSensorData(SensorData *d)
{
  //Our relative altitude... Relative to wherever we last reset the altimeter.
  if(barometerReady) {
    d->altitude = barometer.readAltitude(SEA_LEVEL_PRESSURE) - refAltitude;
  }
  if(mpuReady) {
    d->acceleration = getacceleration();
  }
}



////////////////////////////////////////////////////////////////////////
// Flight Control 

void flightControl(SensorData *d)
{
  double acceleration = d->acceleration;
  double altitude = d->altitude;
  
  //Keep track or our apogee and our max g load
  flightData.apogee = altitude > flightData.apogee ? altitude : flightData.apogee;
  flightData.maxAcceleration = acceleration > flightData.maxAcceleration ? acceleration : flightData.maxAcceleration;
  
  //Experimental.  Log when we've hit some prescribed g load.  This might be more accurate than starting the
  //flight at some altitude x...  The minimum load will probably have to be too small and will pick up things like
  //wind gusts though.
  if(flightState == kOnGround && acceleration > FLIGHT_START_THRESHOLD_ACC && flightData.accTriggerTime == 0) {
    flightData.accTriggerTime = millis() - resetTime;
  }

  if (flightState == kOnGround && altitude > FLIGHT_START_THRESHOLD_ALT) {
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
    log("Descending");
    flightState = kDescending;

    //Deploy our drogue chute
    setDeploymentRelay(ON, &drogueChute);
    flightData.drogueEjectionAltitude = altitude;
  }
  else if (flightState == kDescending && altitude < FLIGHT_END_THRESHOLD_ALT) {
    flightState = kOnGround;
    log(String("Landed"));
    
    logData(flightCount, &flightData);
    recordFlight(flightData);
    flightCount++;

    //Reset the chutes, reset the relays if we haven't already.  Start the locator
    //beeper and start blinking...
    setDeploymentRelay(OFF, &drogueChute);
    setDeploymentRelay(OFF, &mainChute);
    mainChute.reset();
    drogueChute.reset();
    
    enableBuzzer = true;
    readyToFly = false;
  }

  //Main chute deployment at kDeployment Altitude
  if (flightState == kDescending && !mainChute.deployed && altitude < deploymentAltitude) {
    //If we're descening and we're below our deployment altitude, deploy the chute!
    flightData.ejectionAltitude = altitude;
    setDeploymentRelay(ON, &mainChute);
  }
  
  //Safety measure in case we don't switch to the onGround state.  This will disable the igniter relay
  //after 5 seconds to avoid draining or damaging the battery
  checkChuteIgnitionTimeout(&mainChute, MAX_FIRE_TIME);
  checkChuteIgnitionTimeout(&drogueChute, MAX_FIRE_TIME);
}


void checkChuteIgnitionTimeout(ChuteState *c, int maxIgnitionTime)
{
    if (flightState == kDescending) {
      return;
    }

    if (!c->timedReset && c->deployed &&
        millis() - c->deploymentTime > maxIgnitionTime) 
    {
      int chuteId = c->id;
      log("Chute #" + String(chuteId) + " Timeout");
      setDeploymentRelay(OFF, c);
      c->timedReset = true;
    }
}


void setDeploymentRelay(RelayState relayState, ChuteState *c)
{
  if(relayState == c->relayState)return;
  int chuteId = c->id;
  
   switch(relayState) {
    case ON:
      log("Deploying Chute #" + String(chuteId) );
      digitalWrite(c->relayPin, HIGH);
      c->deployed = true;
      c->deploymentTime = millis();
      break;
    case OFF:
      log("Resetting Chute #" + String(chuteId));
      digitalWrite(c->relayPin, LOW);
      c->deployed = false;
      break;
  }
  c->relayState = relayState;
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
  log("Reading Flights");
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
  log("EEPROM Full.  That's probably an error.  Reset your EEPROM");
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
}

void logData(int index, FlightData *d) {
   log("Flight " + String(index) + 
       ": Apogee " + String(d->apogee) + 
       "m : Main " + String(d->ejectionAltitude) + 
       "m : Drogue " + String(d->drogueEjectionAltitude) + 
       "m : Acc " + String(d->maxAcceleration) + 
       "g : Burnout " + String(d->burnoutAltitude) +
       "ms: Acc Trigger " + String(d->accTriggerTime) + 
       "ms : Alt Trigger " + String(d->altTriggerTime) + "s");
}

void log(String msg)
{
#if LOG_TO_SERIAL
  Serial.println(msg);
#endif
}



