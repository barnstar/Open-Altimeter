#ifndef RECOVERYDEVICE_H
#define RECOVERYDEVICE_H

#include <Servo.h>
#include "types.h"

class RecoveryDevice
{
 public:
  RecoveryDevice() { this->reset(); };

  ~RecoveryDevice(){};

  bool deployed      = false;  // True if the the chute has been deplyed
  int deploymentTime = 0;      // Time at which the chute was deployed
  bool timedReset =
      false;  // True if we've reset the chute relay due to a timeout
  RelayState relayState = OFF;  // State of the parachute relay pin.  Recorded
                                // separately.  To avoid fire.
  DeploymentType type = kServo;
  Servo servo;

  byte relayPin = 0;
  byte id       = 0;

 public:
  void init(byte id, byte pin, DeploymentType type);
  void enable();
  void disable();
  void reset();
};

#endif  // RECOVERYDEVICE_H
