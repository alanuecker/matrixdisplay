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

String dateDay(String timestamp)
{
  time_t ts = timestamp.toInt();
  char buff[30];
  sprintf(buff, "%02d-%02d-%4d", day(ts), month(ts), year(ts));
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

class FCastDay
{
public:
  String dt;
  String min_temp;
  String min_icon;
  String max_temp;
  String max_icon;
};

void fiveDayFcast(void)
{
  text(1, 12, "Forecast:", display.color565(96, 96, 250));
  // todo: it is not possible to display text in the next block for some reasons
  OWM_fiveForecast *ow_fcast5 = new OWM_fiveForecast[24];
  byte entries = owF5.updateForecast(ow_fcast5, 24, ow_key, "de", "koeln", "metric");
  Serial.print("Entries: ");
  Serial.println(entries + 1);

  String lastDay = "none";
  float maxTemp = 0.0;
  float minTemp = 0.0;
  String maxTempIcon = "none";
  String minTempIcon = "none";
  int checkedDays = 0;

  FCastDay fcastDays[3];

  for (byte i = 0; i <= entries; ++i)
  {
    String day = dateDay(ow_fcast5[i].dt);
    float fcastTemp = ow_fcast5[i].temp.toFloat();

    if (fcastTemp < minTemp || minTempIcon == "none")
    {
      minTemp = fcastTemp;
      minTempIcon = ow_fcast5[i].icon;
    }

    if (fcastTemp > maxTemp || maxTempIcon == "none")
    {
      maxTemp = fcastTemp;
      maxTempIcon = ow_fcast5[i].icon;
    }

    if (lastDay != day)
    {
      if (lastDay != "none")
      {
        fcastDays[checkedDays].dt = day;
        fcastDays[checkedDays].min_temp = minTemp;
        fcastDays[checkedDays].max_temp = maxTemp;
        fcastDays[checkedDays].min_icon = minTempIcon;
        fcastDays[checkedDays].max_icon = maxTempIcon;

        // increase counter and reset values
        checkedDays++;
        maxTemp = 0.0;
        minTemp = 0.0;
        maxTempIcon = "none";
        minTempIcon = "none";

        // break loop after 3 entries
        if (checkedDays > 2)
          break;
      }
      lastDay = day;
    }
  }

  for (byte j = 0; j <= 3; j++)
  {
    String line = fcastDays[j].dt + " " + fcastDays[j].min_temp + " " + fcastDays[j].max_temp;
    Serial.println(line);
    // todo: displaying text here does not work
  }

  delete[] ow_fcast5;
}

void test(void)
{
  text(1, 20, "test", display.color565(96, 96, 250));
}

void loop()
{
  if (WF_status == W_TRY)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      MDNS.begin(nodename);
      WF_status = W_READY;
      Serial.println("Five days forecast: ");
      fiveDayFcast();
    }
  }
  delay(1000);
}
