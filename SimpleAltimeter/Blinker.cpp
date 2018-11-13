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

#include "Blinker.hpp"
#include "types.h"


void Blinker::blinkValue(long value, int speed, bool repeat)
{
  if(isBlinking()) {
    cancelSequence();
  }

  if(value > 999999) {
    return;
  }

  position = 0;
  memset(bitMap, 0, kBitMapLen);

  long tempValue = value;
  bool foundDigit = false;         //Don't blink leading 0s

  //If we make it past 999km then this failing is probably not a big deal
  //Bitmask should be 10101000 10101010 0010100011 <- For 342... For example
  for(long m=1000000; m>0; m=m/10) {  
    byte digit = tempValue / m;
    if (digit || foundDigit){
      foundDigit = true;
      tempValue = tempValue - digit*m;
      if(digit == 0) { digit = 10; } //Blink "0" as 10
      for(byte i=0;i<digit;i++) {
        byte byteNumber = position / 8;
        byte bitNumber = position % 8;
        bitMap[byteNumber] = bitMap[byteNumber] | 0b100000000>>bitNumber;
        position+=2;
      }
    }
    if(foundDigit) {
      position += 2;
    }
  }
  byte byteNumber = position / 8;
  byte bitNumber = position % 8; 
  bitMap[byteNumber] = bitMap[byteNumber] | 0b11000000>>bitNumber;
  position += 4; //2 for the 11 and 2 for a 00 pause
  sequenceLen = position ;

  state = OFF;
  position = 0;
  isRunning = true;
  this->repeat = repeat;
  this->speed = speed;
  handleTimeout();
}



void Blinker::cancelSequence()
{
  timer.deleteTimer(timerNumber);
  setHardwareState(OFF);
  isRunning = false;
}

static Blinker *blinker;

void interrupt()
{
  blinker->handleTimeout();
}


void Blinker::handleTimeout()
{
  byte byteNumber = position / 8;
  byte bitNumber = position % 8; 
  byte mask = 0b10000000>>bitNumber;

  byte enable = bitMap[byteNumber] & mask;

  if(enable) {
    setHardwareState(ON);
  } else {
    setHardwareState(OFF);
  }

  position ++;
  if(position == sequenceLen) {
    if(repeat) {
      position = 0;
    }else{
      cancelSequence();
      return;
    }
  }
  //Horrible... But Simpletimer is... simple and takes (*void)() so we have
  //no good way of passing the Blinker instance to it.
  blinker = this;
  timerNumber = timer.setTimeout(speed, &interrupt);
}


bool Blinker::isBlinking()
{
  return isRunning;
}


void Blinker::setHardwareState(BlinkerState hwState)
{
  //TODO - use tone(), noTone() for passive piezos.
  if(hwState==ON)
  {
    if(ledPin != NO_PIN) {
      digitalWrite(ledPin, HIGH);
    }
    if(piezoPin != NO_PIN){
       digitalWrite(piezoPin, HIGH);
    }
  }
  else if(hwState == OFF) {
    if(ledPin != NO_PIN) {
      digitalWrite(ledPin, LOW);
    }
    if(piezoPin != NO_PIN){
      digitalWrite(piezoPin, LOW);
    }
  }
  this->state = hwState;
}

