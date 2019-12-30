#include <PxMatrix.h>

#include "digits.h"
#include "display.h"

// Object to store the current position on animation direction
struct ClockDisplay
{
  int x, y;
  boolean goRight, goDown;
} clockDisplay;

// draw a single digit at the given position
void drawDigit(int xPos, int yPos, int digit, uint16_t color)
{
  for (int y = 0; y < 14; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      if (digits[digit][y][x] == 1)
      {
        display.drawPixel(xPos + x, yPos + y, color);
      }
    }
  }
}

// this will draw the clock at a new position
void drawClock(int hours, int minutes)
{
  // new clock position
  int newx = 0;
  int newy = 0;

  if (clockDisplay.goRight)
  {
    newx = clockDisplay.x + 1;
  }
  else
  {
    newx = clockDisplay.x - 1;
  }
  // change the direction if an edge is reached
  if (newx > 18)
  {
    newx = clockDisplay.x - 1;
    clockDisplay.goRight = false;
  }
  else if (newx < 0)
  {
    newx = clockDisplay.x + 1;
    clockDisplay.goRight = true;
  }

  if (clockDisplay.goDown)
  {
    newy = clockDisplay.y + 1;
  }
  else
  {
    newy = clockDisplay.y - 1;
  }

  if (newy > 18)
  {
    newy = clockDisplay.y - 1;
    clockDisplay.goDown = false;
  }
  else if (newy < 1)
  {
    newy = clockDisplay.y + 1;
    clockDisplay.goDown = true;
  }

  // apply movement
  clockDisplay.x = newx;
  clockDisplay.y = newy;

  // get the first digit of the hour or prefix with zero
  int h0 = hours < 10 ? 0 : (hours / 10) % 10;
  // get the second digit of the hour
  int h1 = hours < 10 ? hours : hours % 10;
  // get minutes
  int m0 = minutes < 10 ? 0 : (minutes / 10) % 10;
  int m1 = minutes < 10 ? minutes : minutes % 10;
  // get random colors for the clock and the shadow
  uint16_t color = randomColor();
  uint16_t shadow = randomColor();
  // draw the shadow before the clock digit and apply an offset of 1
  drawDigit(clockDisplay.x + 1, clockDisplay.y - 1, h0, shadow);
  // draw the clock digit
  drawDigit(clockDisplay.x, clockDisplay.y, h0, color);
  // draw next digit with an offset
  drawDigit(clockDisplay.x + 12, clockDisplay.y - 1, h1, shadow);
  drawDigit(clockDisplay.x + 11, clockDisplay.y, h1, color);
  // draw the divider between hours and minutes
  drawDigit(clockDisplay.x + 20, clockDisplay.y - 1, 10, shadow);
  drawDigit(clockDisplay.x + 19, clockDisplay.y, 10, color);

  drawDigit(clockDisplay.x + 28, clockDisplay.y - 1, m0, shadow);
  drawDigit(clockDisplay.x + 27, clockDisplay.y, m0, color);

  drawDigit(clockDisplay.x + 38, clockDisplay.y - 1, m1, shadow);
  drawDigit(clockDisplay.x + 37, clockDisplay.y, m1, color);
}

// initial clock position and movement direction
void setupClock()
{
  clockDisplay.x = 1;
  clockDisplay.y = 1;
  clockDisplay.goRight = true;
  clockDisplay.goRight = false;
}
