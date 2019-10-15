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

#define VERSION 3

#include "SimpleAltimeter.h"

/////////////////////////////////////////////////////////////////
// Global State

FlightData flightData;
FlightState flightState = kOnGround;  // The flight state

RecoveryDevice mainChute;
RecoveryDevice drogueChute;

double refAltitude = 0;  // The reference altitude (altitude of the launch pad)
int flightCount    = 0;  // The number of flights recorded in EEPROM
int resetTime      = 0;  // millis() after starting the current flight
double deploymentAltitude = 100;  // Deployment altitude in m.
int testFlightTimeStep    = 0;

Barometer barometer;
#if ENABLE_MPU
ImuSensor imu(Wire, 0x68);
#endif

KalmanFilter filter(0);
#if ENABLE_MPU
Mahony sensorFusion;
#endif

bool barometerReady = false;  // True if the barometer/altimeter is ready
bool mpuReady       = false;  // True if the barometer/altimeter is ready
SimpleTimer timer;
Blinker blinker(timer, MESSAGE_PIN, BUZZER_PIN);
TimerProxy flightControlInterruptProxy(flightControllInterrupt);

static unsigned long lastFireTime = 0;
int flightControlTimer            = 0;
SensorData data;

//////////////////////////////////////////////////////////////////
// main()

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  log("V " + String(VERSION));

  pinMode(RESET_PIN, INPUT_PULLUP);

  // All LED pins sset to outputs
  pinMode(MESSAGE_PIN, OUTPUT);
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(READY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  if (TEST_PIN) {
    pinMode(TEST_PIN, INPUT_PULLUP);
  }

  // Start in the "error" state.  Status pin should be high and message
  // pin should be low to indicate a good startup
  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(MESSAGE_PIN, HIGH);

  mainChute.init(2, MAIN_DEPL_RELAY_PIN, MAIN_TYPE);
  drogueChute.init(1, DROGUE_DEPL_RELAY_PIN, DROGUE_TYPE);

  if (barometer.begin(BARO_I2C_ADDR)) {  // Omit the parameter for adafruit
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

#if ENABLE_MPU
  mpuReady = !(imu.begin() < 0);
  log(mpuReady ? "IMU OK" : "IMU failed");
#endif

//For MPU6050 
#if ENABLE_MPU
//  mpu.initialize();
//  mpuReady = mpu.testConnection();
//  log(mpuReady ? "IMU OK" : "IMU failed");
//  if (mpuReady) {
//    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
//  }
#endif

  reset(&flightData);
  deploymentAltitude = readDeploymentAltitude();
  log("Depl Alt: " + String(deploymentAltitude));
  refAltitude = barometer.readAltitude(SEA_LEVEL_PRESSURE);
  log("Pad Alt:" + String(refAltitude));

  configureEeprom();
}


void loop()
{
  timer.run();
  if (!barometerReady && !blinker.isBlinking()) {
    blinker.blinkValue(3, BLINK_SPEED_MS, true);
  }
  if (flightState == kOnGround) {
    // Kill Timer
    timer.deleteTimer(flightControlTimer);
    flightControlTimer = -1;
    digitalWrite(READY_PIN, LOW);
    if (flightData.apogee && !blinker.isBlinking()) {
      blinker.blinkValue(flightData.apogee, BLINK_SPEED_MS, true);
    }
    failsafeCheck();
  }
  checkResetPin();

}

void failsafeCheck()
{ 
  //If the unit resets itself in flight, the reference altitude will reset 
  //and the state will be reset to kOnGround
  //This won't catch all failures, but it should deploy all chutes if we
  //detect that we're "underground"
  if(flightState == kOnGround) {
    SensorData d;
    readSensorData(&d);
    if(d.altitude < FAILSAFE_ALTITUDE) {
       log("Failsafe Triggerd");
       setRecoveryDeviceState(ON, &mainChute);
       setRecoveryDeviceState(ON, &drogueChute);
    }
  }
}


void flightControllInterrupt()
{
  if (flightState != kOnGround) {
    readSensorData(&data);
    flightControl(&data);
    digitalWrite(READY_PIN, HIGH);
    //checkResetPin();
  }
}


int readDeploymentAltitude()
{
  pinMode(ALT_PIN_A, INPUT_PULLUP);
  pinMode(ALT_PIN_B, INPUT_PULLUP);

  int a = (digitalRead(ALT_PIN_A) == LOW) ? 1 : 0;
  int b = (digitalRead(ALT_PIN_B) == LOW) ? 1 : 0;

  static const int altitudes[] = {75, 100, 150, 200};

  int val = (b * 2 + a) % 4;
  log("Alt Index:" + String(val) + String(":") + String(b) + String(1));
  return altitudes[val];
}

bool isLow = false;
bool checkResetPin()
{ 
  if (digitalRead(RESET_PIN) == LOW && !isLow) {
    isLow = true;
    if(flightState != kReadyToFly) {      
      log("Resetting");
      if(!barometerReady) {
        playCancelTone();
      }else{
        blinker.cancelSequence();
        reset(&flightData);
        resetTime = millis();
        setRecoveryDeviceState(OFF, &drogueChute);
        drogueChute.reset();
        setRecoveryDeviceState(OFF, &mainChute);
        mainChute.reset();
        testFlightTimeStep = 0;
        playReadyTone();
        flightState = kReadyToFly;
        filter.reset(0);
        #if ENABLE_MPU
        sensorFusion.begin(1000 / SENSOR_READ_DELAY_MS);
        #endif
        flightControlTimer =
            timer.setInterval(SENSOR_READ_DELAY_MS, &flightControlInterruptProxy);
      }
    }
    else
    {
      log("Stopping");
      flightState = kOnGround;
      flightData.apogee = 0;
      playCancelTone();
    }
    return true;
  }
  
  if(digitalRead(RESET_PIN) == HIGH) {
    isLow = false;
  }
  
  return false;
}

void playReadyTone() { blinker.blinkValue(3, 100, false); }

void playCancelTone() { blinker.blinkValue(1, 100, false); }

////////////////////////////////////////////////////////////////////////
// Sensors
void readSensorData(SensorData *d)
{
  if (TEST_PIN && ((digitalRead(TEST_PIN) == LOW) || testFlightTimeStep)) {
    testFlightData(d);
    return;
  }

  // Our relative altitude... Relative to wherever we last reset the altimeter.
  if (barometerReady) {
    d->altitude = barometer.readAltitude(SEA_LEVEL_PRESSURE) - refAltitude;
  }
#if ENABLE_MPU
  if (mpuReady) {
    imu.readSensor();
    sensorFusion.update(imu.getGyroX_rads(), imu.getGyroY_rads(),
                        imu.getGyroZ_rads(), imu.getAccelX_mss(),
                        imu.getAccelY_mss(), imu.getAccelZ_mss(),
                        imu.getMagX_uT(), imu.getMagY_uT(), imu.getMagZ_uT());
  }
#endif
}

////////////////////////////////////////////////////////////////////////
// Flight Control

int samples_below_apogee  = 0;
int samples_at_min_height = 0;
int samples_above_min_acc = 0;

void flightControl(SensorData *d)
{
  double acceleration = d->acceleration;
  double altitude     = filter.step(d->altitude);

  if (PLOT_ALTITUDE) {
    log(String(altitude));
#if ENABLE_MPU
    log(String(sensorFusion.getYaw()) + ":" + String(sensorFusion.getPitch()) +
        ":" + String(sensorFusion.getRoll()));
#endif
  }

  // Keep track or our apogee and our max g load
  if (altitude > flightData.apogee) {
    flightData.apogee     = altitude;
    flightData.apogeeTime = millis() - resetTime;
  }
  flightData.maxAcceleration = acceleration > flightData.maxAcceleration
                                   ? acceleration
                                   : flightData.maxAcceleration;


  if (flightState == kReadyToFly ) 
  {
    if(acceleration > FLIGHT_START_THRESHOLD_ACC) {
      samples_above_min_acc++;
    }else{
      samples_above_min_acc = 0;
    }

    //A flight is either, 3 samples at > .1G or 1 sample above the height threshold
    //If we're above our flight start threshold, we're flying!
    if(altitude >= FLIGHT_START_THRESHOLD_ALT || samples_above_min_acc > 3) {
      // Transition to "InFlight" if we've exceeded the threshold altitude.
      log("Ascending");
      samples_below_apogee = 0;
      flightState               = kAscending;
      flightData.altTriggerTime = millis() - resetTime;
      digitalWrite(READY_PIN, LOW);
      digitalWrite(MESSAGE_PIN, HIGH);
    }
  } 
  else if (flightState == kAscending) 
  {
    // Transition to kDescending if we've recorded 5 samples below our apogee
    if(altitude < flightData.apogee) {
      samples_below_apogee++;
    }else{
      samples_below_apogee = 0;
    }

    if(samples_below_apogee > 5) {
      flightState = kDescending;
      samples_at_min_height = 0;
      log(F("Descending"));
       // Deploy our drogue chute
      setRecoveryDeviceState(ON, &drogueChute);
      flightData.drogueEjectionAltitude = altitude;
    }
  } 
  else if (flightState == kDescending) 
  {
    if(altitude < FLIGHT_END_THRESHOLD_ALT) {
      samples_at_min_height++;
    }else{
      samples_at_min_height = 0;
    }

    //We've most likely hit the ground
    if(samples_at_min_height > 3) {
      flightState = kOnGround;
      log(F("Landed"));
      logData(flightCount, &flightData);
      recordFlight(flightData);
      flightCount++;
  
      // Reset the pyro charges.  Leave chute releases open.  Start the locator
      // beeper and start blinking...
      resetRecoveryDeviceIfRequired(&drogueChute);
      resetRecoveryDeviceIfRequired(&mainChute);
    }
  }

  // Main chute deployment at kDeployment Altitude.
  // We deploy in the onGround state as well just in case an anomalous pressure
  // spike has transitioned us to that state while we're still in the air
  if ((flightState == kDescending || flightState == kOnGround) &&
      !mainChute.deployed && altitude <= deploymentAltitude) {
    // If we're descening and we're below our deployment altitude, deploy the
    // chute!
    flightData.ejectionAltitude = altitude;
    setRecoveryDeviceState(ON, &mainChute);
  }

  // Safety measure in case we don't switch to the onGround state.  This will
  // disable the igniter relay after 5 seconds to avoid draining or damaging the
  // battery.  This only applies for pyro type.
  checkChuteIgnitionTimeout(&mainChute, MAX_FIRE_TIME);
  checkChuteIgnitionTimeout(&drogueChute, MAX_FIRE_TIME);
}

void resetRecoveryDeviceIfRequired(RecoveryDevice *c)
{
  if (c->type == kPyro) {
    setRecoveryDeviceState(OFF, c);
    c->reset();
  }
}

void checkChuteIgnitionTimeout(RecoveryDevice *c, int maxIgnitionTime)
{
  if (!c->timedReset && c->deployed &&
      millis() - c->deploymentTime > maxIgnitionTime && c->type == kPyro) {
    int chuteId = c->id;
    setRecoveryDeviceState(OFF, c);
    c->timedReset = true;
  }
}

void setRecoveryDeviceState(RecoveryDeviceState deviceState, RecoveryDevice *c)
{
  if (deviceState == c->deviceState) return;
  int chuteId = c->id;

  switch (deviceState) {
    case ON:
      c->enable();
      break;
    case OFF:
      c->disable();
      break;
  }
}

////////////////////////////////////////////////////////////////////////
// EEPROM & Peristance

void recordFlight(FlightData d)
{
  int offset = flightCount * sizeof(FlightData);
  EEPROM.put(offset, d);
}

void configureEeprom()
{
  if (digitalRead(RESET_PIN) == LOW) {
    for (int i = 0; i < EEPROM.length(); i++) {
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
// Flight Data Utilities

bool isValid(FlightData *d)
{
  return d->apogee || d->ejectionAltitude || d->drogueEjectionAltitude ||
         d->maxAcceleration || d->burnoutAltitude;
}

void reset(FlightData *d)
{
  d->apogee                 = 0;
  d->ejectionAltitude       = 0;
  d->drogueEjectionAltitude = 0;
  d->maxAcceleration        = 0;
  d->burnoutAltitude        = 0;
  d->accTriggerTime         = 0;
  d->altTriggerTime         = 0;
  d->apogeeTime             = 0;
}

void logData(int index, FlightData *d)
{
  log("# " + String(index) + ": Apo " + String(d->apogee) + "m:Main " +
      String(d->ejectionAltitude));
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
  if (0 == TEST_PIN) return;

  if (testFlightTimeStep == 0) {
    testFlightTimeStep    = 1;
    fakeData.altitude     = 0;
    fakeData.acceleration = 4;
    d->altitude           = fakeData.altitude;
    d->acceleration       = fakeData.acceleration;
    isTestAscending       = true;
    return;
  }

  if (fakeData.altitude > testApogee) {
    isTestAscending = false;
  }

  double increment = isTestAscending ? 5.0 : -2.0;
  fakeData.altitude += increment;

  testFlightTimeStep++;

  d->altitude     = fakeData.altitude;
  d->acceleration = fakeData.acceleration;
}

void log(String msg)
{
#if LOG_TO_SERIAL
  Serial.println(msg);
#endif
}
