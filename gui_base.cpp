#include "gui_base.h"
#include "inputs.h"
#include <eimg_arduino.h>

//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RESET);

Display::Display(uint8_t tft_cs, uint8_t tft_dc, uint8_t tft_reset) :
	tft(tft_cs, tft_dc, tft_reset)
{
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  
  // clear display
  tft.fillScreen(Styles::backColor);
  
  Display::ptft = &tft;
}


Adafruit_ST7735 *Display::ptft = 0;

Adafruit_ST7735* Display::get() { return ptft; }


// ------------ WidgetBase -------------------------

WidgetBase::WidgetBase()
  : DListNode<WidgetBase *>(this), EventReciever(),
   _isShown(false)
{
}

WidgetBase::~WidgetBase() 
{
}

void WidgetBase::eventLoop()
{
}

void WidgetBase::setFocus(bool hasFocus) {
	_hasFocus = hasFocus;
	reDraw();
}

void WidgetBase::hide() {
	 _isShown = false;
	  reDraw();
}

void WidgetBase::show() {
	_isShown = true;
	reDraw();
}


// ------------- ButtonBase -----------------------
ButtonBase::ButtonBase() :
	WidgetBase(), _state(false)
{
}

ButtonBase::~ButtonBase() 
{
}

bool ButtonBase::getState(){
	return _state;
}
	


// ------------ ValueWgtBase ----------------------
ValueWgtBase::ValueWgtBase(int minValue, int maxValue):
	ButtonBase(), _minVlu(minValue), _maxVlu(maxValue), _value(0)
{
}

ValueWgtBase::~ValueWgtBase()
{
}

void ValueWgtBase::press() {
	// toggle state so we can change value with up/down buttons
	_state = !_state;
	reDraw();
	
}

void ValueWgtBase::release() {
}

int ValueWgtBase::getValue(){
	return _value;
}

void ValueWgtBase::setValue(int value) {
	if (_minVlu <= value && _maxVlu >= value) {
		_value = value;
		reDraw();
		EventInfo evt;
		evt.i32 = _value;
		valueChangedEvent.send(&evt);
	}
}

void ValueWgtBase::up() {
	setValue(_value +1);
}

void ValueWgtBase::down() {
	setValue(_value -1);
}



// ------------- PageBase --------------------------

PageBase::PageBase(): 
	WidgetBase(), activeWidgetIdx(0), 
	pageId(0)
{
}

void PageBase::addWidget(WidgetBase &widget, bool setShow){
	static bool activeWidgetSet = false;
	if (widget.type() & t_selectable && !activeWidgetSet){
		activeWidgetIdx = widgets.length();
		activeWidgetSet = true;
	}
	
	widgets.push(widget);
	
	if (setShow)
		widget.show();
}

ButtonBase *PageBase::getFocusedButton(){
	if (widgets.length() > activeWidgetIdx && widgets[activeWidgetIdx]->type() & t_selectable){
		return static_cast<ButtonBase*>(widgets[activeWidgetIdx]);
	}
	return nullptr;
}

void PageBase::show(){
	Serial.println(String("PageBase show() ") + pageId);
	for(size_t idx = 0; idx < widgets.length(); ++idx){
		widgets[idx]->reDraw();
		if (idx == activeWidgetIdx)
			widgets[idx]->setFocus(true);
	}
}

void PageBase::hide(){
	Serial.println(String("PageBase hide() ") + pageId);
		
	Display::get()->fillScreen(Styles::backColor);
}

void PageBase::moveUp(){
	if (widgets.length() == 0) return;
	// route up presses to incrementable widgets
	if (widgets[activeWidgetIdx]->type() & t_incrementable && 
			static_cast<ButtonBase*>(widgets[activeWidgetIdx])->getState())
	{
		static_cast<ValueWgtBase*>(widgets[activeWidgetIdx])->up();
		return;
	}
	
	// find next widgetIdx that is selectable
	size_t idx; bool found = false;
	for(idx = activeWidgetIdx +1; idx < widgets.length(); ++idx){
		if (widgets[idx]->type() & t_selectable){
			found = true;
			break;
		}
	}
	
	if (!found){
		for(idx = 0; idx < activeWidgetIdx; ++idx){
			if (widgets[idx]->type() & t_selectable){
				found = true;
				break;
			}
		}
	}
	
	if (found) {
		widgets[activeWidgetIdx]->setFocus(false);
		activeWidgetIdx = idx;
		widgets[activeWidgetIdx]->setFocus(true);
	}
}

void PageBase::moveDown(){
	if (widgets.length() == 0) return;
	// route down presses to incrementable widgets
	if (widgets[activeWidgetIdx]->type() & t_incrementable && 
			static_cast<ButtonBase*>(widgets[activeWidgetIdx])->getState())
	{
		static_cast<ValueWgtBase*>(widgets[activeWidgetIdx])->down();
		return;
	}
	
	// find previous widgetIdx that is selectable
	size_t idx = 0; bool found = false;
	if (activeWidgetIdx > 0) {
		for(idx = activeWidgetIdx -1; idx >= 0 && idx < widgets.length(); --idx){
			//break;
			if (widgets[idx]->type() & t_selectable){
				found = true;
				break;
			}
		}
	}
	
	if (!found){
		for(idx = widgets.length() - 1; idx > activeWidgetIdx; --idx){
			if (widgets[idx]->type() & t_selectable){
				found = true;
				break;
			}
		}
	}
	
	if (found) {
		widgets[activeWidgetIdx]->setFocus(false);
		activeWidgetIdx = idx;
		widgets[activeWidgetIdx]->setFocus(true);
	}
}

void PageBase::eventLoop(){
	for(WidgetBase *w = widgets.first(); widgets.canMove(); w = widgets.next()){
		w->eventLoop();
	}
}

void PageBase::okPressed(){
	ButtonBase *b = getFocusedButton();
	if (b != nullptr){
		b->press();
	}
}

void PageBase::okReleased(){
	ButtonBase *b = getFocusedButton();
	if (b != nullptr){
		b->release();
	}
}

void PageBase::reDraw(){
	Display::get()->fillScreen(Styles::backColor);
	for (WidgetBase *w = widgets.first(); widgets.canMove();w = widgets.next()){
		w->reDraw();
	}
}


// -------------------- PageControllerBase -------------------
PageControllerBase::PageControllerBase() :
	EventReciever(), _returnFromDialogPageId(-1), activePageIdx(0)
{
	PageControllerBase::pContr = this;
}

PageControllerBase *PageControllerBase::pContr = 0;

PageControllerBase *PageControllerBase::get(){
	return pContr;
}

// a static
void PageControllerBase::setPage(uint8_t pageId){
	PageControllerBase *p = PageControllerBase::get();
	if (p != nullptr)
		p->gotoPage(pageId);
}

void PageControllerBase::gotoPage(uint8_t pageId) {
	//Serial.println(String("PageController gotoPage: ") + pageId);
	if (pages.length() > pageId) {
		//Serial.println(String("PageController pageCnt: ") + pages.length());
		getActivePage()->hide();
		// find the correct page with this Id
		for(size_t idx = 0; idx < pages.length(); ++idx) {
			if (static_cast<PageBase *>(pages[idx])->pageId == pageId) {
				activePageIdx = idx;
				break;
			}
		}
		getActivePage()->show();
	}
}

void PageControllerBase::showDialogPage(uint8_t dialogPageId) {
	if (pages.length() > dialogPageId) {
		_returnFromDialogPageId = getActivePage()->pageId;
		gotoPage(dialogPageId);
	}
}

void PageControllerBase::restoreFromDialogPage(){
	if (_returnFromDialogPageId > -1) {
		gotoPage(_returnFromDialogPageId);
		_returnFromDialogPageId = -1;
	}
}

void PageControllerBase::addPage(PageBase &page){
	pages.push(page);
}

void PageControllerBase::addInput(InputBase &input){
	inputs.push(input);
}


void PageControllerBase::eventLoop(){
	if (pages.length() > 0){
		pages[activePageIdx]->eventLoop();
	}
	// check the inputs
	for(InputBase *input = inputs.first(); inputs.canMove(); input = inputs.next()){
		input->eventLoop();
	}
}

PageBase *PageControllerBase::getActivePage(){
	if (pages.length() > 0){
		return static_cast<PageBase*>(pages[activePageIdx]);
	}
	return nullptr;
}



// -------------------- Gui widgets starts here ----------------------------

// ----------------------TextButton ---------------------------------------
TextButton::TextButton(uint16_t x, uint16_t y, const char *caption,  uint16_t w /*= 0*/, uint16_t h /*= 0*/)
{
	_box.x = x;
	_box.y = y;
	
	_caption = caption;
	
	if (w == 0 && h == 0) {
		const int pad = Styles::padding;

		w += _caption.length() * Styles::charWidth + (pad * 2);
		h = h < (Styles::charHeight + (pad * 2)) ? Styles::charHeight + (pad * 2) : h;
	}
	
	_box.w = w;
	_box.h = h;
	
	startX = _box.x + Styles::padding;
	startY = _box.y + ((_box.h / 2) - (Styles::charHeight / 2));
}
	
TextButton::~TextButton()
{
}


void TextButton::press() {
	if (!_state) {
		_state = true;
		reDraw();
	}
	pressEvent.send();
}

void TextButton::release(){
	if (_state){
		_state = false;
		reDraw();
	}
	releaseEvent.send();
}


// we reuse this funtion to also draw IconButton and some others
void TextButton::reDraw() {
	uint16_t color; 
	
	if (_isShown){
		if (_state)
			color = Styles::pressedBackColor;
		else if (_hasFocus)
			color = Styles::focusBackColor;
		else
			color = Styles::backCtlColor;
	} else {
		color = Styles::backColor;
	}
	
	Display::type_t *tft = Display::get();
	tft->fillRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, color);
	if (_isShown){
		color = _hasFocus ? Styles::focusBorderColor : Styles::borderColor;
		tft->drawRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, color);
		
		if (_caption.length() > 0){
			tft->setTextColor(color);
			tft->setCursor(startX, startY);
			tft->setTextSize(Styles::textSize);
			tft->print(_caption);
		}
	}
}



// --------------------- Icon Button --------------------------------------

IconButton::IconButton(uint16_t x, uint16_t y, const uint8_t *eimg_icon, const char *caption /*= 0*/,  uint16_t w /*= 0*/, uint16_t h /*= 0*/):
	TextButton(x, y, caption, w, h), _icon()
{
	_box.x = x;
	_box.y = y;
	
	_icon = EImg16(eimg_icon, Display::get(), &Adafruit_ST7735::drawPixel);
	
	if (w == 0 && h == 0) {
		const int pad = Styles::padding;
		w += _icon.width() + (pad *2);
		h += _icon.height() + (pad *2);

		if (caption != 0) {
			_caption = caption;
			w += _caption.length() * Styles::charWidth + (pad);
			h =  h < (Styles::charHeight + pad) ? Styles::charHeight + pad : h;
		}
	}
	
	_box.w = w;
	_box.h = h;
	
	startX += _icon.width() + Styles::padding;
	startY = _box.y + ((_box.h / 2) - (Styles::charHeight / 2));
}

IconButton::~IconButton()
{
}

void IconButton::reDraw() {
	TextButton::reDraw(); // using baseclass to draw the main parts
	
	if (_isShown) {
		int iconX = _box.x + Styles::padding,
				iconY = _box.y + Styles::padding;
		
		_icon.draw(iconX, iconY);
	}
}



// ------------------ CheckBox -----------------------------

CheckBox::CheckBox(uint16_t x, uint16_t y, const char *caption,  uint16_t w /*= 0*/, uint16_t h /*= 0*/) :
 TextButton(x, y, caption, w, h), _checked(false)
{
	if (w == 0 && h == 0) {
		const int pad = Styles::padding;
		w += _boxWidth + pad;
		h += _boxHeight + pad;

		if (caption != 0) {
			_caption = caption;
			w += _caption.length() * Styles::charWidth + (pad *2);
			h =  h < (Styles::charHeight + (pad * 2)) ? Styles::charHeight + (pad *2) : h;
		}
	}
	
	_box.w = w;
	_box.h = h;
	
	startX += _boxWidth + Styles::padding;
	startY = _box.y + ((_box.h / 2) - (Styles::charHeight / 2));
}

CheckBox::~CheckBox()
{
}

bool CheckBox::toggle() {
	setChecked(!_checked);
	return _checked;
}

void CheckBox::setChecked(bool checked){
	_checked = checked;
	if (_checked)
		checkedEvent.send();
	else
		uncheckedEvent.send();
	reDraw();
}

void CheckBox::reDraw(){
	TextButton::reDraw();  // let baseclass to most of the rendering, border caption etc
	
	if (_isShown) {
		Display::type_t *tft = Display::get();
		int boxX = _box.x + Styles::padding,
				boxY = _box.y + (_box.h / 2) - (_boxHeight / 2);
				
		uint16_t color = _hasFocus ? Styles::focusBorderColor : Styles::borderColor;
		
		// draw the box
		tft->drawRect(boxX, boxY, _boxWidth, _boxHeight, color);
		
		if (_checked) {
			// draw the check
			tft->fillRect(boxX + 2, boxY + 2, _boxWidth -4, _boxHeight -4, color);
		}
	}
}

void CheckBox::press(){
	toggle();
}

bool CheckBox::isChecked() const {
	return _checked;
}



// ------------------- RadioBox ---------------------------

RadioButton::RadioButton(uint16_t x, uint16_t y, const char *caption,  uint16_t w /*= 0*/, uint16_t h /*= 0*/) :
	CheckBox(x, y, caption, w, h) 
{
}

RadioButton::~RadioButton()
{
}

void RadioButton::reDraw(){
	TextButton::reDraw();
	
	if (_isShown){
		Display::type_t *tft = Display::get();
		int boxX = _box.x + Styles::padding + _circleRadius,
				boxY = _box.y + (_box.h / 2);
				
		uint16_t color = _hasFocus ? Styles::focusBorderColor : Styles::borderColor;
		
		// draw the box
		tft->drawCircle(boxX, boxY, _circleRadius, color);
		
		if (_checked) {
			// draw the checked circle
			tft->fillCircle(boxX, boxY, _circleRadius - 2, color);
		}
	}
}





// ----------------- SpinBox -------------------------------

SpinBox::SpinBox(uint16_t x, uint16_t y, int minValue, int maxValue, const char *caption, uint16_t w /*= 0*/, uint16_t h /*= 0*/) :
	ValueWgtBase(minValue, maxValue)
{
	_box.x = x;
	_box.y = y;
	
	String testMin(_minVlu);
	String testMax(_maxVlu);
	_spinBoxWidth = testMin.length() < testMax.length() ? testMax.length() : testMin.length();
	_spinBoxWidth *= Styles::charWidth;
	_spinBoxWidth += (Styles::padding * 2);
	
	if (w == 0 && h == 0) {
		w += _spinBoxWidth + Styles::padding;
		h += (Styles::padding * 2) + Styles::charHeight + (Styles::padding * 2);

		if (caption != 0) {
			_caption = caption;
			w += _caption.length() * Styles::charWidth + (Styles::padding *2);
			h =  h < (Styles::charHeight + (Styles::padding * 2)) ? Styles::charHeight + (Styles::padding *2) : h;
		}
	}
	
	_box.w = w;
	_box.h = h;
	
	_startX = _box.x + Styles::padding;
	_startY = _box.y + ((_box.h / 2) - (Styles::charHeight / 2));
}

SpinBox::~SpinBox()
{
}

	
void SpinBox::reDraw() {
	uint16_t color; 
	
	if (_isShown){
		if (_state)
			color = Styles::pressedBackColor;
		else if (_hasFocus)
			color = Styles::focusBackColor;
		else
			color = Styles::backCtlColor;
	} else {
		color = Styles::backColor;
	}
	
	Display::type_t *tft = Display::get();
	tft->fillRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, color);
	if (_isShown){
		color = _hasFocus ? Styles::focusBorderColor : Styles::borderColor;
		tft->drawRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, color);
		
		if (_caption.length() > 0){
			tft->setTextColor(color);
			tft->setCursor(_startX, _startY);
			tft->setTextSize(Styles::textSize);
			tft->print(_caption);
		}
		
		// draw the spinBox
		tft->fillRect(_startX + _caption.length() * Styles::charWidth + Styles::padding, _box.y + Styles::padding,
									_spinBoxWidth, (Styles::padding * 2) + Styles::charHeight, 
																		_hasFocus ? Styles::backColor : Styles::backCtlColor);
																		
		tft->drawRect(_startX + _caption.length() * Styles::charWidth + Styles::padding, _box.y + Styles::padding,
									_spinBoxWidth, (Styles::padding * 2) + Styles::charHeight, color);
									
		// draw the value
		tft->setCursor(_startX + _caption.length() * Styles::charWidth + (Styles::padding * 2), 
									_box.y + (Styles::padding * 2));
		tft->print(_value);
	}
}

// ----------------------- HSlider --------------------------

HSlider::HSlider(uint16_t x, uint16_t y, int minValue, int maxValue, uint16_t w, const char *caption /*= 0*/, uint16_t h /*= 0*/):
	ValueWgtBase(minValue, maxValue)
{
	_box.x = x;
	_box.y = y;
	_box.w = w;
	
	if (h == 0){
		h = (Styles::padding * 2) + _indicatorHeight;
	}
	
	_startY = _box.y + Styles::padding;
	
	if (caption != 0) {
		_caption = caption;
		h +=  Styles::charHeight;
		_startY += Styles::charHeight;
	}
	
	_box.h = h;
	
	_factor = (_box.w - (Styles::padding * 2)) / static_cast<float>(_maxVlu - _minVlu);
}

HSlider::~HSlider()
{
}

void HSlider::reDraw(){
	uint16_t color; 
	
	if (_isShown){
		if (_state)
			color = Styles::pressedBackColor;
		else if (_hasFocus)
			color = Styles::focusBackColor;
		else
			color = Styles::backCtlColor;
	} else {
		color = Styles::backColor;
	}
	
	Display::type_t *tft = Display::get();
	tft->fillRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, color);
	if (_isShown){
		color = _hasFocus ? Styles::focusBorderColor : Styles::borderColor;
		tft->drawRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, color);
		
		if (_caption.length() > 0){
			tft->setTextColor(color);
			tft->setCursor(_box.x + Styles::padding, _box.y + Styles::padding);
			tft->setTextSize(Styles::textSize);
			tft->print(_caption + ":" + _value);
		}
		
		// draw the ruler
		tft->fillRoundRect(_box.x + Styles::padding, _startY + ((_indicatorHeight - _rulerHeight) / 2),
											 _box.w - (Styles::padding *2), _rulerHeight, 2, color);
		
		// draw the indicator
		uint16_t Xoffset = static_cast<uint16_t>((_value - _minVlu) * _factor) - (_indicatorWidth / 2);
		tft->fillRoundRect(_box.x + Styles::padding + Xoffset, _startY, _indicatorWidth, _indicatorHeight, 2, color);
																		
	}
}



// --------------------- VSlider a vertical slider -----------------------------
VSlider::VSlider(uint16_t x, uint16_t y, int minValue, int maxValue, uint16_t h, const char *caption /*= 0*/, uint16_t w /*= 0*/):
	ValueWgtBase(minValue, maxValue)
{
	_box.x = x;
	_box.y = y;
	_box.h = h;
	
	if (w == 0){
		w = (Styles::padding * 2) + _indicatorWidth;
	}
	
	_startY = _box.y + Styles::padding;
	_endY = _box.y + _box.h - Styles::padding;
	
	
	if (caption != 0) {
		_caption = caption;
		uint16_t tmpC = _caption.length();
		uint16_t tmpMin = String(_minVlu).length();
		uint16_t tmpMax = String(_maxVlu).length();
		w = (tmpC > tmpMin ? tmpC : (tmpMin > tmpMax ? tmpMin : tmpMax)) * Styles::charWidth; // select the highest value
		w += Styles::padding * 2;
		_startY += Styles::charHeight + Styles::padding;
		_endY -= Styles::charHeight + Styles::padding;
	}
	
	_box.w = w;
	
	_startX = _box.x + (w / 2) - (_rulerWidth / 2);
	
	_factor = (_endY - _startY) / static_cast<float>(_maxVlu - _minVlu);
}

VSlider::~VSlider()
{
}

void VSlider::reDraw(){
	uint16_t color; 
	
	if (_isShown){
		if (_state)
			color = Styles::pressedBackColor;
		else if (_hasFocus)
			color = Styles::focusBackColor;
		else
			color = Styles::backCtlColor;
	} else {
		color = Styles::backColor;
	}
	
	Display::type_t *tft = Display::get();
	tft->fillRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, color);
	if (_isShown){
		color = _hasFocus ? Styles::focusBorderColor : Styles::borderColor;
		tft->drawRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, color);
		
		if (_caption.length() > 0){
			tft->setTextColor(color);
			tft->setCursor(_box.x + Styles::padding, _box.y + Styles::padding);
			tft->setTextSize(Styles::textSize);
			tft->print(_caption);
			// draw value
			tft->setCursor(_box.x + Styles::padding, _box.y + _box.h - Styles::padding - Styles::charHeight);
			tft->print(_value);
		}
		
		// draw the ruler
		tft->fillRoundRect(_startX, _startY, _rulerWidth, _endY - _startY, 2, color);
		
		// draw the indicator
		uint16_t Yoffset = static_cast<uint16_t>(((_maxVlu - _minVlu) - (_value - _minVlu)) * _factor) - (_indicatorHeight / 2);
		tft->fillRoundRect(_startX + (_rulerWidth / 2) - (_indicatorWidth / 2), 
											_startY + (_indicatorHeight / 2) + Yoffset, _indicatorWidth, _indicatorHeight, 2, color);
																		
	}
}



// ------------ Label (text holder) --------------
Label::Label(uint16_t x, uint16_t y, const char *text /*= 0*/, bool drawBorder /*= false*/,  uint16_t w /*= 0*/, uint16_t h /*= 0*/, Align align /*=Align::Left*/) :
	WidgetBase(), _setWidth(w), _setHeight(h), _drawBorder(drawBorder), _align(align)
{
	_box.x = x;
	_box.y = y;
	
	if (text != 0){
		_text = text;
	}
	
	_calcSize();
}

Label::~Label()
{
}

// recalulates the size needed to display it
void Label::_calcSize() {	
	if (_setWidth > 0 && _setHeight > 0) {
		// user defined
		_box.w = _setWidth;
		_box.h = _setHeight;
	} else {
		// autosize, build until tft edge
		_mangleText(false);
	}
}

const String Label::getText() const {
	return _text;
}

void Label::setText(const String text){
	// clear old text before we resize
	Display::get()->fillRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, Styles::labelBackColor);
	
	_text = text;
	_calcSize();
	reDraw();
}


void Label::reDraw() {
	Display::type_t *tft = Display::get();
	
	// draw background (clear old text)
	if (_drawBorder) 
		tft->fillRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, Styles::labelBackColor);
	else 
		tft->fillRect(_box.x, _box.y, _box.w, _box.h, Styles::labelBackColor);
	
	
	if (_isShown) {
		// draw border
		//uint8_t pad = 0;
		if (_drawBorder){
			tft->drawRoundRect(_box.x, _box.y, _box.w, _box.h, Styles::borderRadius, Styles::labelBorderColor);
			//pad = Styles::padding;
		}
		
		tft->setTextColor(Styles::labelTextColor);
		tft->setTextSize(Styles::textSize);
		
		_mangleText(true); // draw the text in this function (shares algorithm with size calculator)
	}
}

void Label::_mangleText(bool draw){
	// this function draws or is used to calculate the size (may happen at different times)
	uint8_t pad = 0;
	if (_drawBorder){
		pad = Styles::padding;
	}
	Display::type_t *tft = Display::get();

	uint16_t x = _box.x + pad,
					 y = _box.y + pad,
					 maxX = draw ? _box.x + _box.w : tft->width() -1; // pixels is 0 based
					 
	String row; uint16_t mostX = 0, mostY = 0;
	for(uint16_t i = 0; i < _text.length(); ++i){
		x += Styles::charWidth; 
		
		row += _text[i];
		
		
		if (x + pad >= maxX || i  == _text.length() -1) {
			if (i < _text.length() -1) {
				if (row[row.length() -1] != ' ') { // try to back to a clear whitespace
					int p = row.lastIndexOf(' ');
					if (p > 0) {
						i -= row.length() - p -1; row = row.substring(0, p);
					}
				} else {
					// find first non whitespace
					int p = row.length();
					while(p-- > 0 && row[p] == ' ')
						;
					row = row.substring(0, p +1);
				}
			}
			
			if (draw){
				switch(_align){ // do the text alignment
				case Align::Center:
					tft->setCursor(_box.x + (_box.w / 2) - ((row.length() * Styles::charWidth) / 2), y);
					break;
				case Align::Right:
					tft->setCursor(_box.x + ((_box.w - pad) - (row.length() * Styles::charWidth)), y);
					break;
				case Align::Left: // falltrough
				default:
					tft->setCursor(_box.x + pad, y);
					break;
				}
				tft->print(row); // print row
			} else if (x > mostX) {
				 mostX = x; // used when calc size
			}
			row = "";
			y += Styles::charHeight + Styles::lineSpace;
			x = _box.x + Styles::charWidth + pad;
		}
		
		if (y > mostY) mostY = y; // used when calc size
	}
	
	// set size
	if (!draw){
		_box.w = mostX - _box.x;
		_box.h = mostY - _box.y;
	}

}

void Label::setAlignment(Align align){
	_align = align;
	reDraw();
}



