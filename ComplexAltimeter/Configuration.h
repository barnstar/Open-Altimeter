
/*********************************************************************************
 * Open Altimeter
 *
 * Mid power rocket avionics software for altitude recording and dual deployment
 *
 * Copyright 2018, Jonathan Nobels
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **********************************************************************************/

#ifndef config_h
#define config_h

#include <Arduino.h>
#include "src/types.h"

// There are two cheap types of OLED displays.  Choose the one that doesn't
// output gargabe.  I2C ony supported here.  #define the one you're using.
#define USE_SH1106
//#define USE_SDD1306

// Display width and height for the SDD1306 constructor
#define kDispWidth 128
#define kDispHeight 64

// Set this to 0 to disable the display while in flight
#define RUN_DISPLAY_WHILE_FLYING 0

// Recording will start at FLIGHT_START_THRESHOLD_ALT m and we'll assume we're
// on the ground at FLIGHT_END_THRESHOLD_ALT m. In theory, these could be lower,
// but we want to account for landing in a tree, on a hill, etc.  30m should be
// sufficient for most launch sites.
const double FLIGHT_START_THRESHOLD_ALT      = 10;
const double FLIGHT_END_THRESHOLD_ALT        = 30;
const double FLIGHT_START_THRESHOLD_ACC      = 20.0;  // in mss
const double FLIGHT_START_THRESHOLD_VELOCITY = 0.5;  // m/s
const double FLIGHT_END_THRESHOLD_VELOCITY   = 0.0;  // m/s

// When the altitude is DESCENT_THRESHOLD meters less than the apogee, we'll
// assume we're descending.  Hopefully, your rocket has a generally upwards
// trajectory....
const double DESCENT_THRESHOLD = 15;

// Maximum on time for pyro type deployment
const int MAX_FIRE_TIME = 5000;

// ESP8266 specific
#define SD2 9
#define SD3 10

// D1 & D2 are used for i2c
const int SERIAL_BAUD_RATE = 57600;
const byte STATUS_PIN      = NO_PIN;  // Unit status pin.  On if OK
const byte MESSAGE_PIN     = D6;      // Blinks out the altitude
const byte READY_PIN       = D5;      // Indicates the unit is ready for flight
const byte BUZZER_PIN      = D0;      // Audible buzzer on landing
const byte RESET_PIN       = SD2;     // SD2 - pin "9"
const byte INPUT_PIN       = SD3;     // SD3 - pin "10"

const byte DEPL_CTL_1           = D3;  // control pin 1
const byte DEPL_CTL_2           = D4;  // control pin 2
const byte DEPL_CTL_3           = D7;  // control pin 1
const byte DEPL_CTL_4           = D8;  // control pin 2
const RecoveryDeviceType CTL_1_TYPE = kServo;  //PWM control
const RecoveryDeviceType CTL_2_TYPE = kServo;  //PWM control 
const RecoveryDeviceType CTL_3_TYPE = kPyro;   //On/off
const RecoveryDeviceType CTL_4_TYPE = kPyro;   //On/off


const int BARO_I2C_ADDR     = 0x76;  // 0x77 or 0x76
const int DISPLAY_I2C_ADDR  = 0x3C;
const int IMU_I2C_ADDR      = 0x68;
const PeizoStyle PEIZO_TYPE = kActive;

// The barometer can only refresh at about 50Hz.
const int SENSOR_READ_DELAY_MS = 10;

// Delay between digit blinks.  Any faster is too quick to keep up with
const int BLINK_SPEED_MS = 250;

// Servo angles for servo chute release
static const int kChuteReleaseTriggeredAngle = 35;
static const int kChuteReleaseArmedAngle = 178;

#endif  // config_h
