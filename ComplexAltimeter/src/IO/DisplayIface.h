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

#ifndef DISPLAYIFACE_H
#define DISPLAYIFACE_H

#include "lib/Adafruit_GFX.h"
#include "lib/Adafruit_SSD1306.h"
#include <Wire.h>
#include "ButtonInput.h"
#include "../../Configuration.h"

#define kDispWidth 128
#define kDispHeight 64
#define kScrollButtonPin 0

class OledView;

typedef Adafruit_SSD1306 Display;

class DisplayIface
{
 public:
  DisplayIface()
      : display(kDispWidth, kDispHeight, &Wire)
  {
    boolean status = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.cp437(true);
    display.display();
  }

  ~DisplayIface() {}

  void addView(OledView *view, bool show)
  {
    if (viewCount < 7) {
      views[viewCount] = view;
      viewCount++;
    }
    if (show) {
      setActiveView(viewCount-1);
    }
  }

  void nextView();
  void previousView();
  void setActiveView(int index);
  Display display;

 private:
  OledView *views[8];
  short activeViewIndex = 0;
  short viewCount       = 0;
};

#endif