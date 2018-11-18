#ifndef BUTTONINPUT_H
#define BUTTONINPUT_H

using button_callback = void(*)();
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
	ButtonInput(uint8_t pin, uint16_t longPressInterval = 1000)  :
		pin(pin),
		longPressInterval(longPressInterval)
	{
		pinMode(pin, INPUT_PULLUP);
	}

	//call in loop();
	void update() 
	{
	  if(lastReleaseTime) {
	    if(millis() - lastReleaseTime < 100) {
	      return;
	    }
	  }
	
		if(digitalRead(pin) == LOW ) {
			if(pushedTime == 0) { pushedTime = millis(); }
			return;
		}
		
		if(pushedTime && digitalRead(pin) == HIGH) {
			long onTime = millis() - pushedTime;
			if(onTime < 30) {
			  pushedTime = 0;
			  return;
			}else if(onTime >= longPressInterval) {
				if(longPressCallback != nullptr) { longPressCallback(); }
				if(delegate != nullptr) { delegate->buttonLongPress(this); }
			}
			else if(onTime < longPressInterval) {
				if(shortPressCallback != nullptr) { shortPressCallback(); }
  			if(delegate != nullptr) { delegate->buttonShortPress(this); }
			}
			pushedTime = 0;
			lastReleaseTime = millis();
		}
	}

	void setDelegate(ButtonInputDelegate *delegate) {
		this->delegate = delegate;
	}

	void setShortPressCallback(button_callback callback) {
		this->shortPressCallback = callback;
	}

	void setLongPressCallback(button_callback callback) {
		this->longPressCallback = callback;
	}

private:
	uint8_t pin;
	uint16_t longPressInterval;
	long pushedTime = 0;
  long lastReleaseTime = 0;

	ButtonInputDelegate *delegate = nullptr;
	button_callback shortPressCallback = nullptr;
	button_callback longPressCallback = nullptr;
};



#endif