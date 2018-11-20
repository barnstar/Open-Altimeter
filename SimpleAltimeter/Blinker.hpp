/*********************************************************************************
 * Simple Alitimeter
 *
 * Mid power rocket avionics software for alitidue recording and dual deployment
 *
 * Copyright 2018, Jonathan Nobels
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 *of this software and associated documentation files (the "Software"), to deal
 *in the Software without restriction, including without limitation the rights
 *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *SOFTWARE.
 **********************************************************************************/

#ifndef BLINKER_H
#define BLINKER_H

#include "SimpleTimer.h"
#include "types.h"

#define kBitMapLen 16
#define kMaxBits \
  kBitMapLen *   \
      8  // This is enough for 62 blinks + padding.  Enough to render 999999.

class Blinker : public TimerDelegate
{
 public:
  Blinker(SimpleTimer &timer, byte ledPin, byte piezoPin)
      : timer(timer), ledPin(ledPin), piezoPin(piezoPin)
  {
    // Zero the bitmap
    memset(bitMap, 0, kBitMapLen);
  }

  ~Blinker() { cancelSequence(); };

  void blinkValue(long value, int speed, bool repeat);
  void cancelSequence();
  bool isBlinking();
  void timerFired(int timerNumber) override;

 private:
  SimpleTimer &timer;
  byte timerNumber = 0;
  byte bitMap[kBitMapLen];

  BlinkerState state;
  void setHardwareState(BlinkerState hwState);

  byte ledPin   = NO_PIN;
  byte piezoPin = NO_PIN;

  byte sequenceLen = 0;
  byte position    = 0;
  bool repeat      = 0;
  int speed        = 0;

  // Ticker ticker;
  bool isRunning;
};

#endif  // BLINKER_H
