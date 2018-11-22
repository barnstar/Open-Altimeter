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


#ifndef BUTTONINPUT_H
#define BUTTONINPUT_H

using button_callback = void (*)();
class ButtonInput;

class ButtonInputDelegate
{
 public:
  virtual void buttonShortPress(ButtonInput *button);
  virtual void buttonLongPress(ButtonInput *button);
};

class ButtonInput
{
 public:
  ButtonInput(uint8_t pin, uint16_t longPressInterval = 1000)
      : pin(pin), longPressInterval(longPressInterval)
  {
    pinMode(pin, INPUT_PULLUP);
  }

  // call in loop();
  void update()
  {
    if (lastReleaseTime) {
      if (millis() - lastReleaseTime < 100) {
        return;
      }
    }

    if (digitalRead(pin) == LOW) {
      if (pushedTime == 0) {
        pushedTime = millis();
      }
      return;
    }

    if (pushedTime && digitalRead(pin) == HIGH) {
      long onTime = millis() - pushedTime;
      if (onTime < 30) {
        pushedTime = 0;
        return;
      } else if (onTime >= longPressInterval) {
        if (longPressCallback != nullptr) {
          longPressCallback();
        }
        if (delegate != nullptr) {
          delegate->buttonLongPress(this);
        }
      } else if (onTime < longPressInterval) {
        if (shortPressCallback != nullptr) {
          shortPressCallback();
        }
        if (delegate != nullptr) {
          delegate->buttonShortPress(this);
        }
      }
      pushedTime      = 0;
      lastReleaseTime = millis();
    }
  }

  void setDelegate(ButtonInputDelegate *delegate) { this->delegate = delegate; }

  void setShortPressCallback(button_callback callback)
  {
    this->shortPressCallback = callback;
  }

  void setLongPressCallback(button_callback callback)
  {
    this->longPressCallback = callback;
  }

 private:
  uint8_t pin;
  uint16_t longPressInterval;
  long pushedTime      = 0;
  long lastReleaseTime = 0;

  ButtonInputDelegate *delegate      = nullptr;
  button_callback shortPressCallback = nullptr;
  button_callback longPressCallback  = nullptr;
};

#endif