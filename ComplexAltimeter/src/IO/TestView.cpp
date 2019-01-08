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
#include "../FlightController.hpp"

void TestView::shortPressAction()
{
  if (testDevice->deployed) {
    testDevice->disable();
  }

  activeOption++;
  if (activeOption == kOptionCount) {
    activeOption = 0;
  }

  testDevice   = &FlightController::shared().devices[activeOption];
  needsRefresh = true;
  refresh();
}

void TestView::longPressAction()
{
  if (testDevice->deployed) {
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

  testDevice = &FlightController::shared().devices[activeOption];

  setText(F("===::: TEST :::==="), 0, false);

  labels[activeOption] = String("::") + labels[activeOption] + String("::");
  for (int i = 1; i < 5; i++) {
    String label;
    if (i == activeOption) {
      label = String("::Test Chan ") + String(i) + String("::");
    } else {
          label =  String("  Test Chan " + String(i);
    }
    setText(label, i, false);
  }
  needsRefresh = false;
  update();
}

void TestView::dismiss()
{
  needsRefresh = true;
  if (testDevice->deployed) {
    testDevice->disable();
    return;
  }
  activeOption = 0;
}