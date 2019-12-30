
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
#include "webserver.h"

// connect to the given wifi
void connectWiFiInit(void)
{
  WiFi.hostname("LEDMatrix");
  String ssid = wifi_ssid;
  String passwd = wifi_passwd;
  WiFi.begin(ssid.c_str(), passwd.c_str());
}

// time zone offset
const long utcOffsetInSeconds = 3600;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

void setup()
{
  Serial.begin(115200);

  setupDisplay();
  setupClock();

  connectWiFiInit();
  timeClient.begin();
  setupWebserver();
}

void loop()
{
  loopWebserver();

  if (clockMode)
  {
    timeClient.update();
    display.clearDisplay();
    drawClock(timeClient.getHours(), timeClient.getMinutes());
    delay(500);
  }
  else
  {
    delay(10);
  }
}
