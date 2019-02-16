#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include "./OpenWeatherMap.h"

const char *ow_key = "26c131ba210fe0e2794c5a68f821005d";
const char *nodename = "esp8266-weather";
const char *wifi_ssid = "notvisable";
const char *wifi_passwd = "44482477";

//==============================================================================================================

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

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n\n\n");

  connectWiFiInit();
  WF_status = W_TRY;
}

void currentConditions(void)
{
  OWM_conditions *ow_cond = new OWM_conditions;
  owCC.updateConditions(ow_cond, ow_key, "de", "koeln", "metric");
  Serial.println("icon: " + ow_cond->icon + ", temp.: " + ow_cond->temp + ", " + ow_cond->description);
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
