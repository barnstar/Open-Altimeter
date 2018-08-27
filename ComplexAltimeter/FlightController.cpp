 #include "FlightController.hpp"
#include "DataLogger.hpp"
#include <FS.h>

#include "types.h"
#include "config.h"

#define kMaxBlinks 64


FlightController::FlightController()
{
  SPIFFS.begin();
  Serial.begin(SERIAL_BAUD_RATE);
  DataLogger::sharedLogger();

  DataLogger::log("Creating Flight Controller");
  blinker = new Blinker(MESSAGE_PIN, BUZZER_PIN);

  this->initialize();
}


FlightController::~FlightController(){
  delete blinker;
}


FlightController& FlightController::shared()
{
  static FlightController sharedInstance;
  return sharedInstance;
}


void FlightController::initialize()
{
  DataLogger::log("Initializing Flight Controller");

  IPAddress serverAddress(IPAddress(192,4,0,1));
  server.start(serverAddress);

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
  if(!(mpuReady = mpu.begin(MPU6050_SCALE_250DPS, MPU6050_RANGE_8G))) {
    DataLogger::log("IMU Startup failed");
  }


  flightData.reset();

  DataLogger::log("Deployment Altitude: " + String(deploymentAltitude));
  DataLogger::log("Pad Altitude:" + String(altimeter.referenceAltitude()));
  DataLogger::log(checkMPUSettings());

  //We don't want to sound the buzzer on first boot, only after a flight
  enableBuzzer = false;
}

String FlightController::getStatus()
{
  String statusStr = (readyToFly ?"Ready To Fly" : "Waiting");

  String ret;
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

  if(flightState == kReadyToFly && altimeter.isReady()) {
    if(!sensorTicker.active()) {
      sensorTicker.attach_ms(50, readSensors, this);
      digitalWrite(READY_PIN, HIGH);
    }
  }

  //Blink out the last recorded apogee on the message pin
  if (flightState == kOnGround) {
    sensorTicker.detach();
    digitalWrite(READY_PIN, LOW);
    blinkLastAltitude();
  }
}

void FlightController::setDeploymentAltitude(int altitude)
{
  DataLogger::log("Deployment Altitude Set to " + String(altitude));
  deploymentAltitude  = altitude;
}

void FlightController::resetAll()
{
  DataLogger::resetAll();
  flightCount = 0;
  reset();
}


void FlightController::reset()
{
  if(flightState = kReadyToFly) {
    return;
  }


  flightCount = DataLogger::sharedLogger().nextFlightIndex();
  flightData.reset();
  resetTime = millis();

  setDeploymentRelay(OFF, drogueChute);
  drogueChute.reset();
  setDeploymentRelay(OFF, mainChute);
  mainChute.reset();

  testFlightTimeStep = 0;

  enableBuzzer = true;
  playReadyTone();
  enableBuzzer = false;
  flightState = kReadyToFly;
  DataLogger::sharedLogger().clearBuffer();
  DataLogger::log("Ready To Fly...");
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


Vector FlightController::getacceleration()
{
  Vector v = mpu.readNormalizeAccel();
  //Vector v = mpu.readRawAccel();

  return v;
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
    d->acc_vec = getacceleration();
    d->acceleration = d->acc_vec.length() - 9.80065;
  }
}


void FlightController::flightControl(SensorData *d)
{
  double acceleration = d->acceleration;
  double altitude = d->altitude;

  FlightDataPoint dp = FlightDataPoint(millis(), altitude, acceleration);

  if(logCounter == 0) { 
    DataLogger::log(String("Alt " + String(altitude) +"    acc: " + d->acc_vec.toString() + "  acc mag:" +  String(d->acceleration))); 
    if(!flightState == kOnGround && !flightState == kReadyToFly) {
      DataLogger::sharedLogger().logDataPoint(dp, false);
    }
  }

  //Log every 5 samples when going fast and every 20 when in a slow descent.
  int sampleDelay = (flightState != kDescending) ? 5 : 20; 
  logCounter = !logCounter ? sampleDelay : logCounter-1;


  //Keep track or our apogee and our max g load
  flightData.apogee = altitude > flightData.apogee ? altitude : flightData.apogee;
  flightData.maxAcceleration = acceleration > flightData.maxAcceleration ? acceleration : flightData.maxAcceleration;

  //Experimental.  Log when we've hit some prescribed g load.  This might be more accurate than starting the
  //flight at some altitude x...  The minimum load will probably have to be too small and will pick up things like
  //wind gusts though.
  if(flightState == kReadyToFly && acceleration > FLIGHT_START_THRESHOLD_ACC && flightData.accTriggerTime == 0) {
    flightData.accTriggerTime = millis() - resetTime;
  }

  if (flightState == kReadyToFly && altitude > FLIGHT_START_THRESHOLD_ALT) {
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

    resetChuteIfRequired(drogueChute);
    resetChuteIfRequired(mainChute);

    enableBuzzer = true;
    readyToFly = false;
  }

  //Main chute deployment at kDeployment Altitude
  if (flightState == kDescending || flightState == kOnGround && !mainChute.deployed && altitude < deploymentAltitude) {
    //If we're descening and we're below our deployment altitude, deploy the chute!
    flightData.ejectionAltitude = altitude;
    DataLogger::log("Deploy Main");
    setDeploymentRelay(ON, mainChute);
  }

  //Safety measure in case we don't switch to the onGround state.  This will disable the igniter relay
  //after 5 seconds to avoid draining or damaging the battery
  checkChuteIgnitionTimeout(mainChute, MAX_FIRE_TIME);
  checkChuteIgnitionTimeout(drogueChute, MAX_FIRE_TIME);
}

void FlightController::resetChuteIfRequired(ChuteState &c)
{
  if(c.type == kPyro) {
     setDeploymentRelay(OFF, c);
     c.reset();
  }
}


void FlightController::checkChuteIgnitionTimeout(ChuteState &c, int maxIgnitionTime)
{
  //We only need to timeout pyrocharges
  if(!c.type == kPyro) {
    return;
  }

    if (!c.timedReset && c.deployed &&
        millis() - c.deploymentTime > maxIgnitionTime &&
        c.type == kPyro)
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

String FlightController::checkMPUSettings()
{
  if(!mpuReady) {
    return String("IMU is not ready");
  }

  String settings;

  settings += " * Sleep Mode: ";
  settings += mpu.getSleepEnabled() ? "Enabled" : "Disabled";

  settings +=  " *<br>\n  Clock Source: ";
  switch(mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     settings +="Stops the clock and keeps the timing generator in reset"; break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: settings +="PLL with external 19.2MHz reference"; break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: settings +="PLL with external 32.768kHz reference"; break;
    case MPU6050_CLOCK_PLL_ZGYRO:      settings +="PLL with Z axis gyroscope reference"; break;
    case MPU6050_CLOCK_PLL_YGYRO:      settings +="PLL with Y axis gyroscope reference"; break;
    case MPU6050_CLOCK_PLL_XGYRO:      settings +="PLL with X axis gyroscope reference"; break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  settings +="Internal 8MHz oscillator"; break;
  }

  Serial.print("<br>\n * Accelerometer:         ");
  switch(mpu.getRange())
  {
    case MPU6050_RANGE_16G:            settings +="+/- 16 g"; break;
    case MPU6050_RANGE_8G:             settings +="+/- 8 g"; break;
    case MPU6050_RANGE_4G:             settings +="+/- 4 g"; break;
    case MPU6050_RANGE_2G:             settings +="+/- 2 g"; break;
  }

  settings +="<br>\n  * Accelerometer offsets: ";
  settings +=String(mpu.getAccelOffsetX());
  settings +=" / ";
  settings +=String(mpu.getAccelOffsetY());
  settings +=" / ";
  settings +=String(mpu.getAccelOffsetZ());

  settings += "<br>\n";
  return settings;
}

