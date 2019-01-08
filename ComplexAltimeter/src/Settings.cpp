
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

#include "Settings.hpp"
#include "FS.h"

String Settings::readStringValue(const String &key, bool &success)
{
  String retVal;
  String fName = "/" + key + ".cfg";
  File f       = SPIFFS.open(fName, "r");
  if (f.available()) {
    retVal = f.readStringUntil('\n');
    success     = true;
  } else {
    success = false;
  }
  f.close();
  return retVal;
}

void Settings::writeStringValue(const String &value, const String &key)
{
  String fName = "/" + key + ".cfg";
  File f       = SPIFFS.open(fName, "w");
  f.println(value);
  f.close();
}

int Settings::readIntValue(const String &key, bool &success)
{
  String val = readStringValue(key, success);
  return val.toInt();
}

void Settings::writeIntValue(int value, const String &key)
{
  writeStringValue(String(value), key);
}
