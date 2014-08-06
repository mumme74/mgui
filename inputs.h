#ifndef INPUTS_H
#define INPUTS_H

#include <Arduino.h>
#include <DList.h>
#include "event.h"


// base class for all input types
class InputBase : public DListNode<InputBase*> {
public:
	InputBase();
	~InputBase(); // should be declared virtual in DListNode
	virtual void eventLoop() = 0;
};



// a simple digital button read on a single pin
class DigitalInputButton : public InputBase {
  bool _state;
  uint32_t _startedAt;
  uint8_t _pinNr;
  uint8_t _debounceTime;
  const char *_name;
public:
	DigitalInputButton() = delete; // prevent construction from empty constructor
	DigitalInputButton(uint8_t buttonPin, const char *name = 0, uint8_t debounceTime = 70);
  bool isPressed();
  EventSender pressedEvent;
  EventSender releasedEvent;
  virtual void onPressed();
  virtual void onReleased();
  virtual void eventLoop();
};

// TODO add a increment stepper class

#endif // INPUTS_H
