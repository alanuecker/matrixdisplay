#include <PxMatrix.h>

#include "digits.h"
#include "display.h"

struct ClockDisplay
{
  int x, y;
  boolean goRight, goDown;
} clockDisplay;

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

void drawClock(int hours, int minutes)
{
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

  clockDisplay.x = newx;
  clockDisplay.y = newy;

  int h0 = hours < 10 ? 0 : (hours / 10) % 10;
  int h1 = hours < 10 ? hours : hours % 10;

  int m0 = minutes < 10 ? 0 : (minutes / 10) % 10;
  int m1 = minutes < 10 ? minutes : minutes % 10;

  uint16_t color = randomColor();
  uint16_t shadow = randomColor();

  drawDigit(clockDisplay.x + 1, clockDisplay.y - 1, h0, shadow);
  drawDigit(clockDisplay.x, clockDisplay.y, h0, color);

  drawDigit(clockDisplay.x + 12, clockDisplay.y - 1, h1, shadow);
  drawDigit(clockDisplay.x + 11, clockDisplay.y, h1, color);

  drawDigit(clockDisplay.x + 20, clockDisplay.y - 1, 10, shadow);
  drawDigit(clockDisplay.x + 19, clockDisplay.y, 10, color);

  drawDigit(clockDisplay.x + 28, clockDisplay.y - 1, m0, shadow);
  drawDigit(clockDisplay.x + 27, clockDisplay.y, m0, color);

  drawDigit(clockDisplay.x + 38, clockDisplay.y - 1, m1, shadow);
  drawDigit(clockDisplay.x + 37, clockDisplay.y, m1, color);
}

void setupClock()
{
  clockDisplay.x = 1;
  clockDisplay.y = 1;
  clockDisplay.goRight = true;
  clockDisplay.goRight = false;
}
