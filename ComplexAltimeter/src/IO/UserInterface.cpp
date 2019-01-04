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

#include "UserInterface.h"
#include "View.hpp"

void UserInterface::eventLoop(bool dispDirty)
{
  primaryButton.update();
  secondaryButton.update();
  if (dispDirty) {
    View *view = views[activeViewIndex];
    view->refresh();
  }
}

void UserInterface::addView(View *view, bool show)
{
  if (viewCount < 7) {
    views[viewCount] = view;
    viewCount++;
  }
  if (show) {
    setActiveView(viewCount - 1);
  }
}

void UserInterface::nextView()
{
  int8_t index = activeViewIndex == viewCount - 1 ? 0 : activeViewIndex + 1;
  setActiveView(index);
}

void UserInterface::previousView()
{
  int8_t index = activeViewIndex == 0 ? viewCount - 1 : activeViewIndex - 1;
  setActiveView(index);
}

void UserInterface::setActiveView(int index)
{
  View *lastView = views[activeViewIndex];
  lastView->active   = false;

  activeViewIndex = index;
  View *view  = views[index];
  view->active    = true;
  view->refresh();
  view->update();
}


//Button Delegate

void UserInterface::buttonLongPress(ButtonInput *b)
{
  int bid = b->buttonId;
  if (bid == PrimaryButton) {
    // No action
  } else if (bid == SecondaryButton) {
    View *view = views[activeViewIndex];
    view->longPressAction();
  }
}

void UserInterface::buttonShortPress(ButtonInput *b)
{
  int bid = b->buttonId;
  if (bid == PrimaryButton) {
    nextView();
  } else if (bid == SecondaryButton) {
    View *view = views[activeViewIndex];
    view->shortPressAction();
  }
}

