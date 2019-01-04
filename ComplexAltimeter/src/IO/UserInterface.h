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

#ifndef UserInterface_H
#define UserInterface_H

#include "../../Configuration.h"
#include "lib/Adafruit_GFX.h"

#ifdef USE_SSD1306
#include "lib/Adafruit_SSD1306.h"
typedef Adafruit_SSD1306 Display;
#define DISP_CONSTRUCTOR display(kDispWidth, kDispHeight, &Wire)
#endif

#ifdef USE_SH1106
#include "lib/Adafruit_SH1106.h"
typedef Adafruit_SH1106 Display;
#define DISP_CONSTRUCTOR display(0)
#endif

#include "ButtonInput.h"
#include "FlightHistoryView.hpp"
#include "SensorDataView.hpp"
#include "SettingsView.hpp"
#include "StatusView.hpp"
#include "View.hpp"

#define kMaxViews 6

class UserInterface : public ButtonInputDelegate
{
 public:
  UserInterface()
      : DISP_CONSTRUCTOR,
        primaryButton(PrimaryButton, RESET_PIN),
        secondaryButton(SecondaryButton, INPUT_PIN),
        statusView(display),
        historyView(display),
        sensorDataView(display),
        settingsView(display)
  {
    primaryButton.setDelegate(this);
    secondaryButton.setDelegate(this);
  }

  ~UserInterface() {}

  void start()
  {
#ifdef USE_SSD1306
    boolean status = display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDR);
#endif

#ifdef USE_SH1106
    display.begin(SH1106_SWITCHCAPVCC, DISPLAY_I2C_ADDR, false);
#endif

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.cp437(true);
    display.display();

    addView(&sensorDataView, false);
    addView(&historyView, false);
    addView(&settingsView, false);
    addView(&statusView, true);
  }

  void addView(View *view, bool show);

  void buttonShortPress(ButtonInput *button) override;
  void buttonLongPress(ButtonInput *button) override;

  void nextView();
  void previousView();
  void setActiveView(int index);

  void eventLoop(bool dispDirty);

 private:
  Display display;
  ButtonInput primaryButton;
  ButtonInput secondaryButton;

  SensorDataView sensorDataView;
  StatusView statusView;
  FlightHistoryView historyView;
  SettingsView settingsView;

  View *views[kMaxViews];
  short activeViewIndex = 0;
  short viewCount       = 0;
};

#endif