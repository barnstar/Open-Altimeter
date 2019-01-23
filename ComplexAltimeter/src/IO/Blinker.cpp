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

void Blinker::blinkValue(long value, int speed, bool repeat, bool pause)
{
  if (isBlinking()) {
    cancelSequence();
  }

  if (value > 999999) {
    return;
  }

  position = 0;
  memset(bitMap, 0, kBitMapLen);

  long tempValue  = value;
  bool foundDigit = false;  // Don't blink leading 0s

  // If we make it past 999km then this failing is probably the least of our
  // worries. Bitmask should be 10101000 10101010 0010100011 <- For 342... For
  // example
  for (long m = 1000000; m > 0; m = m / 10) {
    int digit = tempValue / m;
    if (digit || foundDigit) {
      foundDigit = true;
      tempValue  = tempValue - digit * m;
      if (digit == 0) {
        digit = 10;
      }  // Blink "0" as 10
      for (int i = 0; i < digit; i++) {
        byte byteNumber    = position / 8;
        byte bitNumber     = position % 8;
        bitMap[byteNumber] = bitMap[byteNumber] | 0b10000000 >> bitNumber;
        position += 2;
      }
    }
    if (foundDigit) {
      position += 2;
    }
  }
  if (repeat && pause) {
    byte byteNumber    = position / 8;
    byte bitNumber     = position % 8;
    bitMap[byteNumber] = bitMap[byteNumber] | 0b11000000 >> bitNumber;
    position += 4;  // 2 for the 11 and 2 for a 00 pause
  }
  sequenceLen = position;

  setHardwareState(OFF);
  position     = 0;
  isRunning    = true;
  this->repeat = repeat;
  this->speed  = speed;
  timerFired(0);
}

void Blinker::cancelSequence()
{
  position = 0;

#ifdef IS_SIMPLE_ALT
  timer.deleteTimer(timerNumber);
#else
  ticker.detach();
#endif

  timerNumber = -1;
  setHardwareState(OFF);
  isRunning = false;
}

void interrupt(Blinker *blinker) { blinker->timerFired(0); }

void Blinker::timerFired(int number)
{
  byte byteNumber = position / 8;
  byte bitNumber  = position % 8;
  byte mask       = 0b10000000 >> bitNumber;

  BlinkerState state = bitMap[byteNumber] & mask ? ON : OFF;
  setHardwareState(state);

  position++;
  if (position == sequenceLen) {
    if (!repeat) {
      cancelSequence();
      return;
    }
    position = 0;
  }
#ifdef IS_SIMPLE_ALT
  timerNumber = timer.setTimeout(speed, this);
#else
  ticker.once_ms(speed, interrupt, this);
#endif
}

bool Blinker::isBlinking() { return isRunning; }

void Blinker::setHardwareState(BlinkerState hwState)
{
  if (ledPin != NO_PIN) {
    digitalWrite(ledPin, (hwState == ON) ? HIGH : LOW);
  }

  if (piezoPin != NO_PIN) {
    digitalWrite(piezoPin, (hwState == ON) ? HIGH : LOW);
  }
}