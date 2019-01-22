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

#include "FlightController.hpp"
#include <FS.h>
#include "DataLogger.hpp"

#include "../Configuration.h"
#include "types.h"

#define kMaxBlinks 64

FlightController::FlightController() : imu(1000 / SENSOR_READ_DELAY_MS)
{
  SPIFFS.begin();

  Serial.begin(SERIAL_BAUD_RATE);
  DataLogger::sharedLogger();

  DataLogger::log("Creating Flight Controller");
  blinker = new Blinker(MESSAGE_PIN, BUZZER_PIN);
  DataLogger::log("Creating Flight Controller --");

  this->initialize();
  DataLogger::log("Flight Controller Initialized");

  bool success = false;
  int deplAlt  = settings.readIntValue(F("DepAlt"), &success);
  if (success) {
    DataLogger::log("Read Deployment Altitude");
    deploymentAltitude = deplAlt;
  } else {
    DataLogger::log("Set default deployment altitude 100");
    settings.writeIntValue(100, F("DepAlt"));
  }
}

FlightController::~FlightController() { delete blinker; }

FlightController &FlightController::shared()
{
  static FlightController sharedInstance;
  return sharedInstance;
}

void FlightController::initialize()
{
  DataLogger::log("Initializing Flight Controller");

  IPAddress serverAddress(IPAddress(192, 4, 0, 1));
  server.start(serverAddress);

  pinMode(RESET_PIN, INPUT_PULLUP);

  // All LED pins sset to outputs
  pinMode(MESSAGE_PIN, OUTPUT);
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(READY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Start in the "error" state.  Status pin should be high and message
  // pin should be low to indicate a good startup
  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(MESSAGE_PIN, HIGH);

  barometerReady = altimeter.start();
  mpuReady       = imu.start();

  flightData.reset();

  DataLogger::log("Deployment Altitude: " + String(deploymentAltitude));
  DataLogger::log("Pressure:" + String(altimeter.getRefPressure()));
  // We don't want to sound the buzzer on first boot, only after a flight
  enableBuzzer = false;
}

void FlightController::initRecoveryDevices()
{
  bool didRead = false;
  int onAngle  = settings.readIntValue("servoOnAngle", &didRead);
  if (!didRead) {
    onAngle = kChuteReleaseTriggeredAngle;
  }

  didRead      = false;
  int offAngle = settings.readIntValue("servoOffAngle", &didRead);
  if (!didRead) {
    offAngle = kChuteReleaseArmedAngle;
  }

  RecoveryDevice::setOffAngle(offAngle, false);
  RecoveryDevice::setOnAngle(onAngle, false);

  for(int i=0;i<4;i++) {
    devices[i] = new RecoveryDevice();
  }

  devices[0]->init(ControlChannel1, DEPL_CTL_1, CTL_1_TYPE);
  devices[1]->init(ControlChannel2, DEPL_CTL_2, CTL_2_TYPE);
  devices[2]->init(ControlChannel3, DEPL_CTL_3, CTL_3_TYPE);
  devices[3]->init(ControlChannel4, DEPL_CTL_4, CTL_4_TYPE);

  // These should be software configurable
  setMainChannel(ControlChannel1);
  setDrogueChannel(ControlChannel2);
}

void FlightController::setMainChannel(int channel)
{
  mainChute = getRecoveryDevice(channel);
}

void FlightController::setDrogueChannel(int channel)
{
  drogueChute = getRecoveryDevice(channel);
}

RecoveryDevice* FlightController::getRecoveryDevice(int channel)
{
  // Array location is 0 indexed... Channels are 1 indexed.
  return devices[channel - 1];
}

StatusData const& FlightController::getStatusData()
{
  statusData.deploymentAlt     = deploymentAltitude;
  statusData.status            = flightState;
  statusData.baroReady         = barometerReady;
  statusData.mpuReady          = mpuReady;
  statusData.padAltitude       = altimeter.referenceAltitude();
  statusData.lastApogee        = flightData.apogee;
  statusData.referencePressure = altimeter.getRefPressure();

  return statusData;
}

String FlightController::getStatus()
{
  String statusStr =
      (flightState == kReadyToFly ? F("Ready To Fly") : F("Waiting"));

  String ret;
  ret += "Status :" + statusStr + "<br/>";
  ret += "Flight Count :" + String(flightCount) + "<br/>";
  ret += "Deployment Altitude: " + String(deploymentAltitude) + "<br/>";
  ret += "Pad Altitude:" + String(altimeter.referenceAltitude()) + "<br/>";
  if (flightState != kReadyToFly) {
    ret += "Last Flight:" + flightData.toString(flightCount) + "<br/>";
  }
  return ret;
}

void readSensors(FlightController *f)
{
  if (f != nullptr) {
    f->sampleOnNextLoop = true;
  }
}

void FlightController::loop()
{
  if (!interfaceStarted) {
    interfaceStarted = true;
    initRecoveryDevices();
    userInterface.start();
  }
  // Ignore the wifis and oled when we're flying.
  if (flightState == kReadyToFly || flightState == kOnGround) {
    server.handleClient();
    userInterface.eventLoop(true);
  }else{
    userInterface.eventLoop(false);
  }

  if (sampleOnNextLoop) {
    flightControl();
    sampleOnNextLoop = false;
  }

  if (flightState == kReadyToFly && altimeter.isReady() &&
      !sensorTicker.active()) {
    DataLogger::log(F("Starting Ticker"));
    blinker->cancelSequence();
    blinker->blinkValue(2, 300, true, false);
    DataLogger::sharedLogger().openFlightDataFileWithIndex(flightCount);
    sensorTicker.attach_ms(SENSOR_READ_DELAY_MS, readSensors, this);
    digitalWrite(READY_PIN, HIGH);
    flightControl();
  }

  if (flightState == kAscending && blinker->isBlinking()) {
    blinker->cancelSequence();
  }

  // Blink out the last recorded apogee on the message pin
  if (flightState == kOnGround && sensorTicker.active()) {
    DataLogger::log(F("Stopping Ticker"));
    sensorTicker.detach();
    flightControl();
    blinker->cancelSequence();
    digitalWrite(READY_PIN, LOW);
    if (lastApogee) {
      DataLogger::log(String(F("Starting Blinker: ")) + String(lastApogee));
      blinker->blinkValue(lastApogee, BLINK_SPEED_MS, true);
      lastApogee = 0;
    }
  }
}

void FlightController::setDeploymentAltitude(int altitude)
{
  DataLogger::log("Deployment Altitude Set to " + String(altitude));
  deploymentAltitude = altitude;
  settings.writeIntValue(deploymentAltitude, F("DepAlt"));
}

void FlightController::resetAll()
{
  DataLogger::resetAll();
  flightCount = 0;
  reset();
}

void FlightController::stop() { flightState = kOnGround; }

void FlightController::reset()
{
  if (flightState == kReadyToFly) {
    return;
  }

  flightCount = DataLogger::sharedLogger().nextFlightIndex();
  flightData.reset();
  resetTime = millis();
  altimeter.reset();
  imu.reset();

  setRecoveryDeviceState(OFF, drogueChute);
  drogueChute.reset();
  setRecoveryDeviceState(OFF, mainChute);
  mainChute.reset();

  testFlightTimeStep = 0;
  blinker->cancelSequence();
  flightState = kReadyToFly;
  DataLogger::log(F("Ready To Fly..."));
}

void FlightController::runTest()
{
  if (flightState == kReadyToFly && testFlightTimeStep == 0) {
    testFlightTimeStep = 1;
  }
}

void FlightController::readSensorData(SensorData *d)
{
  if (testFlightTimeStep) {
    testFlightData(d);
    return;
  }

  // Our relative altitude... Relative to wherever we last reset the altimeter.
  if (altimeter.isReady()) {
    altimeter.update();
    d->altitude         = altimeter.altitude();
    d->verticalVelocity = altimeter.verticalVelocity();
  }
  imu.update();
  d->heading      = imu.getRelativeHeading();
  d->acc_vec      = imu.getAcceleration();
  d->gyro_vec     = imu.getGyro();
  d->acceleration = d->acc_vec.length();
}

void FlightController::flightControl()
{
  long t = millis();
  readSensorData(&sensorData);
  double acceleration = sensorData.acceleration;
  double altitude     = sensorData.altitude;

  FlightDataPoint dp = FlightDataPoint(t, altitude, acceleration);

  // Log every 5 samples when going fast and every 20 when in a slow descent.
  int sampleDelay = (flightState != kDescending) ? 5 : 20;
  logCounterUI    = !logCounterUI ? sampleDelay : logCounterUI - 1;
  if (0 == logCounterUI && flightState != kOnGround) {
    // DataLogger::log("Alt:" + String(altitude) + "  " +
    //                 sensorData.heading.toString() + +"   " +
    //                 sensorData.acc_vec.toString());
  }

  DataLogger::sharedLogger().logDataPoint(dp, false);

  // Keep track or our apogee and our max g load
  flightData.apogee          = MAX(flightData.apogee, altitude);
  flightData.maxAcceleration = MAX(flightData.maxAcceleration, acceleration);

  if (flightState == kReadyToFly && acceleration > FLIGHT_START_THRESHOLD_ACC) {
    DataLogger::log("Flight Started - ACC Trigger");
    flightState               = kAscending;
    flightData.accTriggerTime = t - resetTime;
    digitalWrite(READY_PIN, LOW);
    digitalWrite(MESSAGE_PIN, HIGH);
  }

  if (flightState == kAscending && acceleration < 11.0 &&
      flightData.burnoutTime == 0) {
    // We're on the way up, but gravity has taken over... We're now coasting.
    flightData.burnoutTime     = t - resetTime;
    flightData.burnoutAltitude = altitude;
  }

  if (flightState == kReadyToFly && altitude > FLIGHT_START_THRESHOLD_ALT) {
    // Transition to "InFlight" if we've exceeded the threshold altitude.
    DataLogger::log(F("Flight Started"));
    flightState               = kAscending;
    flightData.altTriggerTime = t - resetTime;
    // For testing - to indicate we're in the ascending mode
    digitalWrite(READY_PIN, LOW);
    digitalWrite(MESSAGE_PIN, HIGH);
    DataLogger::sharedLogger().logDataPoint(dp, true);
  } else if (flightState == kAscending &&
             altitude < (flightData.apogee - DESCENT_THRESHOLD)) {
    // Transition to kDescendining if we've we're DESCENT_THRESHOLD meters below
    // our apogee
    DataLogger::log(F("Descending"));
    flightState = kDescending;
    // Deploy our drogue chute
    setRecoveryDeviceState(ON, drogueChute);
    flightData.drogueEjectionAltitude = altitude;
    DataLogger::sharedLogger().logDataPoint(dp, false);
  } else if (flightState == kDescending &&
             altitude < FLIGHT_END_THRESHOLD_ALT) {
    flightState = kOnGround;
    DataLogger::log(F("Landed"));
    lastApogee = flightData.apogee;

    DataLogger::log(flightData.toString(flightCount));
    DataLogger::sharedLogger().endDataRecording(flightData, flightCount);
    server.bindFlight(flightCount);

    DataLogger::log(F("Resetting Pyro"));
    resetRecoveryDeviceIfRequired(drogueChute);
    resetRecoveryDeviceIfRequired(mainChute);
    DataLogger::log(F("Relays Reset"));
    enableBuzzer = true;
  }

  // Main chute deployment at kDeployment Altitude
  if ((flightState == kDescending) && !mainChute->deployed &&
      altitude < deploymentAltitude) {
    // If we're descening and we're below our deployment altitude, deploy the
    // chute!
    flightData.ejectionAltitude = altitude;
    DataLogger::log(F("Deploy Main"));
    setRecoveryDeviceState(ON, mainChute);
  }

  // Safety measure in case we don't switch to the onGround state.  This will
  // disable the igniter relay after 5 seconds to avoid draining or damaging the
  // battery
  checkChuteIgnitionTimeout(mainChute, MAX_FIRE_TIME);
  checkChuteIgnitionTimeout(drogueChute, MAX_FIRE_TIME);
}

void FlightController::resetRecoveryDeviceIfRequired(RecoveryDevice *c)
{
  if (c.type == kPyro) {
    setRecoveryDeviceState(OFF, c);
    c->reset();
  }
}

void FlightController::checkChuteIgnitionTimeout(RecoveryDevice *c,
                                                 int maxIgnitionTime)
{
  if (!c.timedReset && c.deployed &&
      millis() - c->deploymentTime > maxIgnitionTime && c->type == kPyro) {
    setRecoveryDeviceState(OFF, c);
    c->timedReset = true;
  }
}

void FlightController::setRecoveryDeviceState(RecoveryDeviceState deviceState,
                                              RecoveryDevice *c)
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

void FlightController::testFlightData(SensorData *d)
{
  if (testFlightTimeStep == 1) {
    testFlightTimeStep    = 2;
    fakeData.altitude     = 0;
    fakeData.acceleration = 0;
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
