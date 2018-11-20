#ifndef FlightController_h
#define FlightController_h

#include <Arduino.h>

#include <Ticker.h>
#include "IO/Blinker.hpp"
#include "IO/ButtonInput.h"
#include "RecoveryDevice.h"
#include "Sensor/Altimeter.hpp"
#include "Sensor/Imu.hpp"
#include "WebServer.hpp"

#include "config.h"
#include "types.h"

class FlightController : public ButtonInputDelegate
{
 public:
  static FlightController &shared();

  FlightController();
  ~FlightController();

  FlightController(FlightController const &) = delete;

  void loop();

  void setDeploymentAltitude(int altitude);
  String checkMPUSettings();

  WebServer server;
  Altimeter altimeter;
  Imu imu;

  void fly();
  String getStatus();

  void reset();
  void runTest();
  void resetAll();

  void readSensorData(SensorData *d);
  bool sampleOnNextLoop = false;

 private:
  void initialize();

  FlightData flightData;
  FlightState flightState = kOnGround;  // The flight state

  RecoveryDevice mainChute;
  RecoveryDevice drogueChute;

  int flightCount        = 0;      // The number of flights recorded in EEPROM
  int resetTime          = 0;      // millis() after starting the current flight
  bool enableBuzzer      = false;  // True if the buzzer should be sounding
  int testFlightTimeStep = 0;
  bool mpuReady          = false;   // True if the barometer/altimeter is ready
  double deploymentAltitude = 100;  // Deployment altitude in ft.

  Blinker *blinker;
  Ticker sensorTicker;
  int logCounter;

  ButtonInput resetButton;

  SensorData fakeData;
  double testApogee = 400;
  bool isTestAscending;

  void buttonShortPress(ButtonInput *button) override;
  void buttonLongPress(ButtonInput *button) override;

  bool checkResetPin();
  void blinkLastAltitude();
  void playReadyTone();
  Vector getacceleration();
  void flightControl();
  void checkChuteIgnitionTimeout(RecoveryDevice &c, int maxIgnitionTime);
  void setDeploymentRelay(RelayState relayState, RecoveryDevice &c);
  void recordFlight(FlightData &d);
  void testFlightData(SensorData *d);
  void resetChuteIfRequired(RecoveryDevice &c);
};

#endif
