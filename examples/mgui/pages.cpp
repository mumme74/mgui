#include "pages.h"
#include "pinouts.h"
#include "images.h"

const char up[] = "UP";
const char down[] = "DOWN";
const char ok[] = "OK";

PageController::PageController():
	PageControllerBase(), // call base class to complete inheritance
	_upBtn(UP_BUTTON_PIN, up),
  _downBtn(DOWN_BUTTON_PIN, down),
  _okBtn(OK_BUTTON_PIN, ok)
{
	// set up events
  _upPressedListener   = EVENTLISTENER_PTR_WITH_INFO(PageController, this, onPressed);
  _downPressedListener = EVENTLISTENER_PTR_WITH_INFO(PageController, this, onPressed);
  _okPressedListener   = EVENTLISTENER_PTR_WITH_INFO(PageController, this, onPressed);
  _okReleasedListener  = EVENTLISTENER_PTR_WITH_INFO(PageController, this, onReleased);
  // connect events
  _upBtn.pressedEvent.addListener(_upPressedListener);
  _downBtn.pressedEvent.addListener(_downPressedListener);
  _okBtn.pressedEvent.addListener(_okPressedListener);
  _okBtn.pressedEvent.addListener(_okReleasedListener);
  
  // add the inputs to bee checked automatically each the eventLoop
  addInput(_upBtn);
  addInput(_downBtn);
  addInput(_okBtn);
}

// These 2 recieves button pin events 
void PageController::onPressed(const EventInfo *evt){
	if (pages.length() == 0)
		return;
		
	// route events to the current displayed page
	if (evt->cstr == up){
		getActivePage()->moveUp(); 
	} else if (evt->cstr == down) {
		getActivePage()->moveDown();
	} else if (evt->cstr == ok) {
		getActivePage()->okPressed();
	}
}
void PageController::onReleased(const EventInfo *evt){
	if (pages.length() == 0)
		return;
		
	if (evt->cstr == ok){
		getActivePage()->okReleased();
	}
}



// ---------------------- WelcomePage ----------------------------

WelcomePage::WelcomePage():
	PageBase(),
	_prevBtn(3, 100, previous_arrow_png),
	_nextBtn(140, 100, next_arrow_png)
	
{
	pageId = PageController::welcomePage; // set a unique ID to this page
	addWidget(_prevBtn, true); // add button widget page to be controller by PageBase, and show it now
	addWidget(_nextBtn, true);
}

void WelcomePage::show(){
	PageBase::show(); // let baseclass do its stuff
	// draw the icon
	Display::type_t *tft = Display::get();
	EImg16 logo(welcome_png, tft, &Display::type_t::drawPixel);
	
	Serial.println(String("drawing welcome size width:") + logo.width() + " height:" + logo.height());
	logo.draw(16,0);
}

void WelcomePage::okPressed(){
	PageBase::okPressed(); // let baseclass to it thing first
	
	if (getFocusedButton() == &_prevBtn) {
		// goto previous page
		PageController::setPage(PageController::valueChangePage);
	} else if (getFocusedButton() == &_nextBtn) {
		PageController::setPage(PageController::labelsPage);
	}
}


// ------------------- LabelsPage -------------------------------
LabelsPage::LabelsPage() : 
	PageBase(),	
	_leftAlignNoBorder(100, 0, "Left Aligned text"),
	_rightAlignNoBorder(100, 30, "Right Aligned text", false, 0, 0, Label::Align::Right),
	_centerAlignNoBorder(100, 60, "Center Aligned text", false, 0, 0, Label::Align::Center),
	_borderLeftAlign(0, 0, "With border", true),
	_customSizeBorder(0,40, "Custom size", true, 50, 20, Label::Align::Center),
	
	// navigation buttons
	_prevBtn(3, 100, previous_arrow_png),
	_nextBtn(140, 100, next_arrow_png)
{
	pageId = PageController::labelsPage;
	addWidget(_prevBtn, true);
	addWidget(_nextBtn, true);
	
	// add the labels and show them now
	addWidget(_leftAlignNoBorder, true);
	addWidget(_rightAlignNoBorder, true);
	addWidget(_centerAlignNoBorder, true);
	addWidget(_borderLeftAlign, true);
	addWidget(_customSizeBorder, true);
}
				
void LabelsPage::okPressed(){
	PageBase::okPressed(); // let baseclass to it thing first
	
	if (getFocusedButton() == &_prevBtn) {
		// goto previous page
		PageController::setPage(PageController::welcomePage);
	} else if (getFocusedButton() == &_nextBtn) {
		PageController::setPage(PageController::buttonsPage);
	}
}

// --------------- ButtonsPage ---------------------------------
ButtonsPage::ButtonsPage():
	PageBase(),
	_textBtn(3, 3, "TextButton"),
	
	_redoIcon(3, 25, redo_arrow_png),
	_upIcon(3, 50, up_arrow_png),
	_downIcon(3, 75, down_arrow_png),
	_undoText(80, 3, undo_arrow_png, "Undo"),
	_closeText(80, 25, close_png, "Close"),
	_playText(80, 50, play_png, "Play"),
	
	// navigation buttons
	_prevBtn(3, 100, previous_arrow_png),
	_nextBtn(140, 100, next_arrow_png)
{
	// page navigation stuff
	pageId = PageController::buttonsPage;
	addWidget(_prevBtn, true);
	addWidget(_nextBtn, true);
	
	
	// add the display widgets
	addWidget(_textBtn, true);
	addWidget(_redoIcon, true);
	addWidget(_upIcon, true);
	addWidget(_downIcon, true);
	addWidget(_undoText, true);
	addWidget(_closeText, true);
	addWidget(_playText, true);
	
	// set up the event callbacks
	_undoEvent = EVENTLISTENER_PTR(ButtonsPage, this, onUndo);
	_redoEvent = EVENTLISTENER_PTR(ButtonsPage, this, onRedo);
	// connect events
	_undoText.pressEvent.addListener(_undoEvent);
	_redoIcon.pressEvent.addListener(_redoEvent);
}

void ButtonsPage::okPressed(){
	PageBase::okPressed(); // let baseclass to it thing first
	
	if (getFocusedButton() == &_prevBtn) {
		// goto previous page
		PageController::setPage(PageController::labelsPage);
	} else if (getFocusedButton() == &_nextBtn) {
		PageController::setPage(PageController::toggleButtonsPage);
	}
}

// event callbacks These listeners could just as well be any other object
// I attached them here for demo
void ButtonsPage::onUndo(){
	Serial.println("You pressed undo button!");
}

void ButtonsPage::onRedo(){
	Serial.println("You pressed redo button!");
}



// ----------------- ToggleButtonsPage ------------------
ToggleButtonsPage::ToggleButtonsPage() :
	PageBase(),
	_radio1(3, 3, "choice1"),
	_radio2(3, 30, "choice2" ),
	_radio3(3, 60, ""),
							
	_chkBox1(80, 3, "check 1"),
	_chkBox2(80, 30, "check 2"),
	
	// navigation buttons
	_prevBtn(3, 100, previous_arrow_png),
	_nextBtn(140, 100, next_arrow_png)
{
	// page navigation stuff
	pageId = PageController::toggleButtonsPage;
	addWidget(_prevBtn, true);
	addWidget(_nextBtn, true);
	
	
	// add widgets to page
	addWidget(_radio1, true);
	addWidget(_radio2, true);
	addWidget(_radio3, true);
	
	addWidget(_chkBox1, true);
	addWidget(_chkBox2, true);
}

// page navigation
void ToggleButtonsPage::okPressed(){
	PageBase::okPressed(); // let baseclass to it thing first
	
	if (getFocusedButton() == &_prevBtn) {
		// goto previous page
		PageController::setPage(PageController::buttonsPage);
	} else if (getFocusedButton() == &_nextBtn) {
		PageController::setPage(PageController::valueChangePage);
	}
}



// ------------------- ValueChangePage ----------------------
ValueChangePage::ValueChangePage() :
	PageBase(),
	// x,y , min, max, text
	_spinBox1(3, 0, -7, 14, "spin 1"),
	_spinBox2(3, 25, 0, 9, "spin 2"),
	// x, y, min, max, width, name
	_hSldNoText(3, 50, -10, 30, 90),
	_hSldText(3, 70, -2, 5, 90, "Horz"),
	// x, y, min, max, height, name				
	_vSldNoText(100, 0, 0, 20, 100),
	_vSldText(120, 0, -8, 8, 100, "Vsl"),
		
		// navigation buttons
	_prevBtn(3, 100, previous_arrow_png),
	_nextBtn(140, 100, next_arrow_png)
{
	// page navigation stuff
	pageId = PageController::valueChangePage;
	addWidget(_prevBtn, true);
	addWidget(_nextBtn, true);
	
	// add the display widgets
	addWidget(_spinBox1, true);
	addWidget(_spinBox2, true);
	addWidget(_hSldNoText, true);
	addWidget(_hSldText, true);
	addWidget(_vSldNoText, true);
	addWidget(_vSldText, true);
	
	// set up events
	_spin1Evt = EVENTLISTENER_PTR_WITH_INFO(ValueChangePage, this, onSpinChange);
	_hSldEvt  = EVENTLISTENER_PTR_WITH_INFO(ValueChangePage, this, onHSldChange);
	_vSldEvt  = EVENTLISTENER_PTR_WITH_INFO(ValueChangePage, this, onVSldChange);
	
	// connect events to the eventsender
	_spinBox1.valueChangedEvent.addListener(_spin1Evt);
	_hSldNoText.valueChangedEvent.addListener(_hSldEvt);
	_vSldNoText.valueChangedEvent.addListener(_vSldEvt);
}

// page navigation
void ValueChangePage::okPressed(){
	PageBase::okPressed(); // let baseclass to it thing first
	
	if (getFocusedButton() == &_prevBtn) {
		// goto previous page
		PageController::setPage(PageController::toggleButtonsPage);
	} else if (getFocusedButton() == &_nextBtn) {
		PageController::setPage(PageController::welcomePage);
	}
}


// event recievers, they can be any other object as long as it inherits EventReciver
// they are set to set these below for simplicity
void ValueChangePage::onSpinChange(const EventInfo *evt){
	Serial.println(String("You changed spinbox value to:") + evt->i32);
}
void ValueChangePage::onHSldChange(const EventInfo *evt){
	Serial.println(String("You changed horizontal slider value to:") + evt->i32);
}
void ValueChangePage::onVSldChange(const EventInfo *evt){
	Serial.println(String("You changed vertical slider value to:") + evt->i32);
}
