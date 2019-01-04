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

#ifndef OLEDVIEW_H
#define OLEDVIEW_H

#define kMaxLines 6

#include <Arduino.h>
#include "../../Configuration.h"

#ifdef USE_SSD1306
#include "lib/Adafruit_SSD1306.h"
typedef Adafruit_SSD1306 Display;
#endif

#ifdef USE_SH1106
#include "lib/Adafruit_SH1106.h"
typedef Adafruit_SH1106 Display;
#endif


class View
{
 public:
  View(Display &displayRef) : display(displayRef){};
  ~View(){};

  void setText(String text, int line, boolean update);
  void clear();
  void update();

  virtual void refresh() = 0;
  virtual void dismiss() = 0;
  virtual void longPressAction() = 0;
  virtual void shortPressAction() = 0;

  Display &display;

  bool active = false;

 protected:
  String lines[kMaxLines];
  bool needsRefresh = true;
};

#endif