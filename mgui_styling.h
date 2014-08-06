#ifndef MGUI_STYLING
#define MGUI_STYLING

// this file contains all the user setable styles in mgui

struct Styles {
	static const uint16_t 
		backColor = 0xFFFF,			// background color, WHITE
    backCtlColor = 0xEDE5, 	// backgroundColor of buttons, dark yellow
    borderColor = 0x07FF, 	// border color CYAN,
    focusBackColor = 0xEFAD, // backcolor when control has focus, darker yellow
    focusBorderColor = 0x001F, // border color when control has focus BLUE
    pressedBackColor = 0xFFE0, // background color when pressed YELLOW
    pressedBorderColor = 0x001F, // border color when button is pressed
    captionColor = 0x07FF,     // text color on button when no focus
    focusCaptionColor = 0x001F, // text color when button has focus
    pressedCaptionColor = 0x001F, // caption color when button is pressed
    labelBackColor = 0xFFFF,     // background form labels all white
    labelBorderColor = 0x0000,    // border color for labels
    labelTextColor = 0x001F;      // text color of a Label
    
  static const uint8_t
  		textSize = 1;						// set to 2 if you want to double text size, 3 tripple etc
    
	static const uint8_t 
			padding = 4,						// distance to border internally in widgets ie from text to border in pixels
  		charWidth = 6 * textSize,			// how many pixels each char takes on X axis
  		charHeight = 8 * textSize,		// how many pixels each char takes on the Y axis
  		lineSpace = 2,							// how many pixels between 2 rows of text
  		borderRadius = 4;						// how many pixels the rounded corner should be
};

#endif
