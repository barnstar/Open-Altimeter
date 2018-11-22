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


#include "Blinker.hpp"

#define kMaxBlinks 64

void Blinker::blinkValue(long value, int speed, bool repeat)
{
  if (isBlinking()) {
    return;
  }

  static Blink sequence[kMaxBlinks];
  int tempVal = value;
  if (tempVal < 30) {
    return;
  }
  bool foundDigit = false;  // Don't blink leading 0s
  int n           = 0;
  for (int m = 100000; m > 0; m = m / 10) {
    int digit = tempVal / m;
    if (digit || foundDigit) {
      foundDigit = true;
      tempVal    = tempVal - digit * m;
      if (digit == 0) digit = 10;
      for (int i = 0; i < digit; i++) {
        sequence[n].onTime  = speed;
        sequence[n].offTime = speed;
        if (n < kMaxBlinks) n++;
      }
    }
    if (foundDigit) {
      sequence[n - 1].offTime = speed * 2;
    }
  }
  sequence[n].onTime  = speed * 2;
  sequence[n].offTime = speed * 2;
  blinkSequence(sequence, n + 1, repeat);
}

void Blinker::blinkSequence(Blink *sequence, size_t len, bool repeat)
{
  cancelSequence();
  this->sequence = new Blink[len];
  memcpy(this->sequence, sequence, len * sizeof(Blink));
  state        = OFF;
  position     = 0;
  sequenceLen  = len;
  isRunning    = true;
  this->repeat = repeat;
  handleTimeout();
}

void Blinker::cancelSequence()
{
  ticker.detach();
  setHardwareState(OFF);
  if (sequence != nullptr) {
    delete sequence;
    sequence  = nullptr;
    isRunning = false;
  }
}

void interrupt(Blinker *blinker) { blinker->handleTimeout(); }

void Blinker::handleTimeout()
{
  int duration = 0;
  Blink b      = sequence[position];
  if (state == OFF) {
    setHardwareState(ON);
    duration = b.onTime;
  } else if (state == ON) {
    setHardwareState(OFF);
    duration = b.offTime;
    position++;
  }

  if (position == sequenceLen) {
    if (repeat) {
      position = 0;
    } else {
      cancelSequence();
      return;
    }
  }
  ticker.once_ms(duration, interrupt, this);
}

bool Blinker::isBlinking() { return isRunning; }

void Blinker::setHardwareState(BlinkerState hwState)
{
  if (hwState == ON) {
    if (ledPin != NO_PIN) {
      digitalWrite(ledPin, HIGH);
    }
    if (piezoPin != NO_PIN) {
      digitalWrite(piezoPin, HIGH);
    }
  } else if (hwState == OFF) {
    if (ledPin != NO_PIN) {
      digitalWrite(ledPin, LOW);
    }
    if (piezoPin != NO_PIN) {
      digitalWrite(piezoPin, LOW);
    }
  }
  this->state = hwState;
}
