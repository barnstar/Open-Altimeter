//////////////////////////////////////////////////////////////////
// Types

#ifndef types_h
#define types_h

#include <Servo.h>
#include "DataLogger.hpp"
#include "FlightData.hpp"

#define NO_PIN -1

typedef enum {
  kNoEjection,
  kPyro,
  kServo  
} EjectionType;



typedef enum {
  OFF = 0,
  ON =1 
}OnOffState;

typedef OnOffState RelayState;
typedef OnOffState BlinkerState;

typedef struct {
  double altitude =0;
  double acceleration =0;
}SensorData;

typedef enum {
  kInFlight,
  kAscending,
  kDescending,
  kOnGround
} FlightState;

typedef enum {
  kNone,
  kActive,
  kPassive
}PeizoStyle;


typedef struct {
  bool         deployed ;         //True if the the chute has been deplyed
  int          deploymentTime ;   //Time at which the chute was deployed
  bool         timedReset ;       //True if we've reset the chute relay due to a timeout
  RelayState   relayState = OFF;  //State of the parachute relay pin.  Recorded separately.  To avoid fire.
  EjectionType type;
  Servo        servo;
  
  int    relayPin=0;
  int    id = 0; 
  
  void init(int id, int pin, EjectionType type) {
      DataLogger::log("Init Chute " + String(id) + " on pin " + String(pin));
      this->relayPin = pin;
      this->id = id;
      this->type = type;
      switch(type){
        case kPyro:  pinMode(pin, OUTPUT); break;
        case kServo: servo.attach(pin); break;
        case kNoEjection: break;
      }
      reset();
  }

  void enable() {
    deployed = true;
    deploymentTime = millis();
    relayState = ON;
    switch(type){
      case kPyro:  digitalWrite(relayPin, HIGH);; break;
      case kServo: servo.write(5); break;
      case kNoEjection: break;
    }
    DataLogger::log("Deploying Chute #" + String(id) );
  }

  
  void disable() {
    deployed = false;
    relayState = OFF;
    switch(type){
      case kPyro:  digitalWrite(relayPin, LOW);; break;
      case kServo: servo.write(120); break;
      case kNoEjection: break;
    }
    DataLogger::log("Disabling Chute #" + String(id) );
  }

  void reset() {
    deployed = false;
    deploymentTime = 0;
    timedReset = false;
    disable();
  }
}ChuteState;

#endif //types_h
