#include "FlightController.hpp"
#include "types.h"
#include "config.h"
#include "DataLogger.hpp"
#include <FS.h>

#define kMaxBlinks 64

FlightController::FlightController()
{
  SPIFFS.begin();
  Serial.begin(SERIAL_BAUD_RATE);
  DataLogger::sharedLogger();

  DataLogger::log("Creating Flight Controller");
  this->initialize();

}

FlightController::~FlightController(){
  delete blinker;
}

static FlightController *sharedInstance = nullptr;

FlightController& FlightController::shared()
{
  if(nullptr == sharedInstance) {
    sharedInstance = new FlightController();
  }
  return *sharedInstance;
}

void FlightController::initialize()
{
  DataLogger::log("Initializing Flight Controller");

  blinker = new Blinker(MESSAGE_PIN, BUZZER_PIN);
  IPAddress serverAddress(IPAddress(192,4,0,1));
  server.setAddress(serverAddress);

  pinMode(RESET_PIN, INPUT_PULLUP);

  //All LED pins sset to outputs
  pinMode(MESSAGE_PIN, OUTPUT);
  pinMode(STATUS_PIN,  OUTPUT);
  pinMode(READY_PIN,   OUTPUT);
  pinMode(BUZZER_PIN,  OUTPUT);

  //Start in the "error" state.  Status pin should be high and message
  //pin should be low to indicate a good startup
  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(MESSAGE_PIN, HIGH);

  mainChute.init(2, MAIN_DEPL_RELAY_PIN, MAIN_TYPE);
  drogueChute.init(1, DROGUE_DEPL_RELAY_PIN, DROGUE_TYPE);

  altimeter.start();
  flightData.reset();

  DataLogger::log("Deployment Altitude: " + String(deploymentAltitude));
  DataLogger::log("Pad Altitude:" + String(altimeter.referenceAltitude()));


  //We don't want to sound the buzzer on first boot, only after a flight
  enableBuzzer = false;
}

 String FlightController::getStatus()
 {
    String ret = "Controller Status<br/>";
    String statusStr = (readyToFly ?"Ready To Fly" : "Waiting");
    ret += "Status :" + statusStr + "<br/>";
    ret += "Flight Count :" +  String(flightCount) + "<br/>";
    ret += "Deployment Altitude: " + String(deploymentAltitude) + "<br/>";
    ret += "Pad Altitude:" + String(altimeter.referenceAltitude()) + "<br/>";
    if(!readyToFly) {
      ret += "Last Flight:" + flightData.toString(flightCount) + "<br/>";
    }
    return ret;
 }

void readSensors(FlightController *f)
{
  f->fly();
}

void FlightController::fly()
{
  SensorData data;
  readSensorData(&data);
  flightControl(&data);
}

void FlightController::loop()
{
  server.handleClient();

  if(readyToFly && altimeter.isReady()) {
    if(!sensorTicker.active()) {
      DataLogger::sharedLogger().clearBuffer();
      sensorTicker.attach_ms(50, readSensors, this);
      digitalWrite(READY_PIN, HIGH);
      DataLogger::log("Ready to Fly");
    }
  }

  //Blink out the last recorded apogee on the message pin
  if (!readyToFly && flightState == kOnGround) {
    sensorTicker.detach();
    digitalWrite(READY_PIN, LOW);
    blinkLastAltitude();
  }
}

void FlightController::resetAll()
{
  DataLogger::resetAll();
  flightCount = 0;
  reset();
}

void FlightController::reset()
{
  if(!readyToFly) {
    flightCount = DataLogger::sharedLogger().nextFlightIndex();
    flightData.reset();
    resetTime = millis();
    readyToFly = true;
    setDeploymentRelay(OFF, drogueChute);
    drogueChute.reset();
    setDeploymentRelay(OFF, mainChute);
    mainChute.reset();
    testFlightTimeStep = 0;
    enableBuzzer = true;
    playReadyTone();
    enableBuzzer = false;
    DataLogger::log("Ready To Fly...");
  }
}

void FlightController::blinkLastAltitude()
{
  if(blinker->isBlinking()) {
    return;
  }

  static Blink sequence[kMaxBlinks];
  int tempApogee = flightData.apogee;
  if(tempApogee < 30){
    return;
  }
  bool foundDigit = false;         //Don't blink leading 0s
  int n=0;
  for(int m=100000; m>0; m=m/10) {  //If we make it past 99km, we're in trouble :)
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
  blinker->blinkSequence(sequence, n+1, true);
}

void FlightController::playReadyTone()
{
  static Blink sequence[3] = {{100,100},{100,100},{100,100}};
  blinker->blinkSequence(sequence, 3, false);
}



double FlightController::getacceleration()
{
    return 0;
  //mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  int16_t grx, gry, grz;
  const double onegee = 65536.0 / 16.0; //units/g for scale of +/- 8g


  double axd = ax / onegee ;
  double ayd = ay / onegee ;
  double azd = az / onegee ;

  //Remove gravity.. We now have the absolute acceleration
  double acc = sqrt( axd * axd + ayd * ayd + azd * azd) - 1.0;
  return acc;
}

void FlightController::runTest()
{
  if(readyToFly && testFlightTimeStep == 0) {
    testFlightTimeStep=1;
  }
}

void FlightController::readSensorData(SensorData *d)
{
  if(testFlightTimeStep) {
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


void FlightController::flightControl(SensorData *d)
{
  double acceleration = d->acceleration;
  double altitude = d->altitude;

  FlightDataPoint dp = FlightDataPoint(millis(), altitude, acceleration);
  DataLogger::sharedLogger().logDataPoint(dp, false);

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
    DataLogger::sharedLogger().logDataPoint(dp, true);
  }
  else if (flightState == kAscending && altitude < (flightData.apogee - DESCENT_THRESHOLD)) {
    //Transition to kDescendining if we've we're DESCENT_THRESHOLD meters below our apogee
    DataLogger::log("Descending");
    flightState = kDescending;

    //Deploy our drogue chute
    setDeploymentRelay(ON, drogueChute);
    flightData.drogueEjectionAltitude = altitude;
  }
  else if (flightState == kDescending && altitude < FLIGHT_END_THRESHOLD_ALT) {
    flightState = kOnGround;
    DataLogger::log("Landed");

    DataLogger::log(flightData.toString(flightCount));
    DataLogger::sharedLogger().saveFlight(flightData, flightCount);
    server.bindFlight(flightCount);

    //Reset the chutes, reset the relays if we haven't already.  Start the locator
    //beeper and start blinking...

    setDeploymentRelay(OFF, drogueChute);
    drogueChute.reset();

    setDeploymentRelay(OFF, mainChute);
    mainChute.reset();

    enableBuzzer = true;
    readyToFly = false;
  }

  //Main chute deployment at kDeployment Altitude
  if (flightState == kDescending && !mainChute.deployed && altitude < deploymentAltitude) {
    //If we're descening and we're below our deployment altitude, deploy the chute!
    flightData.ejectionAltitude = altitude;
    setDeploymentRelay(ON, mainChute);
  }

  //Safety measure in case we don't switch to the onGround state.  This will disable the igniter relay
  //after 5 seconds to avoid draining or damaging the battery
  checkChuteIgnitionTimeout(mainChute, MAX_FIRE_TIME);
  checkChuteIgnitionTimeout(drogueChute, MAX_FIRE_TIME);
}


void FlightController::checkChuteIgnitionTimeout(ChuteState &c, int maxIgnitionTime)
{
  //We only need to timeout pyrocharges
  if(!c.type == kPyro) {
    return;
  }
  
    if (!c.timedReset && c.deployed &&
        millis() - c.deploymentTime > maxIgnitionTime)
    {
      int chuteId = c.id;
      DataLogger::log("Chute #" + String(chuteId) + " Timeout");
      setDeploymentRelay(OFF, c);
      c.timedReset = true;
    }
}


void FlightController::setDeploymentRelay(RelayState relayState, ChuteState &c)
{
  if(relayState == c.relayState)return;
  int chuteId = c.id;

   switch(relayState) {
    case ON:
      c.enable();
      break;
    case OFF:
      c.disable();
      break;
  }
}


///////////////////////////////////////////////////////////////////
// Test Flight Generator
// When you ground the TEST_PIN, the unit will initate a test flight
//

void FlightController::testFlightData(SensorData *d)
{
  if (testFlightTimeStep == 1) {
    testFlightTimeStep = 2;
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

