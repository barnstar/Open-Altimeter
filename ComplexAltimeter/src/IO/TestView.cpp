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

#include "TestView.hpp"

void TestView::shortPressAction()
{
  if (testDevice->enabled) {
    testDevice->disable();
  }

  activeOption++;
  if (activeOption == ControlChannelCount) {
    activeOption = ControlChannel1;
  }

  testDevice = &FlightController::shared().devices[activeOption];
  needsRefresh = true;
  resfresh();
}

void TestView::longPressAction()
{
  if (testDevice->enabled) {
    testDevice->disable();
  } else {
    testDevice->enable();
  }
}

void TestView::refresh()
{
  if (!needsRefresh) {
    return;
  }

  setText(F("===::: TEST :::==="), 0, false);

  String labels[TestOptionsCount];
  labels[0] = F("Test Ctl 1");
  labels[1] = F("Test Ctl 2");
  labels[2] = F("Test Ctl 3");
  labels[3] = F("Test Ctl 4");

  labels[activeOption] = String("**") + labels[activeOption] + String("**");
  for (int i = 0; i < 4; i++) {
    setText(labels[i], i + 1, false);
  }
  needsRefresh = false;
  update();
}

void TestView::dismiss()
{
  if (testDevice->enabled) {
    testDevice->disable();
    return;
  }
  activeOption = 0;
}