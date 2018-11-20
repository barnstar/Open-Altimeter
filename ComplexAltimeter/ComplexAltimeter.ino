/*********************************************************************************
 * Complex Alitimeter
 *
 * Mid power rocket avionics software for alitidue recording and dual deployment
 *
 * Copyright 2018, Jonathan Nobels
 *
 *
 */

#define VERSION 1

#include "src/FlightController.hpp"

void setup()
{
  //Initialize the shared flightController
  FlightController::shared();
}

void loop()
{
  FlightController::shared().loop();
}
