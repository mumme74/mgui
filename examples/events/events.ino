/*
*	example showing how to use events
* This simplyfied example may seem pointless
* Events realy shines when building a gui were each widget cant be modified all the time to suit each usecase
*/

#include <DList.h> // events depends on this module
#include <mgui.h> // pulls in the event module

// define recever classes (that is able to take events)
class MyValue:
	public EventReciever    // all objects that wants to recieve events must derive from EventReciever
{
	int _value;
public:
	MyValue() { _value = 0; }
	void up() { _value++; }  
	void down() { _value--; }
	void print() { Serial.println(String("MyValue = ")  + _value); }
};

class LastInput : public EventReciever {
	String _str;
public:
	LastInput(){ }
	void onInput(const EventInfo *evt) {
		_str = evt->str;
	}
	void print() { Serial.println(String("your last text was:") + _str); }
};


// this class is the event sender that send out events to any object that is connected to it
class InputReader {
public:
	// define 3 events that EventReciever object can connect to
	EventSender upEvent;
	EventSender downEvent;
	EventSender textEvent;
	EventSender printEvent;
	void printUsage(){
		Serial.println("Please input +, - or any text (max 64 chars):");
	}
	
	void readSerialIn() {
		// read from serial input
		String str;
		while(Serial.available() > 0) {
			str += Serial.readString(); // read char by char until end of input stream
		}
		
		if (str.length() > 0){
			if (str == "+") {
				upEvent.send(); // we want to increment
			} else if (str == "-") {
				downEvent.send(); // we want to decrement
			} else { 
				EventInfo evt; // create a eventinfo that can carry the the str to the reciever/recievers 
											// (you can have as many recievers as you want connected to any event, the RAM is the limit)
				evt.str = str; // here we send a string, but we could also send a int, ptr float etc see EventInfo union for more info
				textEvent.send(&evt); // send out this string to all recievers
			}
			// make them print out what they have
			printEvent.send(); // note that this object doesnt now if it has any events connected here
												 // it doesnt have to have any events
		}
	}
};


// declare some object on the stack
InputReader    sendObj;
LastInput textObj;
MyValue   valueObj;

// create the listeners, they relay the event from the sender to the listener
EventListener upListener = EVENTLISTENER(valueObj, up);
EventListener downListener = EVENTLISTENER(valueObj, down);
EventListener printValueListener = EVENTLISTENER(valueObj, print);
// and the relays for textObj
EventListener textListener = EVENTLISTENER_WITH_INFO(textObj, onInput);
EventListener printTextListener = EVENTLISTENER(textObj, print);


void setup(){
	Serial.begin(115200);
	Serial.setTimeout(20); // only wait fro 20ms before we move on when reading string
	
	// print
	sendObj.printUsage();
	
	// attach the event listeners to the send object
	sendObj.upEvent.addListener(upListener);
	sendObj.downEvent.addListener(downListener);
	sendObj.textEvent.addListener(textListener);
	// note that we can add 2 or ore listeners to the same event, they will be called in insertion order
	sendObj.printEvent.addListener(printValueListener);
	sendObj.printEvent.addListener(printTextListener);
	
}


void loop(){
	sendObj.readSerialIn();
	delay(10);
}
