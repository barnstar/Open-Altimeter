/*********************************************************************************
 * Open Altimeter
 *
 * Mid power rocket avionics software for altitude recording and dual deployment
 *
 * Copyright 2018, Jonathan Nobels
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **********************************************************************************/

#ifndef blinksequence_h
#define blinksequence_h

#ifdef IS_SIMPLE_ALT
#include "SimpleTimer.h"
#include "types.h"
#else
#include <Ticker.h>
#include "../types.h"
class TimerDelegate
{
};
#endif

// 128 bits will represent 64 on-off events.
// the number 1000 would require ~34 so this should
// be sufficient.
#define kBitMapLen 16

class Blinker : public TimerDelegate
{
 public:
#ifdef IS_SIMPLE_ALT
  Blinker(SimpleTimer &timer, byte ledPin, byte piezoPin)
      : timer(timer), ledPin(ledPin), piezoPin(piezoPin)
  {
  }
#else
  Blinker(int ledPin, int piezoPin) : ledPin(ledPin), piezoPin(piezoPin) {}
#endif

  ~Blinker() { cancelSequence(); };

  void blinkValue(long value, int speed, bool repeat, bool pause = true);
  void cancelSequence();
  bool isBlinking();

#ifdef IS_SIMPLE_ALT
  void timerFired(int timerNumber) override;
#else
  void timerFired(int timerNumber);
#endif

 private:
  byte timerNumber = 0;
  byte bitMap[kBitMapLen];

  void setHardwareState(BlinkerState hwState);

  int ledPin   = NO_PIN;
  int piezoPin = NO_PIN;

  byte sequenceLen = 0;
  byte position    = 0;
  bool repeat      = 0;
  int speed        = 0;

#ifdef IS_SIMPLE_ALT
  SimpleTimer &timer;
#else
  Ticker ticker;
#endif
  bool isRunning;
};

#endif
