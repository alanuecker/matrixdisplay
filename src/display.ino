#include <stdlib.h>

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

#include <Time.h>
#include <NTPClient.h>
#include <SPI.h>

#include <PxMatrix.h>
#include <Ticker.h>
#include <Fonts/Org_01.h>

#include "secrets.h"

typedef enum wifi_s
{
  W_AP = 0,
  W_READY,
  W_TRY
} WifiStat;

WifiStat WF_status;

void connectWiFiInit(void)
{
  WiFi.hostname("LEDMatrix");
  String ssid = wifi_ssid;
  String passwd = wifi_passwd;
  WiFi.begin(ssid.c_str(), passwd.c_str());
}

#define N_GRAINS 64
#define WIDTH 64
#define HEIGHT 32

struct Grain
{
  int16_t x, y;
} grain[N_GRAINS];

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

#define NUM_COLORS 7

// Some standard colors
uint16_t RED = display.color565(255, 0, 0);
uint16_t GREEN = display.color565(0, 255, 0);
uint16_t BLUE = display.color565(0, 0, 255);
uint16_t WHITE = display.color565(255, 255, 255);
uint16_t YELLOW = display.color565(255, 255, 0);
uint16_t CYAN = display.color565(0, 255, 255);
uint16_t MAGENTA = display.color565(255, 0, 255);
uint16_t BLACK = display.color565(0, 0, 0);

uint16_t COLORS[8] = {RED, GREEN, BLUE, WHITE, YELLOW, CYAN, MAGENTA, BLACK};

const long utcOffsetInSeconds = 3600;

char daysOfTheWeek[7][12] = {"SO", "MO", "DI", "MI", "DO", "FR", "SA"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

// draw simple text
void text(uint8_t xpos, uint8_t ypos, String text, uint16_t color)
{
  display.setTextColor(color);
  display.setCursor(xpos, ypos);
  display.print(text);
}

uint16_t randomColor()
{
  return display.color565(rand() % 255, rand() % 255, rand() % 255);
}

void pixelTask()
{
  int i;
  int16_t newx, newy;

  for (i = 0; i < N_GRAINS; i++)
  {
    newx = grain[i].x + 0;
    newy = grain[i].y + (rand() % 5 + 1);

    if (newx > WIDTH)
    {
      newx = 0;
    }
    if (newy > HEIGHT)
    {
      newy = 0;
    }

    grain[i].x = newx;
    grain[i].y = newy;
  }
}

void drawGrain(int i)
{
  uint16_t color = randomColor();
  display.drawPixel(grain[i].x, grain[i].y, color);
  display.drawPixel(grain[i].x - 1, grain[i].y, color);
  display.drawPixel(grain[i].x + 1, grain[i].y, color);
  display.drawPixel(grain[i].x, grain[i].y - 1, color);
  display.drawPixel(grain[i].x, grain[i].y + 1, color);
}

// function for ticker
void display_updater()
{
  // This defines the 'on' time of the display is us. The larger this number,
  // the brighter the display. If too large the ESP will crash
  display.display(20);
}

void setup()
{
  Serial.begin(115200);

  display.begin(16);
  display.setBrightness(50);
  display.setFastUpdate(true);
  display.clearDisplay();
  display_ticker.attach(0.002, display_updater);

  display.setTextSize(1);
  display.setFont(&Org_01);

  connectWiFiInit();
  WF_status = W_TRY;
  timeClient.begin();

  for (int i = 0; i < WIDTH; i++)
  {
    grain[i].x = i;
    grain[i].y = 1;
  }
}

void loop()
{
  if (WF_status == W_TRY)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      text(1, 5, "Connected", GREEN);
      WF_status = W_READY;
      delay(1000);
    }
  }

  timeClient.update();
  display.clearDisplay();
  text(1, 5, daysOfTheWeek[timeClient.getDay()], RED);
  text(1, 20, timeClient.getFormattedTime(), RED);

  pixelTask();

  for (int i = 0; i < WIDTH; i++)
  {
    drawGrain(i);
  }

  delay(1000);
}
