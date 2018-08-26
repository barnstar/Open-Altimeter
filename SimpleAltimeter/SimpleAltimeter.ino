
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

#define Nsta 1     // 1 state values: pressure
#define Mobs 1     // 1 measurements: baro pressure


#include <EEPROM.h>
#include <Servo.h>

//Sensor libraries
#include <Adafruit_BMP280.h>             //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h       
//#include "I2Cdev.h"                      //https://diyhacking.com/arduino-mpu-6050-imu-sensor-tutorial/
//#include "MPU6050_6Axis_MotionApps20.h"  //https://diyhacking.com/arduino-mpu-6050-imu-sensor-tutorial/ 
#include <Wire.h>

#define LOG_TO_SERIAL 1   //Set to 0 to disable serial logging...
#define PLOT_ALTITUDE 0   //Set to 1 to watch the altitude on the serial plotter

void log(String val);
void playReadyTone();

//////////////////////////////////////////////////////////////////
// Types

#include <math.h>

class KalmanFilter
{
  private:

  float err_measured = 0;
  float err_estimated = 0 ;
  float q = 0;
  float last_estimate = 0;

  public:

  KalmanFilter::KalmanFilter() {
    this->reset(1,1,0.001);
  } 

  KalmanFilter::~KalmanFilter() {};
  
  public:
  double step(double measurement)
  { 
    double kalman_gain = err_estimated/(err_estimated + err_measured);
    double current_estimate = last_estimate + kalman_gain * (measurement - last_estimate);
    
    err_estimated =  (1.0 - kalman_gain)* err_estimated + fabs(last_estimate-current_estimate)*q;
    last_estimate= current_estimate;
  
    return current_estimate;
  };

  void reset(double measuredError, double estimatedError, double gain)
  {
     this->err_measured=measuredError;
     this->err_estimated=estimatedError;
     this->q = gain;
     this->last_estimate = 0;
  }
};

typedef enum {
  kNoEjection,
  kPyro,
  kServo  
} EjectionType;

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

//Flight Data operations
bool isValid(FlightData *d);
void reset(FlightData *d);
void logData(int index, FlightData *d);

typedef enum {
  OFF = 0,
  ON =1 
}RelayState;

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


static const int kMinServoAngle = 5;
static const int kMaxServoAngle = 120;

class ChuteState
{
  public:

  ChuteState::ChuteState() {
    this->reset();
  };

  ChuteState::~ChuteState() {};
  
  bool         deployed ;         //True if the the chute has been deplyed
  int          deploymentTime ;   //Time at which the chute was deployed
  bool         timedReset ;       //True if we've reset the chute relay due to a timeout
  RelayState   relayState = OFF;  //State of the parachute relay pin.  Recorded separately.  To avoid fire.
  EjectionType type;
  Servo        servo;

  int    relayPin=0;
  char   id = 0; 

  void init(int id, int pin, EjectionType type) {
      log("Init Chute " + String(id) + " on pin " + String(pin));
      this->relayPin = pin;
      this->id = id;
      this->type = type;
      switch(type){
        case kPyro:  pinMode(pin, OUTPUT); break;
        case kServo: servo.attach(pin); break;
        case kNoEjection: break;
      }
      reset();
  };

  void enable() {
    deployed = true;
    deploymentTime = millis();
    relayState = ON;
    switch(type){
      case kPyro:  digitalWrite(relayPin, HIGH);; break;
      case kServo: servo.write(kMinServoAngle); break;
      case kNoEjection: break;
    }
    log("Deploying Chute #" + String(id) );
  };

  
  void disable() {
    deployed = false;
    relayState = OFF;
    switch(type){
      case kPyro:  digitalWrite(relayPin, LOW);; break;
      case kServo: servo.write(kMaxServoAngle); break;
      case kNoEjection: break;
    }
    log("Disabling Chute #" + String(id) );
  };

  void reset() {
    deployed = false;
    deploymentTime = 0;
    timedReset = false;
    disable();
  };
  
};


//////////////////////////////////////////////////////////////////
// Configuration

//Today's pressure at sea level...
const double SEA_LEVEL_PRESSURE = 1013.7;

//Recording will start at FLIGHT_START_THRESHOLD_ALT m and we'll assume we're on the 
//ground at FLIGHT_END_THRESHOLD_ALT m.
//In theory, these could be lower, but we want to account for landing in a tree,
//on a hill, etc.  30m should be sufficient for most launch sites.
const double FLIGHT_START_THRESHOLD_ALT = 30;
const double FLIGHT_END_THRESHOLD_ALT   = 30;
const double FLIGHT_START_THRESHOLD_ACC = 0.1;  //in G's

//When the altitude is DESCENT_THRESHOLD meters less than the apogee, we'll assume we're 
//descending.  Hopefully, your rocket has a generally upwards trajectory....
const double DESCENT_THRESHOLD = 20;

//The deployment relay will be deactivated after this time.
const int MAX_FIRE_TIME = 5000;

//When grounded the reset pin will cancel the last apogee display and
//prepare the alitmiter for the next flight.  If it is grounded on boot
//the eeprom will be erased.

#define USE_PIN_CONFIG_1 0
#define USE_PIN_CONFIG_2 0
#define USE_PIN_CONFIG_3 1
#define USE_PIN_CONFIG_4 0

#if USE_PIN_CONFIG_1
//Configuration A: 1 1/2" PCB
const int SERIAL_BAUD_RATE     = 9600;
const int STATUS_PIN            = 4;   //Unit status pin.  On if OK
const int MESSAGE_PIN           = 2;   //Blinks out the altitude
const int READY_PIN             = 13;  //Inicates the unit is ready for flight
const int BUZZER_PIN            = 8;   //Audible buzzer on landing
const int RESET_PIN             = 6;
const int TEST_PIN              = 7;
const int MAIN_DEPL_RELAY_PIN   = 12;   //parachute deployment pin
const int DROGUE_DEPL_RELAY_PIN = 11;  //parachute deployment pin
const int ALT_PIN_A             = 9;  //Main Chute Alitude Altitude Set Pin.
const int ALT_PIN_B             = 10;  //Main Chute Alitude Altitude Set Pin
const EjectionType MAIN_TYPE    = kPyro;
const EjectionType DROGUE_TYPE  = kPyro; 
const int BARO_I2C_ADDR         = 0x77;
const PeizoStyle PEIZO_TYPE     = kActive;
#elif USE_PIN_CONFIG_2
//Configuration B: 2" PCB w. Servo Sled
const int SERIAL_BAUD_RATE     = 9600;
const int STATUS_PIN            = 4;   //Unit status pin.  On if OK
const int MESSAGE_PIN           = 5;   //Blinks out the altitude
const int READY_PIN             = 6;   //Inicates the unit is ready for flight
const int BUZZER_PIN            = 3;   //Audible buzzer on landing
const int RESET_PIN             = 7;
const int TEST_PIN              = 12;
const int MAIN_DEPL_RELAY_PIN   = 11;  //parachute deployment pin
const int DROGUE_DEPL_RELAY_PIN = 10;  //parachute deployment pin
const int ALT_PIN_A             = 8;   //Main Chute AlitudeAltitude Set Pin.
const int ALT_PIN_B             = 9;   //Main Chute Alitude Altitude Set Pin
const EjectionType MAIN_TYPE    = kServo;
const EjectionType DROGUE_TYPE  = kServo; 
const int BARO_I2C_ADDR         = 0x76;
const PeizoStyle PEIZO_TYPE     = kPassive;
#elif USE_PIN_CONFIG_3
//Configuration B: Small PCB with servo pinout
const int SERIAL_BAUD_RATE     = 9600;
const int STATUS_PIN            = 5;   //Unit status pin.  On if OK
const int MESSAGE_PIN           = 3;   //Blinks out the altitude
const int READY_PIN             = 13;   //Inicates the unit is ready for flight
const int BUZZER_PIN            = 2;   //Audible buzzer on landing
const int RESET_PIN             = 4;
const int TEST_PIN              = 10;
const int MAIN_DEPL_RELAY_PIN   = 11;  //parachute deployment pin
const int DROGUE_DEPL_RELAY_PIN = 12;  //parachute deployment pin
const int ALT_PIN_A             = 8;   //Main Chute AlitudeAltitude Set Pin.
const int ALT_PIN_B             = 9;   //Main Chute Alitude Altitude Set Pin
const EjectionType MAIN_TYPE    = kServo;
const EjectionType DROGUE_TYPE  = kNoEjection; 
#define BARO_I2C_ADDR
#define STATUS_PIN_LEVEL 800
const PeizoStyle PEIZO_TYPE     = kPassive;
#endif

//The barometer can only refresh at about 50Hz. 
const int SENSOR_READ_DELAY_MS = 5;

//Delay between digit blinks.  Any faster is too quick to keep up with
const int BLINK_SPEED_MS       = 200;


//////////////////////////////////////////////////////////////////
// Global State

FlightData flightData;
FlightState flightState = kOnGround;  //The flight state

ChuteState mainChute;
ChuteState drogueChute;

double refAltitude = 0;               //The reference altitude (altitude of the launch pad)
int    flightCount = 0;               //The number of flights recorded in EEPROM
int    resetTime = 0;                 //millis() after starting the current flight
bool   readyToFly = false;            //switches to false at the end of the flight.  Resets on reset.
bool   enableBuzzer = false;          //True if the buzzer should be sounding
double deploymentAltitude = 100;      //Deployment altitude in ft.
int    testFlightTimeStep = 0;

Adafruit_BMP280   barometer;
//MPU6050           mpu;
bool              barometerReady = false;        //True if the barometer/altimeter is ready
bool              mpuReady = false;              //True if the barometer/altimeter is ready
KalmanFilter      filter;

//////////////////////////////////////////////////////////////////
// main()

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
  
  if(TEST_PIN) {
    pinMode(TEST_PIN, INPUT_PULLUP);
  }

  //Start in the "error" state.  Status pin should be high and message
  //pin should be low to indicate a good startup
  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(MESSAGE_PIN, HIGH);

  mainChute.init(2, MAIN_DEPL_RELAY_PIN, MAIN_TYPE);
  drogueChute.init(1, DROGUE_DEPL_RELAY_PIN, DROGUE_TYPE);

  delay(100);   //The barometer doesn't like being queried immediately
  if (barometer.begin(BARO_I2C_ADDR)) {  //Omit the parameter for adafruit
    #ifdef STATUS_PIN_LEVEL
    analogWrite(STATUS_PIN, STATUS_PIN_LEVEL);
    #else
    digitalWrite(STATUS_PIN, HIGH);
    #endif
    digitalWrite(MESSAGE_PIN, LOW);
    log("Barometer Started");
    barometerReady = true;
  } else {
    //If the unit starts with the status pin off and the message pin on,
    //the barometer failed to initialize
    log("Barometer Init Fail");
  }

//  mpu.initialize();
//  mpuReady = mpu.testConnection();
//  log(mpuReady? "MPU6050 connection successful" : "MPU6050 connection failed");
//  if(mpuReady) {
//    Don't really care if it's not ready or not present
//    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
//  }

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

  int a = (digitalRead(ALT_PIN_A) == LOW) ? 1 : 0;
  int b = (digitalRead(ALT_PIN_B) == LOW) ? 1 : 0;

  static const int altitudes[] = { 100, 150, 150, 200 };

  int val = a + b*2;
  log("Alt Index:" + String(val) + String(a)+ String(b));
  if(val > 3)val = 0;
  return altitudes[val]; 
}


void loop()
{
  static SensorData data;
  if(filghtState !=kOnGround && barometerReady) {
    readSensorData(&data);
    flightControl(&data);
    delay(SENSOR_READ_DELAY_MS);
    digitalWrite(READY_PIN, HIGH);
  }

  //Blink out the last recorded apogee on the message pin
  if (flightState == kOnGround) {
        digitalWrite(READY_PIN, LOW);
        blinkLastAltitude();
  }
}

bool checkResetPin()
{
  if (digitalRead(RESET_PIN) == LOW) {
    reset(&flightData); 
    resetTime = millis(); 
    readyToFly = true; 
    setDeploymentRelay(OFF, &drogueChute); 
    drogueChute.reset(); 
    setDeploymentRelay(OFF, &mainChute); 
    mainChute.reset(); 
    testFlightTimeStep = 0;
    enableBuzzer = true;
    playReadyTone();
    enableBuzzer = false;   
    flightState = kReadyToFly; 
    filter.reset(1,1,0.001);
    return true;
  }
  return false;
}

void blinkWithDelay(int onTime, int offTime, int frequency)
{
  digitalWrite(MESSAGE_PIN, HIGH); 
  if(enableBuzzer){
    if(PEIZO_TYPE == kActive){
      digitalWrite(BUZZER_PIN, HIGH); 
    }else if(PEIZO_TYPE == kPassive) {
      tone(BUZZER_PIN, frequency);
    }
  }
  delay(onTime);
  digitalWrite(MESSAGE_PIN, LOW);
  if(enableBuzzer){
    if(PEIZO_TYPE == kActive){
      digitalWrite(BUZZER_PIN, LOW); 
    }else if(PEIZO_TYPE == kPassive) {
      noTone(BUZZER_PIN);
    }
  }
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
        blinkWithDelay(BLINK_SPEED_MS, BLINK_SPEED_MS, 1760);
        if(checkResetPin())return;
      }
      delay(BLINK_SPEED_MS * 2);
      tempApogee = tempApogee - digit * m;
    }
  }
  if(checkResetPin())return;
  blinkWithDelay(BLINK_SPEED_MS * 10, BLINK_SPEED_MS, 1760*2);
  if(checkResetPin())return;
}

void playReadyTone()
{
  blinkWithDelay(150,50,880);
  blinkWithDelay(150,50,1109);
  blinkWithDelay(150,450,1318);
}



////////////////////////////////////////////////////////////////////////
// Sensors 

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t grx, gry, grz;
const double onegee = 65536.0 / 16.0; //units/g for scale of +/- 8g

double getacceleration()
{
  //mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  double axd = ax / onegee ;
  double ayd = ay / onegee ;
  double azd = az / onegee ;

  //Remove gravity.. We now have the absolute acceleration
  double acc = sqrt( axd * axd + ayd * ayd + azd * azd) - 1.0;
  return acc;
}

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
    d->acceleration = getacceleration();
  }
}



////////////////////////////////////////////////////////////////////////
// Flight Control 

static const int kSamples = 8;

void flightControl(SensorData *d)
{
  double acceleration = d->acceleration;
  double altitude = filter.step(d->altitude);

  if(PLOT_ALTITUDE) { log(String(altitude)); }

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

    //Reset the pyro charges.  Leave chute releases open.  Start the locator
    //beeper and start blinking...
    resetChuteIfRequired(&drogueChute);
    resetChuteIfRequired(&mainChute);
     
        
    enableBuzzer = true;
    readyToFly = false;
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

void resetChuteIfRequired(ChuteState *c)
{
  if(c->type == kPyro) {
     setDeploymentRelay(OFF, c);
     c->reset();
  }
}

void checkChuteIgnitionTimeout(ChuteState *c, int maxIgnitionTime)
{
    if (!c->timedReset && c->deployed &&
        millis() - c->deploymentTime > maxIgnitionTime &&
        c->type == kPyro) 
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
  d->apogeeTime = 0;

}

void logData(int index, FlightData *d) {
   log("Flight " + String(index) + 
       " : [Apogee " + String(d->apogee) + 
       "m] : [Main " + String(d->ejectionAltitude) + 
       "m] : [Drogue " + String(d->drogueEjectionAltitude) + 
       "m] : [Acc " + String(d->maxAcceleration) + 
       "ms] : [Apogee Time  " + String(d->apogeeTime) +
       "ms]: [Acc Trigger Time" + String(d->accTriggerTime) + 
       "ms] : [Alt Trigger Time " + String(d->altTriggerTime) + "s]");
}

void log(String msg)
{
#if LOG_TO_SERIAL
  Serial.println(msg);
#endif
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




