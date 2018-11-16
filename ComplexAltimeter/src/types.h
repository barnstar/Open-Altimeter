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

struct Vector
{
    float XAxis=0;
    float YAxis=0;
    float ZAxis=0;

    Vector() {}
    
    Vector(float x, float y, float z) {
      XAxis=x; YAxis=y; ZAxis=z;
    }

    double length() {
      return sqrt( (XAxis * XAxis) + (YAxis * YAxis) + (ZAxis * ZAxis));
    }

    Vector operator + (Vector rhs) {
      return Vector(XAxis+rhs.XAxis, YAxis+rhs.YAxis, ZAxis+rhs.ZAxis);
    }

    Vector operator / (float rhs) {
      return Vector(XAxis/rhs, YAxis/rhs, ZAxis/rhs);
    }

   Vector operator * (float rhs) {
      return Vector(XAxis*rhs, YAxis*rhs, ZAxis*rhs);
    }

   String toString() { return String("(" + String(XAxis) +","+ String(YAxis)+","+String(ZAxis) + ")");}
};

typedef struct {
  double altitude =0;
  double acceleration =0;
  Vector acc_vec;

  String toString() { return String("Alt " + String(altitude) +"    acc: " + String(acceleration)); }
}SensorData;

typedef enum {
  kReadyToFly,
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


class ChuteState
{
  public:

  ChuteState::ChuteState() {
    this->reset();
  };

  ChuteState::~ChuteState() {};
  
  bool         deployed ;         //True if the the chute has been deplyed
  int          deploymentTime ;   //Time at which the chute was deployed
  bool         timedReset ;       //True if we've reset the chute relay due to a timeout
  RelayState   relayState = OFF;  //State of the parachute relay pin.  Recorded separately.  To avoid fire.
  EjectionType type;
  Servo        servo;

  int    relayPin=0;
  char   id = 0; 

  void init(int id, int pin, EjectionType type) {
      log("Init Chute " + String(id) + " on pin " + String(pin));
      this->relayPin = pin;
      this->id = id;
      this->type = type;
      switch(type){
        case kPyro:  pinMode(pin, OUTPUT); break;
        case kServo: servo.attach(pin); break;
        case kNoEjection: break;
      }
      reset();
  };

  void enable() {
    deployed = true;
    deploymentTime = millis();
    relayState = ON;
    switch(type){
      case kPyro:  digitalWrite(relayPin, HIGH);; break;
      case kServo: servo.write(kMinServoAngle); break;
      case kNoEjection: break;
    }
    log("Deploying Chute #" + String(id) );
  };

  
  void disable() {
    deployed = false;
    relayState = OFF;
    switch(type){
      case kPyro:  digitalWrite(relayPin, LOW);; break;
      case kServo: servo.write(kMaxServoAngle); break;
      case kNoEjection: break;
    }
    log("Disabling Chute #" + String(id) );
  };

  void reset() {
    deployed = false;
    deploymentTime = 0;
    timedReset = false;
    disable();
  };
  
};





#endif //types_h
