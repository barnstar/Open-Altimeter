#ifndef BUTTONINPUT_H
#define BUTTONINPUT_H

typedef void (*button_callback)();


class ButtonInput
{
public:
	ButtonInput(byte pin, short longPressInterval = 1000) {
		this->pin = pin;
		this->longPressInterval = longPressInterval;
		pinMode(pin, INPUT_PULLUP);
	}

	//call in loop();
	void update() {
		if(pushedTime == 0 && digitalRead(pin) == LOW ) {
			pushedTime = millis();
		}
		if(pushedTime && digitalRead(pin) == HIGH) {
			int onTime = millis() - pushedTime;
			if(pushedTime < longPressInterval && shortPressCallback) {
				shortPressCallback();
			}
			if(pushedTime >= longPressInterval && longPressCallback) {
				longPressCallback();
			}
			pushedTime = 0;
		}
	}

	void setShortPressCallback(button_callback callback) {
		this->shortPressCallback = callback;
	}

	void setLongPressCallback(button_callback callback) {
		this->longPressCallback = callback;
	}

private:
	byte pin;
	short longPressInterval;
	int pushedTime = 0;

	button_callback shortPressCallback = nullptr;
	button_callback longPressCallback = nullptr;
}



#endif