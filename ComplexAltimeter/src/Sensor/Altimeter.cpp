#include "Altimeter.hpp"
#include "config.h"
#include "DataLogger.hpp"

Altimeter::Altimeter() {}
Altimeter::~Altimeter() {}



void Altimeter::start()
{
  if (barometer.begin(BARO_I2C_ADDR)) {  //Omit the parameter for adafruit
    #ifdef STATUS_PIN_LEVEL
    analogWrite(STATUS_PIN, STATUS_PIN_LEVEL);
    #else
    digitalWrite(STATUS_PIN, HIGH);
    #endif
    digitalWrite(MESSAGE_PIN, LOW);
    DataLogger::log("Barometer Started");
    barometerReady = true;
    refAltitude = barometer.readAltitude(SEA_LEVEL_PRESSURE);
  } else {
    //If the unit starts with the status pin off and the message pin on,
    //the barometer failed to initialize
    DataLogger::log("Barometer Init Fail");
  }
  scanI2cBus();
}

double Altimeter::referenceAltitude()
{
  return refAltitude;
}

double Altimeter::getAltitude()
{
  filter.lastEstimate();
}

double verticalVelocity()
{
  velocityFilter.lastEstimate();
}

void Altimeter::update()
{
  double altIn = barometer.readAltitude(SEA_LEVEL_PRESSURE) - refAltitude;

  long time = millis();
  long delta = time - lastRefreshTime;
  double velocity = (altIn - filter.lastEstimate()) / delta * 1000;
  lastRefreshTime = time;

  velocityFilter.step(velocity);
  filter.step(altIn);
}




bool Altimeter::isReady()
{
  return barometerReady;
}

void Altimeter::scanI2cBus()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknown error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

}

