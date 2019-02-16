#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

#include <Time.h>
#include <TimeLib.h>
#include <SPI.h>
#include <ArduinoJson.h>

#include <PxMatrix.h>
#include <Ticker.h>
#include <Fonts/Picopixel.h>

#include "./icons/icons.h"
#include "secrets.h"
#include "./OpenWeatherMap.h"

typedef enum wifi_s
{
  W_AP = 0,
  W_READY,
  W_TRY
} WifiStat;

OWMconditions owCC;
OWMfiveForecast owF5;
WifiStat WF_status;

void connectWiFiInit(void)
{
  WiFi.hostname(nodename);
  String ssid = wifi_ssid;
  String passwd = wifi_passwd;
  WiFi.begin(ssid.c_str(), passwd.c_str());
}

String dateTime(String timestamp)
{
  time_t ts = timestamp.toInt();
  char buff[30];
  sprintf(buff, "%2d:%02d %02d-%02d-%4d", hour(ts), minute(ts), day(ts), month(ts), year(ts));
  return String(buff);
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

union single_double {
  uint8_t two[2];
  uint16_t one;
} this_single_double;

// draw weather icons
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
  display.display(70);
}

void setup()
{
  Serial.begin(115200);

  display.begin(16);
  display.clearDisplay();
  display_ticker.attach(0.002, display_updater);

  display.setFont(&Picopixel);

  connectWiFiInit();
  WF_status = W_TRY;
}

void currentConditions(void)
{
  OWM_conditions *ow_cond = new OWM_conditions;
  owCC.updateConditions(ow_cond, ow_key, "de", "koeln", "metric");
  Serial.println("icon: " + ow_cond->icon + ", temp.: " + ow_cond->temp + ", " + ow_cond->description);
  text(1, 5, "Temp.: " + ow_cond->temp + "C", display.color565(96, 96, 250));
  delete ow_cond;
}

void fiveDayFcast(void)
{
  OWM_fiveForecast *ow_fcast5 = new OWM_fiveForecast[20];
  byte entries = owF5.updateForecast(ow_fcast5, 20, ow_key, "de", "koeln", "metric");
  Serial.print("Entries: ");
  Serial.println(entries + 1);
  for (byte i = 0; i <= entries; ++i)
  {
    Serial.print(dateTime(ow_fcast5[i].dt));
    Serial.print(", temp.: " + ow_fcast5[i].temp);
    Serial.println(", descr.: " + ow_fcast5[i].description);
  }
  delete[] ow_fcast5;
}

void loop()
{
  if (WF_status == W_TRY)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      MDNS.begin(nodename);
      WF_status = W_READY;
      Serial.println("Current Conditions: ");
      currentConditions();
      Serial.println("Five days forecast: ");
      fiveDayFcast();
    }
  }
  delay(1000);
}
