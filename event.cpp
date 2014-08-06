#include "event.h"
#include <cstring>

EventInfo::EventInfo() : 
	str("")
{
	// clear all memory
//	memset((void*)data; 0; sizeof(data));
	
}
EventInfo::~EventInfo()
{
}

EventReciever::EventReciever()
{
}

EventReciever::~EventReciever()
{
	// unplug any EventListeners that points to this object
	for(EventListener* l = _listeners.first(); _listeners.canMove(); l = _listeners.next()){
		if (l != 0)
		  l->_unplug();
	}
}


// ----------- EventListener ---------------
// this class routes the event signal to the correct reciever
EventListener::EventListener(EventReciever *obj, callback_t callback) :
	DListNode<EventListener *>(this), 
	_reciever(obj), 
	_sender(0),
	_recieverNode(this),
	_callback(callback),
	_callbackEvt(0)
{
	// add this object to recievers collection
	_reciever->_listeners.push(_recieverNode);
}

EventListener::EventListener(EventReciever *obj, callbackEvt_t callback) :
	DListNode<EventListener *>(this),
	_reciever(obj),
	_sender(0),
	_recieverNode(this),
	_callback(0),
	_callbackEvt(callback)
{
	_reciever->_listeners.push(_recieverNode);
}

EventListener::EventListener() :
	DListNode<EventListener *>(this),
	_reciever(0),
	_sender(0),
	_recieverNode(this),
	_callback(0),
	_callbackEvt(0)
{
}


EventListener &EventListener::operator= (const EventListener &other) {
	if (this != &other){
		_unplug();
		_reciever = other._reciever;
		_callback = other._callback;
		_callbackEvt = other._callbackEvt;
		_reciever->_listeners.push(_recieverNode);
	}
	return *this;
}

EventListener::~EventListener()
{
	// disconnect from all as we are being deleted
	//Serial.println(String("Destructor for:") + (int)this );
	_unplug();
}

void EventListener::_unplug(){
	// remove ourselves from sender and reciever collections so they can die in piece if they need to ;)
	if (_sender != 0){
		// compare with obj and callback not pointer references, operator comparisson doesnt seem to work, cant overload pointer comparisson?
		for(size_t idx = 0; idx < _sender->_listeners.length(); ++idx){
			if (_sender->_listeners[idx]->_reciever == _reciever && 
					_sender->_listeners[idx]->_callback == _callback &&
					_sender->_listeners[idx]->_callbackEvt == _callbackEvt)
			{
				_sender->_listeners.remove(idx);
				_sender = 0;
				//Serial.println(String(" dissconnecting from sender:") + (int)_sender + " with idx:" + idx);
				break;
			}
		}
	}
	// the reciever always has a pointer to this object, as its get set on construction
	if (_reciever != 0){
		int32_t idx =  _reciever->_listeners.indexOf(this); 
		//Serial.println(String(" dissconnecting from reciever:") + (int)_sender + " with idx:" + idx);
		if (idx > -1) {
			_reciever->_listeners.remove(idx);
			_reciever = 0; _callback = 0; _callbackEvt = 0; // reset it back to virgin state this object is now unusable as a eventListener but at least it wont crash your app
		}
	}
}


void EventListener::_send(const EventInfo *evt /* = 0*/){
	if (_reciever != 0) {
		if (_callbackEvt != 0) {
				//Serial.println(String("sending event to :") + (int)_reciever );
			(_reciever->*_callbackEvt)(evt);
		} else {
			//Serial.println(String("sending event to :") + (int)_reciever );
			(_reciever->*_callback)();
		}
	}
}



// -------------- EventSender class -----------------
EventSender::EventSender()
{
}

EventSender::~EventSender()
{
	for(EventListener *l = _listeners.first(); _listeners.canMove(); l = _listeners.next()){
		l->_unplug();
	}
}

void EventSender::addListener(EventListener &listener){
	// check so we dont add a connection to same object twice, you never now….
	for(auto l = _listeners.first(); _listeners.canMove(); l = _listeners.next()){
		if (l->_reciever == listener._reciever && 
				l->_callback == listener._callback &&
				l->_callbackEvt == listener._callbackEvt)
				return; // we have already added a event connection to this same object and the same callback
	}
	// a new event we can safely connect it
	listener._sender = this;
	_listeners.push(listener);
}

// removes any listeners that points to the same reciever and reciever callback method
// the Listener object created here doesn't have to be the same object that was added to addListener
// deleteObj is used if the EventListener whas created with new 
void EventSender::removeListener(EventListener &listener, bool deleteObj /*= false*/) {
	listener._sender = this;
	int32_t idx =  _listeners.indexOf(&listener);
	if (idx > -1){
		EventListener* l = _listeners[idx];
		l->_unplug();
		if (deleteObj)
			delete l;
	}
}
	
// should only be called by the owner object	
void EventSender::send(const EventInfo *evt /*= 0*/) {
	//Serial.println(String("EventSender send _listeners count:") + _listeners.length());
	for(EventListener *l = _listeners.first(); _listeners.canMove(); l = _listeners.next()){
		l->_send(evt);
	}
}



