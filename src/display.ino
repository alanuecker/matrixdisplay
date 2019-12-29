#include <stdlib.h>

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

#include <Time.h>
#include <NTPClient.h>
#include <SPI.h>

#include "secrets.h"
#include "clock.h"
#include "display.h"

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

const long utcOffsetInSeconds = 3600;

char daysOfTheWeek[7][3] = {"SO", "MO", "DI", "MI", "DO", "FR", "SA"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

void setup()
{
  Serial.begin(115200);

  setupDisplay();
  setupClock();

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
      WF_status = W_READY;
    }
  }

  timeClient.update();
  clearDisplay();

  drawClock(timeClient.getHours(), timeClient.getMinutes());

  delay(500);
}
