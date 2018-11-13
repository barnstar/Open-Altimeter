

FlightControl::FlightControl(){

}


void FlightControl::begin()
{
	//All the config stuff
}

int FlightControl::readDeploymentAltitude()
{
  pinMode(ALT_PIN_A, INPUT_PULLUP);
  pinMode(ALT_PIN_B, INPUT_PULLUP);

  byte a = (digitalRead(ALT_PIN_A) == LOW) ? 1 : 0;
  byte b = (digitalRead(ALT_PIN_B) == LOW) ? 1 : 0;

  static const int altitudes[] = { 100, 150, 150, 200 };

  byte val = a + b*2;
  if(val > 3)val = 0;
  return altitudes[val]; 
}

double FlightControl::getacceleration()
{
}

void FlightControl::readSensorData(SensorData &d)
{
  if(TEST_PIN && ((digitalRead(TEST_PIN) == LOW) || testFlightTimeStep)) {
    testFlightData(d);
    return;
  }
  
  //Our relative altitude... Relative to wherever we last reset the altimeter.
  if(barometerReady) {
    d.altitude = barometer.readAltitude(SEA_LEVEL_PRESSURE) - refAltitude;
  }
  if(mpuReady) {
    d.acceleration = getacceleration();
  }
}


void FlightControl::flightControl(SensorData &d)
{
  double acceleration = d.acceleration;
  double altitude = filter.step(d.altitude);

  if(PLOT_ALTITUDE) { log(String(altitude)); }

  //Failsafe.. Nothing should happen while we're ready but the altitude is below our threshold
  if(flightState == kReadyToFly && altitude < FLIGHT_START_THRESHOLD_ALT){
    return;
  }
  
  //Keep track or our apogee and our max g load
  if( altitude > flightData.apogee ){
    flightData.apogee = altitude;
    flightData.apogeeTime = millis() - resetTime;
  }
  flightData.maxAcceleration = acceleration > flightData.maxAcceleration ? acceleration : flightData.maxAcceleration;
  
  //Experimental.  Log when we've hit some prescribed g load.  This might be more accurate than starting the
  //flight at some altitude 
  if(flightState == kReadyToFly && acceleration > FLIGHT_START_THRESHOLD_ACC && flightData.accTriggerTime == 0) {
    flightData.accTriggerTime = millis() - resetTime;
  }

  if (flightState == kReadyToFly && altitude >= FLIGHT_START_THRESHOLD_ALT) {
    //Transition to "InFlight" if we've exceeded the threshold altitude.
    log("Flight Started");
    flightState = kAscending;
    flightData.altTriggerTime = millis() - resetTime;
    //For testing - to indicate we're in the ascending mode
    digitalWrite(READY_PIN, LOW);
    digitalWrite(MESSAGE_PIN, HIGH);
  } 
  else if (flightState == kAscending && altitude < (flightData.apogee - DESCENT_THRESHOLD)) {
    //Transition to kDescendining if we've we're DESCENT_THRESHOLD meters below our apogee
    log("Descending");
    flightState = kDescending;

    //Deploy our drogue chute
    setDeploymentRelay(ON, &drogueChute);
    flightData.drogueEjectionAltitude = altitude;
  }
  else if (flightState == kDescending && altitude < FLIGHT_END_THRESHOLD_ALT) {
    flightState = kOnGround;
    log(String("Landed"));
    
    logData(flightCount, &flightData);
    recordFlight(flightData);
    flightCount++;

    //Reset the pyro charges.  Leave chute releases open.  Start the locator
    //beeper and start blinking...
    resetChuteIfRequired(drogueChute);
    resetChuteIfRequired(mainChute);
  }

  //Main chute deployment at kDeployment Altitude.  
  //We deploy in the onGround state as well just in case an anomalous pressure spike has transitioned
  //us to that state while we're still in the air
  if ((flightState == kDescending  || flightState == kOnGround) && !mainChute.deployed && altitude <= deploymentAltitude) {
    //If we're descening and we're below our deployment altitude, deploy the chute!
    flightData.ejectionAltitude = altitude;
    setDeploymentRelay(ON, mainChute);
  }
  
  //Safety measure in case we don't switch to the onGround state.  This will disable the igniter relay
  //after 5 seconds to avoid draining or damaging the battery.  This only applies for pyro type.
  checkChuteIgnitionTimeout(mainChute, MAX_FIRE_TIME);
  checkChuteIgnitionTimeout(drogueChute, MAX_FIRE_TIME);
}

void FlightControl::resetChuteIfRequired(RecoveryDevice &c)
{
  if(c.type == kPyro) {
     setDeploymentRelay(OFF, c);
     c.reset();
  }
}

void FlightControl::checkChuteIgnitionTimeout(RecoveryDevice &c, int maxIgnitionTime)
{
    if (!c.timedReset && c.deployed &&
        millis() - c.deploymentTime > maxIgnitionTime &&
        c.type == kPyro) 
    {
      log("Chute #" + String(c.id) + " Timeout");
      setDeploymentRelay(OFF, c);
      c.timedReset = true;
    }
}


void FlightControl::setDeploymentRelay(RelayState relayState, RecoveryDevice &c)
{
  if(relayState == c.relayState)return;
  
   switch(relayState) {
    case ON:
      c.enable();
      break;
    case OFF:
      c.disable();
      break;
  }
}