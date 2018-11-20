
#ifndef DISPLAYIFACE_H
#define DISPLAYIFACE_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayIface
{
  public:
	DisplayIface()
	{
		display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C
		display.clearDisplay();
		display.setTextColor(WHITE);
		display.setTextSize(1);
	}

  private:
}

#endif