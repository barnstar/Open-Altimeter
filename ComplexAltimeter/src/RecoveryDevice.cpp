#include "RecoveryDevice.h"
#include "../config.h"
#include "types.h"

void RecoveryDevice::init(byte id, byte pin, DeploymentType type)
{
  // log("Init RD " + String(id) + " p:" + String(pin));
  this->relayPin = pin;
  this->id       = id;
  this->type     = type;
  switch (type) {
    case kPyro:
      pinMode(pin, OUTPUT);
      break;
    case kServo:
      servo.attach(pin);
      break;
    case kNoEjection:
      break;
  }
  reset();
};

void RecoveryDevice::enable()
{
  deployed       = true;
  deploymentTime = millis();
  relayState     = ON;
  switch (type) {
    case kPyro:
      digitalWrite(relayPin, HIGH);
      ;
      break;
    case kServo:
      servo.write(kMinServoAngle);
      break;
    case kNoEjection:
      break;
  }
  // log("RD En" + String(id) );
};

void RecoveryDevice::disable()
{
  deployed   = false;
  relayState = OFF;
  switch (type) {
    case kPyro:
      digitalWrite(relayPin, LOW);
      ;
      break;
    case kServo:
      servo.write(kMaxServoAngle);
      break;
    case kNoEjection:
      break;
  }
  // log("RD Dis" + String(id) );
};

void RecoveryDevice::reset()
{
  deployed       = false;
  deploymentTime = 0;
  timedReset     = false;
  disable();
};
