#include <PxMatrix.h>
#include <Ticker.h>

// create ticker
Ticker display_ticker;

// pins for led matrix
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

// create display
PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

// switch between clock and draw mode
boolean clockMode = true;

// draw simple text
void text(uint8_t xpos, uint8_t ypos, String text, uint16_t color)
{
  display.setTextColor(color);
  display.setCursor(xpos, ypos);
  display.print(text);
}

// get a random color
uint16_t randomColor()
{
  return display.color565(rand() % 255, rand() % 255, rand() % 255);
}

// function for ticker
void display_updater()
{
  // This defines the 'on' time of the display is us. The larger this number,
  // the brighter the display. If too large the ESP will crash
  display.display(20);
}

// set display settings
void setupDisplay()
{
  display.begin(16);
  display.setBrightness(40);
  display.setFastUpdate(true);
  display.clearDisplay();
  display_ticker.attach(0.002, display_updater);
}
