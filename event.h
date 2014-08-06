#ifndef EVENTS_H
#define EVENTS_H

#include <limits.h>
#include <DList.h>


#define EVENTLISTENER_PTR(classname, classinstancePtr, callbackname) \
					EventListener(static_cast<EventReciever*>((classinstancePtr)), \
          	static_cast<EventListener::callback_t>(&classname::callbackname))

#define EVENTLISTENER_PTR_WITH_INFO(classname, classinstancePtr, callbackname) \
					EventListener(static_cast<EventReciever*>((classinstancePtr)), \
          	static_cast<EventListener::callbackEvt_t>(&classname::callbackname))
          	
#define EVENTLISTENER(classInstance, callMethodName) \
				  EventListener(static_cast<EventReciever*>(&classInstance), \
				  static_cast<EventListener::callback_t>(&decltype(classInstance)::callMethodName))
				  
#define EVENTLISTENER_WITH_INFO(classInstance, callMethodName) \
				  EventListener(static_cast<EventReciever*>(&classInstance), \
				  static_cast<EventListener::callbackEvt_t>(&decltype(classInstance)::callMethodName))
				  
			//	  static_cast<EventReciever*>(&r1), 
			//						static_cast<EventListener::callback_t>(&decltype(RecieveObj)::clicked));
          	
          	
class EventReciever;
class EventListener;
class EventSender;



union EventInfo {
		String str;
		const char *cstr;
		int64_t i64;
		int32_t i32;
		int16_t i16;
		int8_t  i8;
		uint64_t ui64;
		uint32_t ui32;
		uint16_t ui16;
		uint8_t  ui8;
		uintptr_t ptr;
		double   dbl;
		float    flt;
		struct { uint16_t x; uint16_t y;} point;
		EventInfo();
		~EventInfo();
};

// classes that want to recieve events must derive this class
class EventReciever{
	friend class EventListener;
	DList<EventListener *> _listeners;
public:
	EventReciever();
	virtual ~EventReciever();
};


// this class routes the event signal to the correct reciever
class EventListener : public DListNode<EventListener *> {
public:
	typedef void (EventReciever::*callback_t)();
	typedef void (EventReciever::*callbackEvt_t)(const EventInfo *);
private:
	friend class EventReciever;
	friend class EventSender;
	EventReciever *_reciever;
	EventSender   *_sender;
	DListNode<EventListener *> _recieverNode; // used to store in reciever objects as a node in its DList (dont interfere with sender DList)
	callback_t _callback; // called when event it triggered
	callbackEvt_t _callbackEvt;
	void _unplug();
	void _send(const EventInfo *evt = 0);
	
public:
	EventListener(EventReciever *obj, callback_t callback);
	EventListener(EventReciever *obj, callbackEvt_t callback);
	EventListener();
	~EventListener();
	EventListener &operator= (const EventListener &other);
};


// this one stores and calls in order the event callbacks that are registered
// this class should be constructed on stack, is NOT with "new EventTrigger"
class EventSender {
	friend class EventListener;
	DList<EventListener *> _listeners;
public:
	EventSender();
	~EventSender();
	void addListener(EventListener &listener);
	void removeListener(EventListener &listener, bool deleteObj = false);
	
	// should only be called by the owner object
	void send(const EventInfo *evt = 0);
};


#endif // EVENTS_H
