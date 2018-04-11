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

 #include "types.h"

#include <EEPROM.h>
#include <Servo.h>
#include <Ticker.h>

#include "WebServer.hpp"
#include "Altimeter.hpp"
#include "DataLogger.hpp"
#include "BlinkSequence.hpp"

#define LOG_TO_SERIAL 1   //Set to 0 to disable serial logging...


void log(String val);
void playReadyTone();

#include "config.h"



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
int    testFlightTimeStep = 0;
bool              mpuReady = false;              //True if the barometer/altimeter is ready

WebServer         server;
Altimeter         altimeter;
Blinker           blinker(MESSAGE_PIN, BUZZER_PIN);
Ticker            sensorTicker;

//////////////////////////////////////////////////////////////////
// main()

void setup()
{
  EEPROM.begin(4096);
  Serial.begin(SERIAL_BAUD_RATE);

  DataLogger::sharedLogger();

    //String ipAddress = String("Hello World");
  IPAddress serverAddress(IPAddress(192,4,0,1));
  server.setAddress(serverAddress);

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
  altimeter.start();

  resetFlightData(&flightData);
  DataLogger::log("Deployment Altitude: " + String(deploymentAltitude));
  DataLogger::log("Pad Altitude:" + String(refAltitude));

  configureEeprom();

  //We don't want to sound the buzzer on first boot, only after a flight
  enableBuzzer = false;
}

void fly() {
 static SensorData data;
 readSensorData(&data);
 flightControl(&data);
}

void loop()
{
  server.handleClient();

  if(readyToFly && altimeter.isReady()) {
    if(!sensorTicker.active()) {
      DataLogger::sharedDataLogger().clearBuffer();
      sensorTicker.attach_ms(SENSOR_READ_DELAY_MS, fly);
      digitalWrite(READY_PIN, HIGH);
    }
  }else{
    checkResetPin();
  }

  //Blink out the last recorded apogee on the message pin
  if (!readyToFly && flightState == kOnGround) {
    sensorTicker.detach();
    digitalWrite(READY_PIN, LOW);
    blinkLastAltitude();
  }
}

bool checkResetPin()
{
  if (digitalRead(RESET_PIN) == LOW) {
    resetFlightData(&flightData);
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
    return true;
  }
  return false;
}

#define kMaxBlinks 64

void blinkLastAltitude()
{
  if(blinker.isBlinking()) {
    return;
  }

  static Blink sequence[kMaxBlinks];
  int tempApogee = flightData.apogee;
  bool foundDigit = false;         //Don't blink leading 0s
  int n=0;
  for(int m=10000; m>0; m=m/10) {  //If we make it past 99km, we're in trouble :)
    int digit = tempApogee / m;
    if (digit || foundDigit){
      foundDigit = true;
      tempApogee = tempApogee - digit*m;
      if(digit == 0)digit = 10;
      for(int i=0;i<digit;i++) {
        sequence[n].onTime = BLINK_SPEED_MS;
        sequence[n].offTime = BLINK_SPEED_MS;
        if(n<kMaxBlinks)n++;
      }
    }
    if(foundDigit) {
      sequence[n-1].offTime = BLINK_SPEED_MS*2;
    }
  }
  sequence[n].onTime = BLINK_SPEED_MS*2;
  sequence[n].offTime = BLINK_SPEED_MS*2;
  blinker.blinkSequence(sequence, n+1, true);
}

void playReadyTone()
{
  static Blink sequence[3] = {{100,100},{100,100},{100,100}};
  blinker.blinkSequence(sequence, 3, false);
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
  if(altimeter.isReady()) {
    d->altitude = altimeter.getAltitude();
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

  FlightDataPoint dp = FlightDataPoint(millis(), acceleration, alititude);
  DataLogger::sharedDataLogger().logDataPoint(dp);

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
    DataLogger::log("Flight Started");
    flightState = kAscending;
    flightData.altTriggerTime = millis() - resetTime;
    //For testing - to indicate we're in the ascending mode
    digitalWrite(READY_PIN, LOW);
    digitalWrite(MESSAGE_PIN, HIGH);
  }
  else if (flightState == kAscending && altitude < (flightData.apogee - DESCENT_THRESHOLD)) {
    //Transition to kDescendining if we've we're DESCENT_THRESHOLD meters below our apogee
    DataLogger::log("Descending");
    flightState = kDescending;

    //Deploy our drogue chute
    setDeploymentRelay(ON, &drogueChute);
    flightData.drogueEjectionAltitude = altitude;
  }
  else if (flightState == kDescending && altitude < FLIGHT_END_THRESHOLD_ALT) {
    flightState = kOnGround;
    DataLogger::log("Landed");

    DataLogger::log(flightData.toString(flightCount));
    recordFlight(flightData);
    flightCount++;

    //Reset the chutes, reset the relays if we haven't already.  Start the locator
    //beeper and start blinking...

    setDeploymentRelay(OFF, &drogueChute);
    drogueChute.reset();

    setDeploymentRelay(OFF, &mainChute);
    mainChute.reset();

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
    if (!c->timedReset && c->deployed &&
        millis() - c->deploymentTime > maxIgnitionTime)
    {
      int chuteId = c->id;
      DataLogger::log("Chute #" + String(chuteId) + " Timeout");
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
  EEPROM.commit();
}


void configureEeprom()
{
  if (digitalRead(RESET_PIN) == LOW) {
    for (int i = 0 ; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0);
    }
    DataLogger::log("EEProm Wiped");
  }
  DataLogger::log("Reading Flights");
  flightCount = getFlightCount();
}


int getFlightCount()
{
  size_t maxProgs = EEPROM.length() / sizeof(FlightData);
  FlightData d;
  for (int i = 0; i < maxProgs; i++) {
    EEPROM.get(i * sizeof(FlightData), d);
    if (d.isValid())) {
      return i;
    }
    DataLogger::log(d.toString());
    flightData = d;
  }
  DataLogger::log("EEPROM Full.  That's probably an error.  Reset your EEPROM");
  return 0;
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



