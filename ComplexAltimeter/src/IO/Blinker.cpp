#include "Blinker.hpp"

void Blinker::blinkValue(long value, int speed, bool repeat)
{
  if(isBlinking()) {
    return;
  }

  static Blink sequence[kMaxBlinks];
  int tempVal = value
  if(tempVal < 30){
    return;
  }
  bool foundDigit = false;         //Don't blink leading 0s
  int n=0;
  for(int m=100000; m>0; m=m/10) {  //If we make it past 99km, we're in trouble :)
    int digit = tempApogee / m;
    if (digit || foundDigit){
      foundDigit = true;
      tempVal = tempVal - digit*m;
      if(digit == 0)digit = 10;
      for(int i=0;i<digit;i++) {
        sequence[n].onTime = speed;
        sequence[n].offTime = speed;
        if(n<kMaxBlinks)n++;
      }
    }
    if(foundDigit) {
      sequence[n-1].offTime = speed*2;
    }
  }
  sequence[n].onTime = speed*2;
  sequence[n].offTime = speed*2;
  blinkSequence(sequence, n+1, true, repeat);
}


void Blinker::blinkSequence(Blink *sequence, size_t len, bool repeat)
{
  cancelSequence();
  this->sequence = new Blink[len];
  memcpy(this->sequence, sequence, len*sizeof(Blink));
  state = OFF;
  position = 0;
  sequenceLen = len;
  isRunning = true;
  this->repeat = repeat;
  handleTimeout();
}


void Blinker::cancelSequence()
{
  ticker.detach();
  setHardwareState(OFF);
  if(sequence != nullptr) {
    delete sequence;
    sequence = nullptr;
    isRunning = false;
  }
}


void interrupt(Blinker *blinker)
{
  blinker->handleTimeout();
}


void Blinker::handleTimeout()
{
    int duration = 0;
    Blink b = sequence[position];
    if(state == OFF) {
      setHardwareState(ON);
      duration = b.onTime;
    }
    else if(state == ON) {
      setHardwareState(OFF);
      duration = b.offTime;
      position++;
    }

    if(position == sequenceLen) {
      if(repeat) {
        position = 0;
      }else{
        cancelSequence();
        return;
      }
    }
    ticker.once_ms(duration, interrupt, this);
}


bool Blinker::isBlinking()
{
  return isRunning;
}


void Blinker::setHardwareState(BlinkerState hwState)
{
  if(hwState==ON)
  {
    if(ledPin != NO_PIN) {
      digitalWrite(ledPin, HIGH);
    }
    if(piezoPin != NO_PIN){
        digitalWrite(piezoPin, HIGH);
    }
  }
  else if(hwState == OFF) {
    if(ledPin != NO_PIN) {
      digitalWrite(ledPin, LOW);
    }
    if(piezoPin != NO_PIN){
        digitalWrite(piezoPin, LOW);
    }
  }
  this->state = hwState;
}

