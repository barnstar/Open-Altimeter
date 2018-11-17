
//////////////////////////////////////////////////////////////////
// Configuration
#ifndef config_h
#define config_h

#include <Arduino.h>
#include "src/types.h"

//Today's pressure at sea level...
const double SEA_LEVEL_PRESSURE = 1013.7;

//Recording will start at FLIGHT_START_THRESHOLD_ALT m and we'll assume we're on the 
//ground at FLIGHT_END_THRESHOLD_ALT m.
//In theory, these could be lower, but we want to account for landing in a tree,
//on a hill, etc.  30m should be sufficient for most launch sites.
const double FLIGHT_START_THRESHOLD_ALT = 30;
const double FLIGHT_END_THRESHOLD_ALT   = 30;
const double FLIGHT_START_THRESHOLD_ACC = 0.1;  //in G's

//When the altitude is DESCENT_THRESHOLD meters less than the apogee, we'll assume we're 
//descending.  Hopefully, your rocket has a generally upwards trajectory....
const double DESCENT_THRESHOLD = 15;

//The deployment relay will be deactivated after this time.
const int MAX_FIRE_TIME = 5000;

//When grounded the reset pin will cancel the last apogee display and
//prepare the alitmiter for the next flight.  If it is grounded on boot
//the eeprom will be erased.


const int SERIAL_BAUD_RATE      = 74880;
const int STATUS_PIN            = D0;   //Unit status pin.  On if OK
const int MESSAGE_PIN           = D3;   //Blinks out the altitude 
const int READY_PIN             = D4;   //Inicates the unit is ready for flight
const int BUZZER_PIN            = D6;   //Audible buzzer on landing
const int RESET_PIN             = 9;    //SD2
const int MAIN_DEPL_RELAY_PIN   = D5;  //parachute deployment pin
const int DROGUE_DEPL_RELAY_PIN = NO_PIN;  //parachute deployment pin
const DeploymentType MAIN_TYPE    = kServo;
const DeploymentType DROGUE_TYPE  = kNoEjection; 
const int BARO_I2C_ADDR         = 0x76;
#define STATUS_PIN_LEVEL 800
const PeizoStyle PEIZO_TYPE     = kActive;

//The barometer can only refresh at about 50Hz. 
const int SENSOR_READ_DELAY_MS = 20;

//Delay between digit blinks.  Any faster is too quick to keep up with
const int BLINK_SPEED_MS       = 250;

const int kMaxServoAngle = 180;
const int kMinServoAngle = 0;

#endif //config_h

