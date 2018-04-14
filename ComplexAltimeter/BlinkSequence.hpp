#ifndef blinksequence_h
#define blinksequence_h

#include "types.h"
#include <Ticker.h>

typedef struct {
  int onTime;
  int offTime;  
}Blink;

class Blinker
{
  public:
  Blinker(int ledPin, int piezoPin) :
  ledPin(ledPin),
  piezoPin(piezoPin)
  {}

  ~Blinker() {
    cancelSequence();  
  };

  void blinkSequence(Blink *sequence, size_t len, bool repeat);
  void cancelSequence();
  bool isBlinking();
  void handleTimeout();

  private:
  BlinkerState state;
  void setHardwareState(BlinkerState hwState);

  int ledPin = NO_PIN;
  int piezoPin = NO_PIN;

  Blink *sequence = nullptr;
  size_t sequenceLen =0;
  int position = 0;
  bool repeat = 0;

  Ticker ticker;
  bool isRunning;
};

#endif
