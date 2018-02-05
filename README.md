# SimpleAltimeter

## Concept

This was originally a project to build a simple altimeter to record
the apogee of mid power rocket flights.  Beyond recording altitude,
this "simple" altimeter can now control dual deployment at a variety
of altitudes and record accelleration.

On flight completion, the altitude will be indicated visually and 
audibly.  Flights are recorded to EEProm and logged to the serial
port.

Data logging is limited with only an Arduino Nano, but adding an
SD card would give you full data logging capability.

## Hardware

To build this, you'll need an Arduino - a nano is best.  The pinouts
are noted in the source code and are configurable.  There are several
LEDs for indicators, a pin for an active peizo and two pins which
can be connected to relay boards for firing ematches.  An reset pin
should be connected to a momentary switch (to silence the peizo and
prepare the unit for flight).

### BOM
- Aruduino Nano
- BMP280 Barometer/Altimeter
- 2-3 LEDS (you can use the internal LED for one of the indicators)
- 2-3 330 or 470 Ohm resistors for the LEDs
- Optional: MPU6050 accelerometer
- Optional: Active Peizo Buzzer
- A battery.  Either a 9V or a pair of CR2032 Lithium Cells
- Wire, perfboard, etc

### Wiring
- Connect the LEDs to the pins set in the source then to ground via the resitors.
- Connect the piezo to the piezo pin and ground it
- Connect A4 to the SDA pin on the sensors
- Connect A5 to the SCK pin on the sensors
- Give the sensors 3.3V power and a ground
- Optionally ground A5 and A4 via a 470kOhm pullup resistor
- Add some power and go burn some money

...Happy Flying



