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

#ifndef BLINKER_H
#define BLINKER_H

#include "types.h"
#include <Ticker.h>

typedef struct {
  int onTime;
  int offTime;
}Blink;

class Blinker
{
public:
  Blinker(int ledPin, int piezoPin) :
    ledPin(ledPin),
    piezoPin(piezoPin) {}

  ~Blinker() {
    cancelSequence();
  };

  void blinkValue(double value, double speed);

  void blinkSequence(Blink *sequence, size_t len, bool repeat);
  void cancelSequence();
  bool isBlinking();
  void handleTimeout();

private:
  BlinkerState state;
  void setHardwareState(BlinkerState hwState);

  int ledPin = NO_PIN;
  int piezoPin = NO_PIN;

  Blink sequence[kMaxBlinks];

  size_t sequenceLen =0;
  int position = 0;
  bool repeat = 0;

  Ticker ticker;
  bool isRunning;
};

#endif //BLINKER_H
