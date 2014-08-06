#ifndef GUI_BASE_H
#define GUI_BASE_H
#include <Arduino.h>
#include <Adafruit_ST7735.h>
//#include "pinouts.h"
#include <eimg_arduino.h>
#include "event.h"
#include "mgui_styling.h"
#include "inputs.h"


// forward declarations
class PageControllerBase;
class PageBase;
class GuiButton;

class Display {
	Adafruit_ST7735 tft;
public:
	static Adafruit_ST7735 *ptft;
	
	Display(uint8_t tft_cs, uint8_t tft_dc, uint8_t tft_reset);
	typedef Adafruit_ST7735 type_t;
	static Adafruit_ST7735 *get();
};

// dimensions of widgets are based on this rect
class Rect {
public:
	uint16_t x, y, w, h;
	Rect() : x(0),y(0),w(0), h(0) {}
};

// base class forr all widgets
class WidgetBase : public DListNode<WidgetBase *>, public EventReciever {
protected:
  Rect _box;
  
  bool _isShown,
  		 _hasFocus;
public:
	enum t_Info {
		t_none = 0,
		t_selectable = 0x01,   // 00000001   // tells if we should stop at widget when navigating (think tabstop)
		t_incrementable = 0x02 // 00000010   // tells if we should call up or down when selected insted of moving to next widget
	};
	virtual uint8_t type() const { return t_none; }; // = t_none; // this baseclass is 
  virtual void show();
  virtual void hide();
  virtual void setFocus(bool hasFocus);
  const Rect &getSize() const { return _box; }
  
  WidgetBase();
  ~WidgetBase(); // subclasses has virtual defined
  virtual void reDraw() = 0;
  
  virtual void eventLoop();
};

// all button like widgets should inherit this one
class ButtonBase : public WidgetBase {
protected:
	bool _state;
public:
	virtual uint8_t type() const { return t_selectable; }
	ButtonBase();
	~ButtonBase();
	virtual void press() = 0;
	virtual void release() = 0;
	virtual bool getState(); // should be true when pressed
};

// baseclass for all widgets that hodls a value
// spinboxes, sliders etc
class ValueWgtBase : public ButtonBase {
protected:
	int _minVlu,
			_maxVlu,
			_value;
public:
	virtual uint8_t type() const { return t_selectable | t_incrementable; }
	ValueWgtBase(int minValue, int maxValue);
	~ValueWgtBase();
	virtual void press();
	virtual void release();
	virtual int getValue();
	virtual void setValue(int value);
	virtual void up();
	virtual void down();
	
	EventSender valueChangedEvent;
};

// each menu screen is handled by this class (sort of a form in regular gui)


class PageBase : public WidgetBase {
protected:
	uint8_t activeWidgetIdx;
	DList<WidgetBase *> widgets;
public:
	uint8_t pageId;
  PageBase();
  void addWidget(WidgetBase &widget, bool setShow = false);
  ButtonBase *getFocusedButton();
  virtual void okPressed(); // called when ok button is pushed
  virtual void okReleased(); // called when button is released
  virtual void moveUp(); // called when move button is pushed
  virtual void moveDown();
  virtual void eventLoop();
  virtual void show();
  virtual void hide();
  virtual void reDraw();
};

// this is a non GUI class but it handles all the different pages
class PageControllerBase : public EventReciever {
  int _returnFromDialogPageId;
protected:  
  DList<WidgetBase *> pages; // PageBase inherits WidgetBase and is as such a DListNode<WidgetBase *>
  DList<InputBase *> inputs; // All inputs inherits DListNode<InputsBase *>
  uint8_t activePageIdx;
  static PageControllerBase *pContr;
public:
  PageControllerBase();
  static PageControllerBase *get();
  static void setPage(uint8_t pageId); 
  PageBase *getActivePage();
  void gotoPage(uint8_t pageId);
  void showDialogPage(uint8_t dialogPageId);
  void restoreFromDialogPage();
  void addPage(PageBase &page);
  void addInput(InputBase &);
  void eventLoop();
};

// ---------------- end of base classes ------------------------------


// ---------------- actual GUI widgets starts here -------------------


// ---------------- Text only button --------------------------
class TextButton : public ButtonBase {
protected:
	String _caption;
	uint32_t _displayTimer;
	uint16_t startX, startY;
	
public:
	TextButton(uint16_t x, uint16_t y, const char *caption,  uint16_t w = 0, uint16_t h = 0);
	~TextButton();
	EventSender pressEvent;
	EventSender releaseEvent;
	virtual void press();
	virtual void release();
	//virtual void show();
	//virtual void hide();
	//virtual void setFocus(bool hasFocus);
	virtual void reDraw();
};

// --------------- Button with Icon (and text) -----------------
// button with a Icon in it (may also have caption)
class IconButton : public TextButton {
	EImg16 _icon;
	
public:
	IconButton(uint16_t x, uint16_t y, const uint8_t *eimg_icon, const char *caption = 0,  uint16_t w = 0, uint16_t h = 0);
	~IconButton();
	virtual void reDraw();
};


// ----------------- Checkbox ------------------
// a simple CheckBox
class CheckBox : public TextButton {
	static const uint8_t 
						_boxWidth = 10,
						_boxHeight = 10;
protected:
	bool _checked;
public:
	CheckBox(uint16_t x, uint16_t y, const char *caption,  uint16_t w = 0, uint16_t h = 0);
	~CheckBox();
	virtual bool toggle();
	virtual void setChecked(bool checked);
	virtual void press();
	virtual bool isChecked() const;
	virtual void reDraw();
	
	EventSender checkedEvent;
	EventSender uncheckedEvent;
};

// -----------------Radio button ----------------
// currently we dont have any radiobox controller that unchecks the others, that will be up to each owner Page to deal with
class RadioButton : public CheckBox {
	static const uint8_t _circleRadius = 5;
public:
	RadioButton(uint16_t x, uint16_t y, const char *caption,  uint16_t w = 0, uint16_t h = 0);
	~RadioButton();
	virtual void reDraw();
};


// -------------- a spinbox ----------------------
class SpinBox : public ValueWgtBase {
	String _caption;
	uint16_t _spinBoxWidth;
protected:
	uint16_t _startX,
					_startY;
	
public:
	SpinBox(uint16_t x, uint16_t y, int minValue, int maxValue, const char *caption = 0, uint16_t w = 0, uint16_t h = 0);
	~SpinBox();
	virtual void reDraw();
};

//---------------- Horizontal slider ---------------
class HSlider : public ValueWgtBase {
protected:
	float _factor;
	String _caption;
	static const uint8_t
						_rulerHeight = 2,
						_indicatorHeight = 6,
						_indicatorWidth = 3;
	uint16_t _startY;
public:
	HSlider(uint16_t x, uint16_t y, int minValue, int maxValue, uint16_t w, const char *caption = 0, uint16_t h = 0);
	~HSlider();
	virtual void reDraw();
};

// ------------- Vertical slider ---------------
class VSlider : public ValueWgtBase {
protected:
	String _caption;
	float _factor;
	static const uint8_t
						_rulerWidth = 2,
						_indicatorHeight = 3,
						_indicatorWidth = 6;
	uint16_t _startY,
					 _startX,
					 _endY;
public:
	VSlider(uint16_t x, uint16_t y, int minValue, int maxValue, uint16_t h, const char *caption = 0, uint16_t w = 0);
	~VSlider();
	virtual void reDraw();
};

// ------------ Label (text holder) --------------
class Label : public WidgetBase {
public:
	enum class Align: uint8_t { Left, Center, Right };
protected: 
	String _text;
	uint16_t _setWidth,
					_setHeight;
	void _calcSize();
	bool _drawBorder;
	Align _align;
	void _mangleText(bool draw);
public:
	Label(uint16_t x, uint16_t y, const char *text = 0, bool drawBorder = false, uint16_t w = 0, uint16_t h = 0, Align align = Align::Left);
	~Label();
	const String getText() const;
	void setText(const String text);
	void setAlignment(Align align);
	void reDraw();
};

#endif // GUI_BASE_H
