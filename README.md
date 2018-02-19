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
- One or two momentary switches (normally open).
- 2-3 330 or 470 Ohm resistors for the LEDs (up to 4.7K to dim the LEDs if wanted)
- Optional: MPU6050 accelerometer
- Optional: Active Peizo Buzzer
- A battery.  Either a 9V or a pair of CR2032 Lithium Cells
- Wire, perfboard, etc


### Wiring
- See Schematic.png.  The pin-outs differ from what is in the code.  
  The various digital pins should be wired to their respective components and the 
  constants updated to reflect your chosen layout.  One of the digital pins acts as
  a control for the unit.  Ground this via a momentary.
- For the i2c sensor(s), connect A4 to the SDA (data_ pin on the sensors and connect A5 to the 
  SCK pin (clock) on the sensors.  If your chosen sensors lack internal pull-up resistors, wire
  and additional ~470Kohm resistor between the SDA and SDK lines and ground.
- Give the sensors 3.3V power and a ground

### Usage
- NOTE: When we're talking about the reset switch here, we're talking about the switch
  wired to the specified reset pin, *not* the Arudino reset pin (though you should wire an
  external switch for that too if you're not using an Arduino with one built in).
- This guide doesn't get into dual deployment, but pinouts are provided which are set high
  for 5 seconds at apogee and a deployment altitude.  The deployment altitude can be set
  by grounding different pins (the schematic shows DIP switches).  Alternatively, to save
  space and weight, you can update the deployment altitude with a reflash. 
- On first boot with a fresh Arduino, you'll need to zero the EEPROM.  To clear the eprom
  power the unit up with the reset pin grounded.
- The status LED should be lit if all the sensors are ready.
- The message LED will be fully on if the altimeter didn't initialize correctly.  If the
  unit is initialized correctly, and a previous flight exists, the message LED will blink
  out the last altitude.
- NOTE: 10 blinks means "0"  so a pattern of **** ** **********  means 420 meters.  Between
  read-outs, the unit will buzz/blink a long tone.
- To put the unit into ready mode, ground the reset pin until the ready light activates.
- If both the status pin and ready LED are on, the model is ready to fly.  Don't forget
  to place the unit where the pressure is properly equalized 
- On landing the piezo and message LED will beep/blink our the last recorded apogee.  
- To silence the unit, hold the reset button (this will put it into ready mode again.)
- To log all the recorded flights, start he unit while connected to a terminal via USB
  All flights saved in the EEPROM will be logged to the serial port.  To clear the unit,
  hold reset on start to zero the EEPROM.

...Happy Flying



