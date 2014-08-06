#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#include <DList.h>
#include <mgui.h>
#include <eimg_arduino.h>
#include "pinouts.h"
#include "pages.h"


Display disp(TFT_CS, TFT_DC, TFT_RESET); // initialize display controller

// define application pages (implemented in pages.h and pages.cpp)
PageController    pageCtl;
WelcomePage       welcomePage;
LabelsPage        labelsPage;
ButtonsPage       buttonsPage;
ToggleButtonsPage togglePage;
ValueChangePage   valueChangePage;

void setup(){
  Serial.begin(115200);
  
  // add pages to page controller
  pageCtl.addPage(welcomePage);
  pageCtl.addPage(labelsPage);
  pageCtl.addPage(buttonsPage);
  pageCtl.addPage(togglePage);
  pageCtl.addPage(valueChangePage);
  
  // goto welcome page
  pageCtl.gotoPage(PageController::welcomePage);
}

void loop() {
  pageCtl.eventLoop(); // checks input change, let widgets do periodic stuff (like blinking and such)
  delay(10); // or do other things
}
