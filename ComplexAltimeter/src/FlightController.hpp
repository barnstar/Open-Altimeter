#ifndef FlightController_h
#define FlightController_h

#include <Arduino.h>

#include "WebServer.hpp"
#include "Sensor/Altimeter.hpp"
#include "IO/Blinker.hpp"
#include "Sensor/Imu.h"
#include <Ticker.h>

#include "config.h"
#include "types.h"

#define SENSOR_FREQUENCY 100  //Hz


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
    Imu imu( 1000 / SENSOR_FREQUENCY );

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
    void resetChuteIfRequired(ChuteState &c)
};

#endif
