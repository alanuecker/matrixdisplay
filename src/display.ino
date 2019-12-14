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

// Some standard colors
uint16_t RED = display.color565(255, 0, 0);
uint16_t GREEN = display.color565(0, 255, 0);
uint16_t BLUE = display.color565(0, 0, 255);
uint16_t WHITE = display.color565(255, 255, 255);
uint16_t YELLOW = display.color565(255, 255, 0);
uint16_t CYAN = display.color565(0, 255, 255);
uint16_t MAGENTA = display.color565(255, 0, 255);
uint16_t BLACK = display.color565(0, 0, 0);

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
  delay(1000);
}
