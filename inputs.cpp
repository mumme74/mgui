#include "inputs.h"


// ------------- base class for all inputs -----------

InputBase::InputBase() :
  DListNode<InputBase*>(this)
{
}

InputBase::~InputBase()
{
}


// -------------- A simple digital button class, reads from a single pin --------------

DigitalInputButton::DigitalInputButton(uint8_t buttonPin, const char *name /*= 0*/, uint8_t debounceTime /*= 70*/) : 
  InputBase(), 
  _state(false), 
  _startedAt(0),
  _pinNr(buttonPin),
  _debounceTime(debounceTime),
  _name(name)
{
    pinMode(_pinNr, INPUT_PULLUP);
}

  
bool DigitalInputButton::isPressed(){
	//Serial.print("checkpressed:");
  int btnState = digitalRead(_pinNr);
  //Serial.print(btnState); Serial.print(" pin:");Serial.println(_pinNr);
  if (btnState != _state) {
     if (_startedAt == 0){
        _startedAt = millis(); // start new debounce
     } else if (millis() - _debounceTime > _startedAt){
       _state = btnState;
       _startedAt = 0;
       EventInfo evt;
       evt.cstr = _name;
       if (_state == true){
         onPressed();
         pressedEvent.send(&evt);
       } else {
         onReleased();
       	 releasedEvent.send(&evt); 
       }
     }
  } else if (_startedAt > 0) {
    _startedAt = 0;
  }
  return _state;
}

void DigitalInputButton::eventLoop() {
	isPressed();
}

// subclasses may override
void DigitalInputButton::onPressed(){
}

void DigitalInputButton::onReleased() {
}

