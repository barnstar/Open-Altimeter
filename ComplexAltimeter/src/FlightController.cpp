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

FlightController::FlightController()
    : resetButton(RESET_PIN, 900),
      inputButton(INPUT_PIN, 1500),
      imu(1000 / SENSOR_READ_DELAY_MS),
      statusView(display.display),
      sensorDataView(display.display),
      historyView(display.display)
{
  SPIFFS.begin();
  Serial.begin(SERIAL_BAUD_RATE);
  DataLogger::sharedLogger();


  DataLogger::log("Creating Flight Controller");
  blinker = new Blinker(MESSAGE_PIN, BUZZER_PIN);
  resetButton.setDelegate(this);
  inputButton.setDelegate(this);

  display.start();
  display.addView(&sensorDataView, false);
  display.addView(&historyView, true);
  display.addView(&statusView, true);

  this->initialize();
  DataLogger::log("Flight Controller Initialized");
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

  mainChute.init(2, MAIN_DEPL_RELAY_PIN, MAIN_TYPE);
  drogueChute.init(1, DROGUE_DEPL_RELAY_PIN, DROGUE_TYPE);

  barometerReady = altimeter.start();
  mpuReady       = imu.start();

  flightData.reset();

  DataLogger::log("Deployment Altitude: " + String(deploymentAltitude));
  DataLogger::log("Pad Altitude:" + String(altimeter.referenceAltitude()));
  DataLogger::log(checkMPUSettings());

  // We don't want to sound the buzzer on first boot, only after a flight
  enableBuzzer = false;

  statusView.setInfo(getStatusData());
  historyView.setHistoryInfo(DataLogger::sharedLogger().apogeeHistory());
  sensorDataView.setWaiting();
}

StatusData const &FlightController::getStatusData()
{
  statusData.deploymentAlt = deploymentAltitude;
  statusData.status        = flightState;
  statusData.baroReady     = barometerReady;
  statusData.mpuReady      = mpuReady;
  statusData.padAltitude   = altimeter.referenceAltitude();
  statusData.lastApogee    = flightData.apogee;
  return statusData;
}

String FlightController::getStatus()
{
  String statusStr = (flightState == kReadyToFly ? "Ready To Fly" : "Waiting");

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

void FlightController::fly() { flightControl(); }

void FlightController::loop()
{
  server.handleClient();
  resetButton.update();
  inputButton.update();

  if (sampleOnNextLoop) {
    flightControl();
    sampleOnNextLoop = false;
  }

  if (flightState == kReadyToFly && altimeter.isReady() &&
      !sensorTicker.active()) {
    DataLogger::log("Starting Ticker");
    blinker->cancelSequence();
    DataLogger::sharedLogger().openFlightDataFileWithIndex(flightCount);
    sensorTicker.attach_ms(1000 / SENSOR_READ_DELAY_MS, readSensors, this);
    digitalWrite(READY_PIN, HIGH);
  }

  // Blink out the last recorded apogee on the message pin
  if (flightState == kOnGround && sensorTicker.active()) {
    DataLogger::log("Stopping Ticker");
    statusView.setInfo(getStatusData());
    sensorDataView.setWaiting();
    historyView.setHistoryInfo(DataLogger::sharedLogger().apogeeHistory());
    sensorTicker.detach();
    digitalWrite(READY_PIN, LOW);
    if (lastApogee) {
      blinker->cancelSequence();
      DataLogger::log(String("Starting Blinker: ") + String(lastApogee));
      blinker->blinkValue(lastApogee, BLINK_SPEED_MS, true);
      lastApogee = 0;
    }
  }
}

void FlightController::setDeploymentAltitude(int altitude)
{
  DataLogger::log("Deployment Altitude Set to " + String(altitude));
  deploymentAltitude = altitude;
}

void FlightController::resetAll()
{
  DataLogger::resetAll();
  flightCount = 0;
  reset();
}

void FlightController::buttonShortPress(ButtonInput *button)
{
  if (button == &resetButton) {
    DataLogger::log("Reset Short Press");
    reset();
  } else if (button == &inputButton) {
    display.nextView();
  }
}

void FlightController::buttonLongPress(ButtonInput *button)
{
  if (button == &resetButton) {
    DataLogger::log("Reset Long Press");
    flightState = kOnGround;
  } else if (button == &inputButton) {
  }
}

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

  setDeploymentRelay(OFF, drogueChute);
  drogueChute.reset();
  setDeploymentRelay(OFF, mainChute);
  mainChute.reset();

  testFlightTimeStep = 0;
  blinker->cancelSequence();
  enableBuzzer = true;
  ///playReadyTone();
  enableBuzzer = false;
  flightState  = kReadyToFly;
  DataLogger::log("Ready To Fly...");
  statusView.setInfo(getStatusData());
}

void FlightController::playReadyTone()
{
  blinker->blinkValue(3, 400, false);
}

Vector FlightController::getacceleration() { return Vector(0, 0, 0); }

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
    d->altitude = altimeter.altitude();
    d->verticalVelocity = altimeter.verticalVelocity();
  }
  imu.update();
  d->heading      = imu.getHeading();
  d->acc_vec      = imu.getAcceleration();
  d->gyro_vec     = imu.getGyro();
  d->acceleration = d->acc_vec.length();
}

void FlightController::flightControl()
{
  SensorData d;
  readSensorData(&d);
  double acceleration = d.acceleration;
  double altitude     = d.altitude;

  FlightDataPoint dp = FlightDataPoint(millis(), altitude, acceleration);

  // Log every 5 samples when going fast and every 20 when in a slow descent.
  int sampleDelay = (flightState != kDescending) ? 5 : 20;
  logCounterUI    = !logCounterUI ? sampleDelay : logCounterUI - 1;
  if (0 == logCounterUI && flightState != kOnGround) {
    DataLogger::log("Alt:" + String(altitude) + "  " + d.heading.toString() +
                    +"   " + d.acc_vec.toString());
    sensorDataView.setData(d);
  }

  // Log slightly more to the file system
  sampleDelay = (flightState != kDescending) ? 3 : 6;
  logCounterLogger = !logCounterUI ? 3 : logCounterUI - 1;
  if (0 == logCounterLogger && flightState != kOnGround) {
    DataLogger::sharedLogger().logDataPoint(dp, false);
  }

  // Keep track or our apogee and our max g load
  flightData.apogee          = MAX(flightData.apogee, altitude);
  flightData.maxAcceleration = MAX(flightData.maxAcceleration, acceleration);

  // Experimental.  Log when we've hit some prescribed g load.  This might be
  // more accurate than starting the flight at some altitude x...  The minimum
  // load will probably have to be too small and will pick up things like wind
  // gusts though.
  if (flightState == kReadyToFly && acceleration > FLIGHT_START_THRESHOLD_ACC &&
      flightData.accTriggerTime == 0) {
    flightData.accTriggerTime = millis() - resetTime;
  }

  if (flightState == kReadyToFly && altitude > FLIGHT_START_THRESHOLD_ALT) {
    // Transition to "InFlight" if we've exceeded the threshold altitude.
    DataLogger::log("Flight Started");
    flightState               = kAscending;
    flightData.altTriggerTime = millis() - resetTime;
    // For testing - to indicate we're in the ascending mode
    digitalWrite(READY_PIN, LOW);
    digitalWrite(MESSAGE_PIN, HIGH);
    statusView.setInfo(getStatusData());
  } else if (flightState == kAscending &&
             altitude < (flightData.apogee - DESCENT_THRESHOLD)) {
    // Transition to kDescendining if we've we're DESCENT_THRESHOLD meters below
    // our apogee
    DataLogger::log("Descending");
    flightState = kDescending;
    // Deploy our drogue chute
    setDeploymentRelay(ON, drogueChute);
    flightData.drogueEjectionAltitude = altitude;
    statusView.setInfo(getStatusData());

  } else if (flightState == kDescending &&
             altitude < FLIGHT_END_THRESHOLD_ALT) {
    flightState = kOnGround;
    DataLogger::log("Landed");
    lastApogee = flightData.apogee;

    DataLogger::log(flightData.toString(flightCount));
    DataLogger::sharedLogger().endDataRecording(flightData, flightCount);
    server.bindFlight(flightCount);

    DataLogger::log("Resetting Relays");
    resetChuteIfRequired(drogueChute);
    resetChuteIfRequired(mainChute);
    DataLogger::log("Relays Reset");

    statusView.setInfo(getStatusData());
    enableBuzzer = true;
  }

  // Main chute deployment at kDeployment Altitude
  if ((flightState == kDescending) &&
      !mainChute.deployed && altitude < deploymentAltitude) {
    // If we're descening and we're below our deployment altitude, deploy the
    // chute!
    flightData.ejectionAltitude = altitude;
    DataLogger::log("Deploy Main");
    setDeploymentRelay(ON, mainChute);
  }

  // Safety measure in case we don't switch to the onGround state.  This will
  // disable the igniter relay after 5 seconds to avoid draining or damaging the
  // battery
  checkChuteIgnitionTimeout(mainChute, MAX_FIRE_TIME);
  checkChuteIgnitionTimeout(drogueChute, MAX_FIRE_TIME);
}

void FlightController::resetChuteIfRequired(RecoveryDevice &c)
{
  if (c.type == kPyro) {
    setDeploymentRelay(OFF, c);
    c.reset();
  }
}

void FlightController::checkChuteIgnitionTimeout(RecoveryDevice &c,
                                                 int maxIgnitionTime)
{
  if (!c.timedReset && c.deployed &&
      millis() - c.deploymentTime > maxIgnitionTime && c.type == kPyro) {
    int chuteId = c.id;
    setDeploymentRelay(OFF, c);
    c.timedReset = true;
  }
}

void FlightController::setDeploymentRelay(RelayState relayState,
                                          RecoveryDevice &c)
{
  if (relayState == c.relayState) return;
  int chuteId = c.id;

  switch (relayState) {
    case ON:
      c.enable();
      break;
    case OFF:
      c.disable();
      break;
  }
}

void FlightController::testFlightData(SensorData *d)
{
  if (testFlightTimeStep == 1) {
    testFlightTimeStep    = 2;
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

String FlightController::checkMPUSettings()
{
  //   if(!mpuReady) {
  //     return String("IMU is not ready");
  //   }
  //
  //   String settings;
  //
  //   settings += " * Sleep Mode: ";
  //   settings += mpu.getSleepEnabled() ? "Enabled" : "Disabled";
  //
  //   settings +=  " *<br>\n  Clock Source: ";
  //   switch(mpu.getClockSource())
  //   {
  //     case MPU6050_CLOCK_KEEP_RESET:     settings +="Stops the clock and
  //     keeps the timing generator in reset"; break; case
  //     MPU6050_CLOCK_EXTERNAL_19MHZ: settings +="PLL with external 19.2MHz
  //     reference"; break; case MPU6050_CLOCK_EXTERNAL_32KHZ: settings +="PLL
  //     with external 32.768kHz reference"; break; case
  //     MPU6050_CLOCK_PLL_ZGYRO:      settings +="PLL with Z axis gyroscope
  //     reference"; break; case MPU6050_CLOCK_PLL_YGYRO:      settings +="PLL
  //     with Y axis gyroscope reference"; break; case MPU6050_CLOCK_PLL_XGYRO:
  //     settings +="PLL with X axis gyroscope reference"; break; case
  //     MPU6050_CLOCK_INTERNAL_8MHZ:  settings +="Internal 8MHz oscillator";
  //     break;
  //   }
  //
  //   Serial.print("<br>\n * Accelerometer:         ");
  //   switch(mpu.getRange())
  //   {
  //     case MPU6050_RANGE_16G:            settings +="+/- 16 g"; break;
  //     case MPU6050_RANGE_8G:             settings +="+/- 8 g"; break;
  //     case MPU6050_RANGE_4G:             settings +="+/- 4 g"; break;
  //     case MPU6050_RANGE_2G:             settings +="+/- 2 g"; break;
  //   }
  //
  //   settings +="<br>\n  * Accelerometer offsets: ";
  //   settings +=String(mpu.getAccelOffsetX());
  //   settings +=" / ";
  //   settings +=String(mpu.getAccelOffsetY());
  //   settings +=" / ";
  //   settings +=String(mpu.getAccelOffsetZ());
  //
  //   settings += "<br>\n";
  //   return settings;
}
