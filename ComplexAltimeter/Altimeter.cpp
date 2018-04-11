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
}

double Altimeter::getAltitude()
{
  return barometer.readAltitude(SEA_LEVEL_PRESSURE) - refAltitude;
}

bool Altimeter::isReady()
{
  return barometerReady;
}
