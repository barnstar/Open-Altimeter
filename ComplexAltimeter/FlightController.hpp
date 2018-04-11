#ifndef FlightController_h
#define FlightController_h

#include "WebServer.h"
#include "Altimeter.h"
#include "Blinker.h"
#include <Ticker.h>

#include "config.h"
#include "types.h"

class FlightController
{
public:
     static FlightController& shared();

     FlightController();
     ~FlightController();

    void initialize();
    void loop();

    WebServer server;
    Altimeter altimeter;

private:
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
    bool   mpuReady = false;              //True if the barometer/altimeter is ready


    Blinker blinker(MESSAGE_PIN, BUZZER_PIN);
    Ticker  sensorTicker;

    SensorData fakeData;
    double testApogee = 400;
    bool isTestAscending;

    void fly();
    bool checkResetPin();
    void blinkLastAltitude();
    void playReadyTone();
    double getacceleration();
    void readSensorData(SensorData *d);
    void flightControl(SensorData *d);
    void checkChuteIgnitionTimeout(ChuteState &c, int maxIgnitionTime);
    void setDeploymentRelay(RelayState relayState, ChuteState &c);
    void recordFlight(FlightData d);
    void configureEeprom();
    int getFlightCount();
    void testFlightData(SensorData *d);

}

#endif