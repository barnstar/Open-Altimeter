/*********************************************************************************
 * Simple Alitimeter 
 * 
 * Mid power rocket avionics software for alitidue recording and dual deployment
 * 
 * Copyright 2018, Jonathan Nobels
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of 
 * this software and associated documentation files (the "Software"), to deal in the 
 * Software without restriction, including without limitation the rights to use, copy, 
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the 
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **********************************************************************************/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "types.h"

#define LOG_TO_SERIAL 1   //Set to 0 to disable serial logging...
#define PLOT_ALTITUDE 0   //Set to 1 to watch the altitude on the serial plotter

#define USE_BMP085 1
#define USE_BMP280 0

static const int kMinServoAngle = 5;
static const int kMaxServoAngle = 120;

//////////////////////////////////////////////////////////////////
// Configuration

//Today's pressure at sea level...


//Recording will start at FLIGHT_START_THRESHOLD_ALT m and we'll assume we're on the 
//ground at FLIGHT_END_THRESHOLD_ALT m.
//In theory, these could be lower, but we want to account for landing in a tree,
//on a hill, etc.  30m should be sufficient for most launch sites.
const double FLIGHT_START_THRESHOLD_ALT = 30;
const double FLIGHT_END_THRESHOLD_ALT   = 30;
const double FLIGHT_START_THRESHOLD_ACC = 0.1;  //in G's

//When the altitude is DESCENT_THRESHOLD meters less than the apogee, we'll assume we're 
//descending.  Hopefully, your rocket has a generally upwards trajectory....
const double DESCENT_THRESHOLD = 20;

//The deployment relay will be deactivated after this time.
const int MAX_FIRE_TIME = 5000;

//When grounded the reset pin will cancel the last apogee display and
//prepare the alitmiter for the next flight.  If it is grounded on boot
//the eeprom will be erased.

#define USE_PIN_CONFIG_1 0
#define USE_PIN_CONFIG_2 0
#define USE_PIN_CONFIG_3 1
#define USE_PIN_CONFIG_4 0


#if USE_PIN_CONFIG_1
//Configuration A: 1 1/2" PCB
const int SERIAL_BAUD_RATE      = 57600;
const int STATUS_PIN            = 4;   //Unit status pin.  On if OK
const int MESSAGE_PIN           = 2;   //Blinks out the altitude
const int READY_PIN             = 13;  //Inicates the unit is ready for flight
const int BUZZER_PIN            = 8;   //Audible buzzer on landing
const int RESET_PIN             = 6;
const int TEST_PIN              = 7;
const int MAIN_DEPL_RELAY_PIN   = 12;   //parachute deployment pin
const int DROGUE_DEPL_RELAY_PIN = 11;  //parachute deployment pin
const int ALT_PIN_A             = 9;  //Main Chute Alitude Altitude Set Pin.
const int ALT_PIN_B             = 10;  //Main Chute Alitude Altitude Set Pin
const DeploymentType MAIN_TYPE    = kPyro;
const DeploymentType DROGUE_TYPE  = kPyro; 
const int BARO_I2C_ADDR         = 0x77;
const PeizoStyle PEIZO_TYPE     = kActive;
#define ENABLE_MPU 0
#elif USE_PIN_CONFIG_2
//Configuration B: 2" PCB w. Servo Sled
const int SERIAL_BAUD_RATE      = 57600;
const int STATUS_PIN            = 4;   //Unit status pin.  On if OK
const int MESSAGE_PIN           = 5;   //Blinks out the altitude
const int READY_PIN             = 6;   //Inicates the unit is ready for flight
const int BUZZER_PIN            = 3;   //Audible buzzer on landing
const int RESET_PIN             = 7;
const int TEST_PIN              = 12;
const int MAIN_DEPL_RELAY_PIN   = 11;  //parachute deployment pin
const int DROGUE_DEPL_RELAY_PIN = 10;  //parachute deployment pin
const int ALT_PIN_A             = 8;   //Main Chute AlitudeAltitude Set Pin.
const int ALT_PIN_B             = 9;   //Main Chute Alitude Altitude Set Pin
const DeploymentType MAIN_TYPE    = kServo;
const DeploymentType DROGUE_TYPE  = kServo; 
const int BARO_I2C_ADDR         = 0x76;
const PeizoStyle PEIZO_TYPE     = kPassive;
#define ENABLE_MPU 0
#elif USE_PIN_CONFIG_3
//Configuration B: Small PCB with servo pinout
const int SERIAL_BAUD_RATE      = 57600;
const int STATUS_PIN            = 5;   //Unit status pin.  On if OK
const int MESSAGE_PIN           = 3;   //Blinks out the altitude
const int READY_PIN             = 13;   //Inicates the unit is ready for flight
const int BUZZER_PIN            = 2;   //Audible buzzer on landing
const int RESET_PIN             = 4;
const int TEST_PIN              = 10;
const int MAIN_DEPL_RELAY_PIN   = 11;  //parachute deployment pin
const int DROGUE_DEPL_RELAY_PIN = 12;  //parachute deployment pin
const int ALT_PIN_A             = 8;   //Main Chute AlitudeAltitude Set Pin.
const int ALT_PIN_B             = 9;   //Main Chute Alitude Altitude Set Pin
const DeploymentType MAIN_TYPE    = kServo;
const DeploymentType DROGUE_TYPE  = kNoEjection; 
#define BARO_I2C_ADDR
#define STATUS_PIN_LEVEL 800
const PeizoStyle PEIZO_TYPE     = kPassive;
#define ENABLE_MPU 0
#endif

//The barometer can only refresh at about 50Hz. 
const int SENSOR_READ_DELAY_MS = 5;

//Delay between digit blinks.  Any faster is too quick to keep up with
const int BLINK_SPEED_MS       = 200;

#endif //CONFIGURATION_H
