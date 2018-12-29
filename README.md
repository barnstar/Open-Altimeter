# OpenAltimeter

## Concept

This was originally a project to build a simple altimeter to record
the apogee of mid power rocket flights.  Beyond recording altitude,
this "simple" altimeter can now control dual deployment at a variety
of altitudes and record accelleration.

On flight completion, the altitude will be indicated visually and 
audibly.  Flights are recorded to EEProm and logged to the serial
port.

There are two variants.  A "simple" version with basic logging and
deployment capabilities designed for low end Amtel devices (like
an arduino nano or micro) and a "complex" version with much more
capability designed to run on an ESP8266 based board.

## Hardware

To build this, you'll need an Arduino - a nano is best.  The pinouts
are noted in the source code and are configurable.  There are several
LEDs for indicators, a pin for an active peizo and two pins which
can be connected to relay boards for firing ematches.  An reset pin
should be connected to a momentary switch (to silence the peizo and
prepare the unit for flight).

### BOM
- Aruduino Nano or an ESP8266 based board
- Optional OLED display (for the complex variant)
- BMP280/180 Barometer/Altimeter or an MPU9250 IMU
- 2-3 LEDS (you can use the internal LED for one of the indicators)
- One or two momentary switches (normally open).
- 2-3 330 or 470 Ohm resistors for the LEDs (up to 4.7K to dim the LEDs if wanted)
- Optional: Active Peizo Buzzer
- A suitable power source
- Wire, perfboard, etc


### Wiring
- See Schematic.png.  The pin-outs differ from what is in the code.  
  The various digital pins should be wired to their respective components and the 
  constants updated to reflect your chosen layout.  One of the digital pins acts as
  a control for the unit.  Ground this via a momentary.
- For the i2c sensor(s), connect A4 to the SDA (data_ pin on the sensors and connect A5 to the 
  SCK pin (clock) on the sensors.  If your chosen sensors lack internal pull-up resistors, wire
  and additional ~470Kohm resistor between the SDA and SDK lines and ground.
- Give the sensors 3.3V power and a ground.


...Happy Flying



