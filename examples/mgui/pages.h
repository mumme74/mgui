#ifndef PAGES_H
#define PAGES_H
#include <mgui.h>

// this file declares your different pages


// pagecontroller handles input/output switches pages for you etc
class PageController : public PageControllerBase {
	
	DigitalInputButton // declare some inputs 
  			 _upBtn,
         _downBtn,
         _okBtn;
         
  EventListener      // declare events that listens on the input pins
  			 _upPressedListener,
  			 _downPressedListener,
  			 _okPressedListener,
  			 _okReleasedListener;
public:
  PageController();
  void onPressed(const EventInfo *evt); // recieves press events from input buttons
  void onReleased(const EventInfo *evt);
  
  // ids to the application pages, 
  // an enum makes sure we have a uniqe id to each page
  enum Pages {
  	welcomePage, 			// the welcome screen
		labelsPage, // shows different labels
		buttonsPage,		// change settings
		toggleButtonsPage,
		valueChangePage
  };
};




// ----------- WelcomePage ----------------

// all pages must inherit from PageBase
class WelcomePage : public PageBase {
	IconButton _prevBtn,
					   _nextBtn;
public:
	WelcomePage();
	void okPressed();
	void show();
};



// ---------- LabelsPage ------------------
class LabelsPage : public PageBase {
	Label _leftAlignNoBorder,
				_rightAlignNoBorder,
				_centerAlignNoBorder,
				_borderLeftAlign,
				_customSizeBorder;
				
	IconButton _prevBtn,
						_nextBtn;
public:
	LabelsPage();
	void okPressed();
};



// ---------- ButtonsPage -----------------
class ButtonsPage : public PageBase {
	// buttons with only text
	TextButton _textBtn;
	// buttons with icons
	IconButton _redoIcon,
						_upIcon,
						_downIcon,
						_undoText,
						_closeText,
						_playText;
						
	// event listeners listenes on button events and calls a callback method
	EventListener _undoEvent,
							_redoEvent;
	void onUndo();
	void onRedo();
	
	// for page navigation
	IconButton _prevBtn,
						 _nextBtn;
						 
						
public:
	ButtonsPage();
	void okPressed();
};

// ---------- ToggleButtonsPage -----------
class ToggleButtonsPage : public PageBase {
	RadioButton _radio1,
							_radio2,
							_radio3;
							
	CheckBox	_chkBox1,
						_chkBox2;
		
	// for page navigation
	IconButton _prevBtn,
						 _nextBtn;
						
public:
	ToggleButtonsPage();
	void okPressed();
};



// ---------- ValueChangePage -------------
class ValueChangePage : public PageBase {
	SpinBox _spinBox1,
					_spinBox2;
					
	HSlider _hSldNoText,
					_hSldText;
					
	VSlider _vSldNoText,
					_vSldText;
					
	EventListener _spin1Evt,
								_hSldEvt,
								_vSldEvt;
								
	// for page navigation
	IconButton _prevBtn,
						 _nextBtn;
						
						
	void onSpinChange(const EventInfo *evt);
	void onHSldChange(const EventInfo *evt);
	void onVSldChange(const EventInfo *evt);
public:
	ValueChangePage();
	void okPressed();
};


#endif // PAGES_H
