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
 *
 *
 * Components Required:
 *
 * An Arduino.. Nano preferably
 * A few LEDs and some current limiting resistors
 * A momentary switch
 * A BMP-280 compatible altimeter (Adafruit works)
 * Optional: An active peizo buzzer
 * Optional: Relay breakout boards for dual deployment
 * A battery and power switch - 9V (lipo, alk, or nihm) or even 2 cr2025 button cells
 * Wire/perfboard, etc to connect all this together
 *
 *
 * - Records apogee, deployment altitude for hundreds of flights (but who's kidding who,
 *   you'll lose this long before it makes it 100 flights)
 * - Trigger drogue depoloyment at apogee
 * - Triggers main deployment relay at a predefined altitude
 * - Plays an audible buzzer on landing to aid in locating
 * - Blinks out the last recorded apogee
 * - All flight data will be recored to EEPROM anddumped to the serial port
 *   on startup.
 *
 * For deployment, the depoloyment pins should be connected to a relay which
 * fires the deployment charge(s).
 *
 * All signal PINs should be connected to LEDs via an 330ohm (or larger) resistor
 * The reset pin should be connected to ground via a momentary switch
 *
 * This is designed for an Adafruit-compatible BMP280 board connected via
 * i2c.  On a nano that's Clock->A5 and Data->A4.  Other barometer sensors
 * should work just as well.  Cheap MBP280s require you to add the 0x76 paramter
 * to correct the i2c addresss.
 *
 * An optional MPU6050 can be attached which will recored the maximum accelleration
 * and a triggered acceleration initiation event time.
 *
 * See comments on the various pin-outs for operation
 *
 *********************************************************************************/
#define VERSION 1

#include "FlightController.hpp"

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);

  DataLogger::sharedLogger();
  FlightController::sharedInstance().initialize();
}

void loop()
{
  FlightController::sharedInstance().loop();
}

