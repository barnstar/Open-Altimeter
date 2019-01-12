#ifndef BUTTONINPUT_H
#define BUTTONINPUT_H

using button_callback = void (*)();
class ButtonInput;

class ButtonInputDelegate
{
  public:
	virtual void buttonShortPress(ButtonInput *button);
	virtual void buttonLongPress(ButtonInput *button);
}

class ButtonInput
{
  public:
	ButtonInput(uint8_t pin, uint16_t longPressInterval = 1000) : pin(pin),
																  longPressInterval(longPressInterval)
	{
		pinMode(pin, INPUT_PULLUP);
	}

	//call in loop();
	void update(long t)
	{
		if (pushedTime == 0 && digitalRead(pin) == LOW)
		{
			pushedTime = t;
		}
		if (pushedTime && digitalRead(pin) == HIGH)
		{
			int onTime = t - pushedTime;
			if (pushedTime < longPressInterval)
			{
				if (shortPressCallback != nullptr)
				{
					shortPressCallback();
				}
				if (delegate != nullptr)
				{
					delegate->buttonShortPress(this);
				}
			}
			if (pushedTime >= longPressInterval)
			{
				if (longPressCallback != nullptr)
				{
					longPressCallback();
				}
				if (delegate != nullptr)
				{
					delegate->buttonLongPress(this);
				}
			}
			pushedTime = 0;
		}
	}

	void setDelegate(ButtonInputDelegate *delegate)
	{
		this->delegate = delegate;
	}

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
	uint32_t pushedTime = 0;

	ButtonInputDelegate *delegate = nullptr;
	button_callback shortPressCallback = nullptr;
	button_callback longPressCallback = nullptr;
}

