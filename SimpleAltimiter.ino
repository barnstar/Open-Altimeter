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
 * All signal PINs should be connected to LEDs via an appropriate resistor
 * The reset pin should be connected to ground via a momentary switch
 * The configuration below uses the internal LED for the status LED.
 * 
 * This is designed for an Adafruit-compatible BMP280 board connected via
 * i2c.  On a nano that's Clock->A5 and Data->A4.  Other barometer sensors 
 * should work just as well.  Cheap MBP280s require you to add the 0x77 paramter
 * to correct the i2c addresss.
 * 
 * See comments on the various pin-outs for operation 
 *
 *********************************************************************************/
 
#include <EEPROM.h>


#include <Adafruit_BMP280.h>             //https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h       
#include "I2Cdev.h"                      //https://diyhacking.com/arduino-mpu-6050-imu-sensor-tutorial/
#include "MPU6050_6Axis_MotionApps20.h"  //https://diyhacking.com/arduino-mpu-6050-imu-sensor-tutorial/ 

#define LOG_TO_SERIAL 1   //Set to 0 to disable serial logging...

typedef Adafruit_BMP280 Barometer;

//Today's pressure at sea level...
const double kSeaLevelPressureHPa = 1013.7;

//Recording will start at 10 m and we'll assume we're on the ground at 30 m.
//In theory, these could be lower, but we want to account for landing in a tree,
//on a hill, etc.  30m should be sufficient for most launch sites.
const double kThresholdStartAltitude = 10;
const double kThresholdEndAltitude = 30;

//100m deployment altitude (330 ft) 
const double kDeploymentAltitude = 100;

//When the altitude is kDescentThreshold meters less than the apogee, we'll assume we're 
//descending.  Hopefully, your rocket has a generally upwards trajectory....
const double kDescentThreshold = 20;

//The deployment relay will be deactivated after this time.
const int kDeploymentRelayTimoutMs = 5000;

//When grounded the reset pin will cancel the last apogee display and
//prepare the alitmiter for the next flight.  If it is grounded on boot
//the eeprom will be erased.
const int RESET_PIN            = 6;

//Configuration A: 1 1/2" PCB
//const int BUZZER_PIN            = 8;   //Audible buzzer on landing
//const int MAIN_DEPL_RELAY_PIN  = 12;  //parachute deployment pin
//const int DROGUE_DEPL_RELAY_PIN= 11;  //parachute deployment pin
//const int STATUS_PIN           = 4;   //Unit status pin.  On if OK
//const int MESSAGE_PIN          = 2;   //Blinks out the altitude
//const int READY_PIN            = 13;  //Inicates the unit is ready for flight

//Configurtion B: 2 1/2" PCB
const int BUZZER_PIN            = 7;  //Audible buzzer on landing
const int MAIN_DEPL_RELAY_PIN  =  8;  //parachute deployment pin
const int DROGUE_DEPL_RELAY_PIN=  9;  //parachute deployment pin
const int STATUS_PIN           =  2;  //Unit status pin.  On if OK
const int MESSAGE_PIN          =  3;  //Blinks out the altitude
const int READY_PIN            =  4;  //Inicates the unit is ready for flight

const int SERIAL_BAUD_RATE     = 9600;

//The barometer can only refresh at about 50Hz. 
const int SENSOR_READ_DELAY_MS = 20;

//Delay between digit blinks.  Any faster is too quick to keep up with
const int BLINK_SPEED_MS       = 200;


typedef struct  {
  double apogee;
  double ejectionAltitude;
  bool isValid() {
    return apogee || ejectionAltitude;
  }
} FlightData;

typedef enum {
  kInFlight,
  kAscending,
  kDescending,
  kOnGround
} FlightState;

Barometer   barometer;
MPU6050 mpu;


double refAltitude = 0;         //The reference altitude (altitude of the launch pad)
double apogee = 0;              //The apogee for the current flight
double previousApogee = 0;      //The last recoreded apogee
bool   deployedMain = false;    //True if the the chute has been deplyed
bool   deployedDrogue = false;  //True if the the chute has been deplyed
int    drogueDeploymentTime = 0;//Time at which the chute was deployed
int    mainDeploymentTime = 0;  //Time at which the chute was deployed
bool   timedResetMain = false;  //True if we've reset the chute relay due to a timeout
bool   timedResetDrogue = false;//True if we've reset the chute relay due to a timeout
bool   enableBuzzer = false;    //True if the buzzer should be sounding
int    flightCount = 0;         //The number of flights recorded in EEPROM
bool   drogueChuteRelayState = false; //State of the parachute relay pin.  Recorded separately.  To avoid fire.
bool   mainChuteRelayState = false;   //State of the parachute relay pin.  Recorded separately.  To avoid fire.
bool   barometerReady = false; 
double maxAcc = 0;

FlightState state = kOnGround;  //The flight state

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  pinMode(MAIN_DEPL_RELAY_PIN, OUTPUT);
  pinMode(DROGUE_DEPL_RELAY_PIN, OUTPUT);

  pinMode(RESET_PIN, INPUT_PULLUP);

  pinMode(MESSAGE_PIN, OUTPUT);
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(READY_PIN, OUTPUT);

  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(MESSAGE_PIN, HIGH);

  delay(100);   //The barometer doesn't like being queried immediately
  if (barometer.begin(0x76)) {  //Omit the parameter for adafruit
    digitalWrite(STATUS_PIN, HIGH);
    digitalWrite(MESSAGE_PIN, LOW);
    log("Barometer Started");
    barometerReady = true;
  } else {
    //If the unit starts with the status pin off and the message pin on,
    //the barometer failed to initialize
    previousApogee = 0;
    log("Barometer Init Fail");
  }

  mpu.initialize();
  bool mpuActive = mpu.testConnection();
  log(mpuActive? "MPU6050 connection successful" : "MPU6050 connection failed");
  if(mpuActive) {
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
    log("MPU Started");
  }
  
  log("Deployment Alt: " + String(kDeploymentAltitude));
  refAltitude = barometer.readAltitude(kSeaLevelPressureHPa);
  log("Ref Alt:" + String(refAltitude));

  configureEeprom();

  //We don't want to sound the buzzer on first boot, only after a flight
  enableBuzzer = false;
}


void loop()
{
  if(previousApogee == 0 && barometerReady) {
    readSensorData();
    delay(SENSOR_READ_DELAY_MS);
    digitalWrite(READY_PIN, HIGH);
  }

  //Blink out the last recorded apogee on the message pin
  if (previousApogee != 0 && state == kOnGround) {
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
        previousApogee = 0;\
        enableBuzzer = false;\
        return;\
      }\
   }while(0); \

//Not particularily efficent... We could use timers or interrupts here
//but it's good enough.  You'll need to hold down the reset button for
//a couple of seconds at worst.  
void blinkLastAltitude()
{
  int tempApogee = previousApogee;
  bool foundDigit = false;         //Don't blink leading 0s
  for(int m=10000; m>0; m=m/10) {  //If we make it past 99km, we're in trouble :)
    int digit = tempApogee / m;
    if (digit || foundDigit){
      foundDigit = true;
      if(digit == 0)digit = 10;
      for (int i = 0; i < digit; i++) {
        digitalWrite(MESSAGE_PIN, HIGH); delay(BLINK_SPEED_MS);
        digitalWrite(MESSAGE_PIN, LOW);  delay(BLINK_SPEED_MS);
        CHECK_RESETPIN_AND_RETURN();
      }
      delay(BLINK_SPEED_MS * 2);
      tempApogee = tempApogee - digit * m;
    }
  }
  CHECK_RESETPIN_AND_RETURN();
  digitalWrite(MESSAGE_PIN, HIGH);
  if(enableBuzzer)digitalWrite(BUZZER_PIN, HIGH);
  delay(BLINK_SPEED_MS * 10);
  digitalWrite(MESSAGE_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  delay(BLINK_SPEED_MS);
  CHECK_RESETPIN_AND_RETURN();
}

void log(String msg)
{
#if LOG_TO_SERIAL
  Serial.println(msg);
#endif
}



int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t grx, gry, grz;
const double onegee = 65536.0 / 16.0; //units/g for scale of +/- 8g

double getAccelleration()
{
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  double axd = ax / onegee ;
  double ayd = ay / onegee ;
  double azd = az / onegee ;

  //Remove gravity.. We now have the absolute accelleration
  double acc = sqrt( axd * axd + ayd * ayd + azd * azd) - 1.0;
  return acc;
}

void readSensorData()
{
  //Our relative altitude... Relative to wherever we last reset the altimeter.
  double altitude = barometer.readAltitude(kSeaLevelPressureHPa) - refAltitude;
  double accelleration = getAccelleration();

  if(state != kOnGround) {
    log(String("Alt:") + String(altitude) + " Acc:" + String(accelleration));
  }

  apogee = altitude > apogee ? altitude : apogee;

  if (state == kOnGround && altitude > kThresholdStartAltitude) {
    //Transition to "InFlight" if we've exceeded the threshold altitude.
    log("Flight Started");
    state = kAscending;
    digitalWrite(READY_PIN, LOW);
    digitalWrite(MESSAGE_PIN, HIGH);
  } else if (state == kAscending && altitude < (apogee - kDescentThreshold)) {
    //Transition to "Descnding" if we've we're kDescentThreshold meters below our apogee
    log("Descending");
    state = kDescending;
    //We record the apogee here just in case something breaks on descent.. At least
    //we have the altitude in EEPROM.
    recordApogee(apogee);
    setMainDeploymentRelay(true);
    mainDeploymentTime = millis();
  }
  else if (state == kDescending && altitude < kThresholdEndAltitude) {
    //Transition to "On Ground" if we've we're below our ending threshold altitude.
    //With any luck, we're acutally on the ground...
    state = kOnGround;
    log(String("Landing.  Apogee:" + String(apogee)));
    previousApogee = apogee;
    apogee = 0;
    flightCount++;
    deployedMain = false;
    deployedDrogue = false;
    timedResetMain = false;
    timedResetDrogue = false;
    setDrogueDeploymentRelay(false);
    setMainDeploymentRelay(false);
    enableBuzzer = true;
  }

  if (state == kDescending && !deployedMain && altitude < kDeploymentAltitude) {
    //If we're descening and we're below our deployment altitude, deploy the chute!
    deployedMain = true;
    recordDepolyment(altitude);
    setDrogueDeploymentRelay(true);
    drogueDeploymentTime = millis();
  }

  //Safety measure in case we don't switch to the onGround state.  This will disable the igniter relay
  //after 5 seconds to avoid draining or damaging the battery
  if (state == kDescending) {
    if (millis() - drogueDeploymentTime > kDeploymentRelayTimoutMs && !timedResetDrogue && deployedDrogue) {
      setDrogueDeploymentRelay(false);
      deployedDrogue = false;
      timedResetDrogue = true;
    }
    if (millis() - mainDeploymentTime > kDeploymentRelayTimoutMs &&!timedResetMain && deployedMain) {
      setMainDeploymentRelay(false);
      deployedMain = false;
      timedResetMain = true;
    }
  }

}

void setDrogueDeploymentRelay(bool enabled)
{
  if(enabled == drogueChuteRelayState)return;
  
  if (enabled) {
    digitalWrite(DROGUE_DEPL_RELAY_PIN, HIGH);
    log("Deploying drogue");
  } else {
    digitalWrite(DROGUE_DEPL_RELAY_PIN, LOW);
    log("Resetting drogue deployment relay");
  }
  drogueChuteRelayState = enabled;
}

void setMainDeploymentRelay(bool enabled)
{
  if(enabled == mainChuteRelayState)return;
  
  if (enabled) {
    digitalWrite(MAIN_DEPL_RELAY_PIN, HIGH);
    log("Deploying parachte");
  } else {
    digitalWrite(MAIN_DEPL_RELAY_PIN, LOW);
    log("Resetting main deployment relay");
  }
  mainChuteRelayState = enabled;
}



//Note that we record these separately so that we get at least an altitude
//reading if things go south on descent...
void recordApogee(double a)
{
  int offset = flightCount * sizeof(FlightData);
  EEPROM.put(offset, a);
}


void recordDepolyment(double d)
{
  int offset = flightCount * sizeof(FlightData) + sizeof(double);
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
    if (!d.isValid()) {
      return i;
    }
    log(String("Flight " + String(i) + ":" + String(d.apogee) + ":" + String(d.ejectionAltitude)));
    previousApogee = d.apogee;
  }
  log("EEPROM Full.  That's probably an error.  Reset your EEPROM");
  return 0;
}




