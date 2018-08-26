#ifndef FlightController_h
#define FlightController_h

#include <Arduino.h>

#include "WebServer.hpp"
#include "Altimeter.hpp"
#include "BlinkSequence.hpp"
#include "MPU6050.h"
#include <Ticker.h>

#include "config.h"
#include "types.h"

class FlightController
{
public:
    static FlightController& shared();

    FlightController();
    ~FlightController();

    FlightController(FlightController const&)        = delete;

    void loop();

    void setDeploymentAltitude(int altitude);
    String checkMPUSettings();

    WebServer server;
    Altimeter altimeter;
    MPU6050 mpu;

    void fly();
    String getStatus();

    void reset();
    void runTest();
    void resetAll();

    void readSensorData(SensorData *d);

private:
    void initialize();

    FlightData flightData;
    FlightState flightState = kOnGround;  //The flight state

    ChuteState mainChute;
    ChuteState drogueChute;

    int    flightCount = 0;               //The number of flights recorded in EEPROM
    int    resetTime = 0;                 //millis() after starting the current flight
    bool   readyToFly = false;            //switches to false at the end of the flight.  Resets on reset.
    bool   enableBuzzer = false;          //True if the buzzer should be sounding
    int    testFlightTimeStep = 0;
    bool   mpuReady = false;              //True if the barometer/altimeter is ready
    double deploymentAltitude = 100;      //Deployment altitude in ft.


    Blinker *blinker;
    Ticker  sensorTicker;
    int logCounter;

    SensorData fakeData;
    double testApogee = 400;
    bool isTestAscending;

    bool checkResetPin();
    void blinkLastAltitude();
    void playReadyTone();
    Vector getacceleration();
    void flightControl(SensorData *d);
    void checkChuteIgnitionTimeout(ChuteState &c, int maxIgnitionTime);
    void setDeploymentRelay(RelayState relayState, ChuteState &c);
    void recordFlight(FlightData &d);
    void testFlightData(SensorData *d);

};

#endif
