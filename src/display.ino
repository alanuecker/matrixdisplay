
#include <PxMatrix.h>
#include <Ticker.h>
#include <Fonts/Picopixel.h>

#include "./icons/icons.h"

Ticker display_ticker;

// pins for led matrix
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

// Some standard colors
uint16_t RED = display.color565(255, 0, 0);
uint16_t GREEN = display.color565(0, 255, 0);
uint16_t BLUE = display.color565(0, 0, 255);
uint16_t WHITE = display.color565(255, 255, 255);
uint16_t YELLOW = display.color565(255, 255, 0);
uint16_t CYAN = display.color565(0, 255, 255);
uint16_t MAGENTA = display.color565(255, 0, 255);
uint16_t BLACK = display.color565(0, 0, 0);

uint16_t myCOLORS[8] = {RED, GREEN, BLUE, WHITE, YELLOW, CYAN, MAGENTA, BLACK};

void display_updater()
{
  display.display(70);
}

void setup()
{
  Serial.begin(115200);
  display.begin(16);
  display.clearDisplay();
  display_ticker.attach(0.002, display_updater);

  display.setFont(&Picopixel);
}

union single_double {
  uint8_t two[2];
  uint16_t one;
} this_single_double;

// This draws the weather icons
void draw_weather_icon(uint8_t icon)
{
  if (icon > 10)
    icon = 10;
  for (int yy = 0; yy < 10; yy++)
  {
    for (int xx = 0; xx < 10; xx++)
    {
      uint16_t byte_pos = (xx + icon * 10) * 2 + yy * 220;
      this_single_double.two[1] = weather_icons[byte_pos];
      this_single_double.two[0] = weather_icons[byte_pos + 1];
      display.drawPixel(1 + xx, yy, this_single_double.one);
    }
  }
}

void text(uint8_t xpos, uint8_t ypos, String text, uint16_t color)
{
  display.setTextColor(color);
  display.setCursor(xpos, ypos);
  display.print(text);
}

uint8_t icon_index = 0;
void loop()
{
  text(1, 17, "Welcome to PxMatrix!", display.color565(96, 96, 250));
  draw_weather_icon(icon_index);
  icon_index++;
  if (icon_index > 10)
    icon_index = 0;

  delay(3000);
}
