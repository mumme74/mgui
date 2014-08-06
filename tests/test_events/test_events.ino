#include <DList.h>
#include <mgui.h>
#include <testmacros.h>

class RecieveObj : public EventReciever {
public:
	int evtCalledCnt;
	int evtCalled2Cnt;
	String eventMsg;
	RecieveObj() : evtCalledCnt(0), evtCalled2Cnt(0), eventMsg(0) {}
	void reset(){ eventMsg = ""; evtCalledCnt = 0; evtCalled2Cnt = 0; }
	
	void clicked() { ++evtCalledCnt; }
	void clicked2() { ++evtCalled2Cnt; }
	
	void evtMsg(const EventInfo *evt) {
		eventMsg = evt->str;
	}
};


class SenderObj {
public:
	SenderObj() {}
	EventSender onClickEvt;
	EventSender onStrEvt;
	void click() { onClickEvt.send(); }
	void sendStr(String s) { 
		EventInfo e;
		e.str = s;
		onStrEvt.send(&e);
	}
};


RecieveObj r1, r2, r3;
SenderObj s1, s2, s3;

void runTests(){
	
	r1.reset();
	r2.reset();
	r3.reset();
	
	testBegin();
	
	test(r1.evtCalledCnt, 0);
	test(r1.eventMsg, String(""));
	
	// create eventsListeners (they route the callbacks to the reciever)
	EventListener e1_1 = EVENTLISTENER(r1, clicked);
	EventListener e1_2 = EVENTLISTENER(r1, clicked2);
	
	// test that it doesnt get invoked implicitly
	s1.click();
	test(r1.evtCalledCnt, 0);
	test(r1.evtCalled2Cnt, 0);
	
	// connect them to listen on events
	s1.onClickEvt.addListener(e1_1);
	
	test(r1.evtCalledCnt, 0);
	test(r1.evtCalled2Cnt, 0);
	
	// trigger event on sender
	s1.click();
	test(r1.evtCalledCnt, 1);
	test(r1.evtCalled2Cnt, 0);
	
	// add event listener to same sendoer event
	s1.onClickEvt.addListener(e1_2);
	s1.click();
	
	test(r1.evtCalled2Cnt, 1);
	test(r1.evtCalledCnt, 2);
	
	
	s1.click();
	s1.click();
	test(r1.evtCalledCnt, 4);
	test(r1.evtCalled2Cnt, 3);
	
	
	// test diconnect with same EventListener
	s1.onClickEvt.removeListener(e1_1);
	
	// click should not be invoke reciever
	s1.click();
	s1.click();
	test(r1.evtCalledCnt, 4);
	test(r1.evtCalled2Cnt, 5);
	
	// test to send messages
	testS(r1.eventMsg, "");
	
	// connect the 2 different objects together
	EventListener e1str = EVENTLISTENER_WITH_INFO(r1, evtMsg);
	s1.onStrEvt.addListener(e1str);
	
	// trigger a event and make sure it reaches the destination
	s1.sendStr("test1");
	testS(r1.eventMsg, "test1");
	
	//make sure it doesnt interfere with each other
	r1.reset();
	
	// set up a new event with diffenet sender and reciever
	test(r2.evtCalledCnt, 0);
	EventListener e2 = EVENTLISTENER(r2, clicked);
	s2.onClickEvt.addListener(e2);
	test(r2.evtCalledCnt, 0);
	
	s2.click();
	test(r2.evtCalledCnt, 1);
	
	// test that it doesnt interfere with s1 and r1
	test(r1.evtCalledCnt, 0);
	
	// test remove listener with a new listenerobject created in a new scope
	{
		r2.reset();
		test(r2.evtCalledCnt, 0);
		EventListener e2_tmp = EVENTLISTENER(r2, clicked);
		s2.onClickEvt.removeListener(e2_tmp);
	} // e2_tmp goes out of scope here and gets destructed, so it should disconnect automatically
	
	//r2 and s2 should now be disconnected
	s2.click();
	test(r2.evtCalledCnt, 0);
	
	// test adding a listener that should auto disconnect when it goes out of scope
	{
		r2.reset();
		test(r2.evtCalledCnt, 0);
		EventListener e2_tmp = EVENTLISTENER(r2, clicked);
		s2.onClickEvt.addListener(e2_tmp);
		s2.click();
		test(r2.evtCalledCnt, 1);
	} // e2_tmp goes out of scope and gets dissconnected
	
	s2.click();
	test(r2.evtCalledCnt, 1);
	
	// check double connect rejection
	{
		r2.reset();
		test(r2.evtCalledCnt, 0);
		EventListener e2_tmp = EVENTLISTENER(r2, clicked);
		EventListener e3_tmp = EVENTLISTENER(r2, clicked);
		s2.onClickEvt.addListener(e2_tmp);
		s2.onClickEvt.addListener(e3_tmp); // this should get rejected as it points to the same object and callback
		s2.click();
		test(r2.evtCalledCnt, 1); 
	} // e2_tmp and e3_tmp ges out of scope
	
	// test that receiverObject cleans up after itself
	{
		SenderObj sTmp;
		{
			RecieveObj rTmp;
			EventListener eTmp = EVENTLISTENER(rTmp, clicked);
			sTmp.onClickEvt.addListener(eTmp);
			sTmp.click();
			test(rTmp.evtCalledCnt, 1);
		} // eTmp and rTmp goes out of scope 
		sTmp.click(); // if auto dissconnect dont work it goes kaboom here
	} // or here when it tries to clean up in destructor
	
	// test that sender object cleans up after itself
	{
		RecieveObj rTmp;
		EventListener eTmp = EVENTLISTENER(rTmp, clicked);
		{
			SenderObj sTmp;
			sTmp.onClickEvt.addListener(eTmp);
			test(rTmp.evtCalledCnt, 0);
			sTmp.click();
			test(rTmp.evtCalledCnt, 1);
		} // sender object goes out of scope and should clean unplug itself
		
		test(rTmp.evtCalledCnt, 1);
	}


	// test to make it reset by operator=
	EventListener e3 = EVENTLISTENER(r3, clicked);
	s3.onClickEvt.addListener(e3);
	test(r3.evtCalledCnt, 0);
	s3.click();
	test(r3.evtCalledCnt, 1);
	// reset e3, should disconnect automatically
	e3 = EVENTLISTENER(r2, clicked);
	r2.reset(); // ensure zero cnt
	// click s3 to make sure it was disconnected properly
	s3.click();
	test(r3.evtCalledCnt, 1);
	// make sure we can re-use e3 as another EventListener now
	s2.onClickEvt.addListener(e3);
	test(r2.evtCalledCnt, 0);
	s3.click();
	test(r3.evtCalledCnt, 1);
	test(r2.evtCalledCnt, 0);
	
	s2.click();
	test(r3.evtCalledCnt, 1);
	test(r2.evtCalledCnt, 1);
	
	testEnd();
}


void setup(){
	Serial.begin(115200);
}

void loop(){
	testLoop();
}

