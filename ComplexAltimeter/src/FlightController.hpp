
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

#ifndef FlightController_h
#define FlightController_h

#include <Arduino.h>

#include <Ticker.h>
#include "IO/Blinker.hpp"
#include "RecoveryDevice.h"
#include "Sensor/Altimeter.hpp"
#include "Sensor/Imu.hpp"
#include "WebServer.hpp"

#include "../Configuration.h"
#include "IO/UserInterface.h"
#include "Settings.hpp"
#include "types.h"

class FlightController
{
 public:
  static FlightController &shared();

  FlightController();
  ~FlightController();

  FlightController(FlightController const &) = delete;

  void loop();

  void setDeploymentAltitude(int altitude);
  int deploymentAltitude = 100;  // Deployment altitude in m

  WebServer server;

  String getStatus();
  StatusData const &getStatusData();

  SensorData sensorData;
  void readSensorData(SensorData *d);

  FlightState flightState = kOnGround;  // The flight state
  FlightData flightData;

  void reset();
  void stop();

  void runTest();
  void resetAll();

  bool sampleOnNextLoop = false;

  RecoveryDevice *getRecoveryDevice(int channel);

 private:
  void initialize();

  RecoveryDevice *devices[4];
  void initRecoveryDevices();
  void setMainChannel(int channel);
  void setDrogueChannel(int channel);

  Settings settings;

  Altimeter altimeter;
  Imu imu;

  int lastApogee        = 0;
  bool refreshInterface = false;

  RecoveryDevice *mainChute;
  RecoveryDevice *drogueChute;

  int flightCount        = 0;      // The number of flights recorded in EEPROM
  int resetTime          = 0;      // millis() after starting the current flight
  bool enableBuzzer      = false;  // True if the buzzer should be sounding
  int testFlightTimeStep = 0;
  bool mpuReady          = false;  // True if the barometer/altimeter is ready
  bool barometerReady    = false;  // True if the barometer/altimeter is ready

  Blinker *blinker;
  Ticker sensorTicker;

  int logCounterUI     = 0;
  int logCounterLogger = 0;

  UserInterface userInterface;
  boolean interfaceStarted = false;

  StatusData statusData;

  SensorData fakeData;
  double testApogee = 400;
  bool isTestAscending;
  void failsafeCheck();
  bool checkResetPin();
  void blinkLastAltitude();

  void flightControl();

  void checkChuteIgnitionTimeout(RecoveryDevice *c, int maxIgnitionTime);
  void setRecoveryDeviceState(RecoveryDeviceState deviceState,
                              RecoveryDevice *c);
  void resetRecoveryDeviceIfRequired(RecoveryDevice *c);

  void recordFlight(FlightData &d);
  void testFlightData(SensorData *d);
};

#endif
