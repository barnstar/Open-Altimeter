# OpenAltimeter

## Concept

This was originally a project to build a simple altimeter to record
the apogee of mid power rocket flights... Then things got out of hand
as they are liable to do in this hobby........

The complex version runs on  an ESP8266 (or compatible) and includes 
sensor fusion to calculate spatial orientation, pyro and servo channels,
a web server and an  oled display to provide a "nice" user interface.

The "simple" version is designed for an Arduino nano and has no
interface beyond the serial logger and the indicator LEDs.

## Hardware

To build this, you'll need an Arduino - a nano is best or an ESP8266.
An ESP12 should also work but hasn't been tested.

The pinouts are noted in Configuration.h.  There are several
LEDs for indicators, a pin for an active peizo and two pins which
can be used for firing ematches via an NPN mosfet.  Two pins are also
allocated to control servos for parachute deployment (and/or for active
flight control for the ambitious).

The complex version also includes support for Oled displays via i2c.
A second switch can be used to navigate between the screens.

Notes:
- Power can be supplied from a 2s lipo.  Both the arduino nano
  and ESP8266 based boards like the Node MCU v1.0 will happily run off
  of an ~7.4V supply.   If using a nano with only a barometer, you
  can also use a pair of lithium button cells to construct a unit not 
  much larger than a a commercial altimeter.
- For deployment, it is advised to add a ~1A 5V regulator to drive
  the servos and/or mosfets.  You can get away with powering a small
  piezo and the indicator LEDs directly from the logic pin but be
  cautious.  Use a transistor switch for anything that exceeds the 
  single pin current rating for whichever micro controller you choose.
  You may be able to drive a small servo from the arduino 5V Vout as
  well.  This will not work for any board with 3.3V logic.


...Happy Flying



