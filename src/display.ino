#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h" // https://github.com/electronicsguy/ESP8266/tree/master/HTTPSRedirect

#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)

#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h> // https://github.com/JChristensen/Timezone
#include "time_ntp.h" // NTP & time routines for ESP8266 for ESP8266 adapted Arduino IDE by Stefan Thesen 05/2015 - free for anyone

#include <SPI.h>

#include <PxMatrix.h>
#include <Ticker.h>
#include <Fonts/Picopixel.h>

#include "./icons/icons.h"
#include "secrets.h"

// Params needed to fetch events from Google Calendar
const char* dstHost = "script.google.com";
int const dstPort = 443;
int32_t const timeout = 2000;
int const GoogleServerMaxRetry = 1; //maximum tries to reach google server.

// define buffersize for several buffers - needs to be tidy, otherwise HTTPS crashs due to low free heap
int const bufferSize = 1350;

// for time conversion
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};   //Central European Standard Time
Timezone CE(CEST, CET);

// ntp flag
bool bNTPStarted = false;

//------ NETWORK VARIABLES---------
char* const City = "Cologne";  // Your home city
char* const Country = "de";    // Your country
char* const LangCode = "en";   // Language Code for localization
char* const DataMode = "json"; // Data mode for response
char* const Units = "metric";  // Units for numbers -> metric, imperial
char* const RowCount = "3";    // JSON data set count, like list entries in forecast

char* const owmserver = "api.openweathermap.org"; // Address for OpenWeatherMap

unsigned long updateInterval = 1L * 60L * 1000000L; //Delay between updates, in microseconds for deepsleep - this relates to 1 minutes

int ConnectionTimedOut = 0;

//################ PROGRAM VARIABLES and OBJECTS ################
// Conditions
String icon0, high0, low0, temp0, pressure0, humidity0, datetime0,
    icon1, high1, low1, temp1, pressure1, humidity1, datetime1;

// Actual
String sunrise, sunset, conditionsa, icona, temp_lowa, temp_higha, tempa, pressurea, humiditya, datetimea;

// Google calendar
String appointment0, appointment1, appointment2, appointment3;
String appdate0, appdate1, appdate2, appdate3;
String appDateString0, appDateString1, appDateString2, appDateString3;

// Helper string to get proper phrase from weather-icon-id
String conditionPhrase;
String localTimeString;

char* const weekDay0 = "Montag";
char* const weekDay1 = "Dienstag";
char* const weekDay2 = "Mittwoch";
char* const weekDay3 = "Donnerstag";
char* const weekDay4 = "Freitag";
char* const weekDay5 = "Samstag";
char* const weekDay6 = "Sonntag";

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

  unsigned long startTime = millis();

  WiFi.begin(ssid, password); // connect to network
  DPRINTLN(F("Connecting to:"));
  DPRINT(ssid);

  // show connection
  int wifiTry = 0;
  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(1000);
    DPRINT(++wifiTry);
    DPRINT(' ');
  }

  DPRINTLN("Connection established!");
  DPRINTLN("IP address:\t");
  DPRINT(WiFi.localIP());

  display.begin(16);
  display.clearDisplay();
  display_ticker.attach(0.002, display_updater);

  display.setFont(&Picopixel);

  DPRINT(F("Setup time:"));
  DPRINTLN(startTime);


  ////////////////////////////////////////////
  // below here we got a valid WIFI connection
  ////////////////////////////////////////////

  // connect to NTP and get time each day
  // timelib now synced up to UTC

  DPRINTLN(F("Setup sync with NTP service."));
  setSyncProvider(getNTP_UTCTime1970);

  // UTC
  time_t tT = now();
  DPRINT("UTC: ");
  DPRINTLN(ctime(&tT));

  // local time
  time_t tTlocal = CE.toLocal(tT);
  DPRINT(F("Local Time: "));
  DPRINTLN(ctime(&tTlocal));

  localTimeString = weekday(tTlocal);

  if (localTimeString == "2")
    localTimeString = weekDay0;
  else if (localTimeString == "3")
    localTimeString = weekDay1;
  else if (localTimeString == "4")
    localTimeString = weekDay2;
  else if (localTimeString == "5")
    localTimeString = weekDay3;
  else if (localTimeString == "6")
    localTimeString = weekDay4;
  else if (localTimeString == "7")
    localTimeString = weekDay5;
  else if (localTimeString == "1")
    localTimeString = weekDay6;
  else
    localTimeString = " ";

  localTimeString += F(" - ");
  localTimeString += day(tTlocal) / 10;
  localTimeString += day(tTlocal) % 10;
  localTimeString += F(".");
  localTimeString += month(tTlocal) / 10;
  localTimeString += month(tTlocal) % 10;
  localTimeString += F(".");
  localTimeString += year(tTlocal);
  localTimeString += F(" - ");
  localTimeString += hour(tTlocal) / 10;
  localTimeString += hour(tTlocal) % 10;
  localTimeString += F(":");
  localTimeString += minute(tTlocal) / 10;
  localTimeString += minute(tTlocal) % 10;

  DPRINT(F("localTimeString: "));
  DPRINT(localTimeString);

  unsigned long currentTime = millis();
  unsigned long elapsedTime = (currentTime - startTime);
  Serial.print(F("Time since start until weather start: "));
  Serial.print(elapsedTime);
  Serial.println(F(" milliseconds"));
  elapsedTime = currentTime;

  // obtain_forecast("forecast");
  // obtain_forecast("weather");

  currentTime = millis();
  unsigned long weatherTime = (currentTime - elapsedTime);
  Serial.print(F("Time to get weather done: "));
  Serial.print(weatherTime);
  Serial.println(F(" milliseconds"));
  elapsedTime = currentTime;

  syncCalendar();
  DPRINTLN(F("Fetching data is done now."));

  currentTime = millis();
  unsigned long googleTime = (currentTime - elapsedTime);
  Serial.print(F("Time used for google calendar sync: "));
  Serial.print(googleTime);
  Serial.println(F(" milliseconds"));

  WiFi.forceSleepBegin(); // power down WiFi, as it is not needed anymore.

  DPRINTLN(F("Powering down WiFi now!"));

  //last check for adjusting the deep sleep intervall

  int localHour = hour(tTlocal);

  DPRINT(F("localHour: "));
  DPRINTLN(localHour);

  if (localHour >= 6 && localHour <= 9)
    updateInterval = 15L * 60L * 1000000L; // 15 minutes in the morning from 06:00 till 09:59
  else if (localHour >= 18 && localHour <= 20)
    updateInterval = 15L * 60L * 1000000L; // 15 minutes in the evening from 18:00 till 20:59
  else
    updateInterval = 60U * 60L * 1000000L; // 60 minutes for the rest of the day

  // todo: update display here

  unsigned long updateMinutes = updateInterval / (1000000L * 60L);
  Serial.print(F("Next update in: "));
  Serial.print(updateMinutes);
  Serial.println(F(" minutes."));

  //Good Bye
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

//
// Localize condition strings based on OpenWeatherMap icon-id
//

// String getConditionPhrase(String iconstatus)
// {

//   String phrase;
//   if (iconstatus == "10d" || iconstatus == "10n")
//     phrase = "eventl. Regen"; // expect rain
//   else if (iconstatus == "09d" || iconstatus == "09n")
//     phrase = "Regen"; // rain
//   else if (iconstatus == "13d" || iconstatus == "13n")
//     phrase = "Schnee"; // snow
//   else if (iconstatus == "01d")
//     phrase = "Klarer Himmel"; // sunny
//   else if (iconstatus == "01n")
//     phrase = "Klare Nacht"; // moon
//   else if (iconstatus == "02d")
//     phrase = "teilw. sonnig"; // partly sunny
//   else if (iconstatus == "02n")
//     phrase = "meist klar"; // partly moon
//   else if (iconstatus == "03d")
//     phrase = "meist wolkig"; // mostly cloudy
//   else if (iconstatus == "03n")
//     phrase = "meist wolkig"; // mostly claudy moon
//   else if (iconstatus == "04d" || iconstatus == "04n")
//     phrase = "Bedeckt"; // cloudy
//   else if (iconstatus == "11d" || iconstatus == "11n")
//     phrase = "Gewitter"; // thunderstorms
//   else if (iconstatus == "50d" || iconstatus == "50n")
//     phrase = "Nebel"; // fog
//   else
//     phrase = "";

//   return phrase;
// }

// Map icon drawing function based on OpenWeatherMap icon-id
//

// void DisplayConditionIcon(int x, int y, String IconName, int scale)
// {

//   DPRINTLN(IconName);
//   if (IconName == "09d" || IconName == "09n" || IconName == "10d" || IconName == "10n")
//   // todo: rain
//   else if (IconName == "13d" || IconName == "13n")
//   // todo: snow
//   else if (IconName == "01d")
//   // todo: sunny
//   else if (IconName == "01n")
//   // todo: moon
//   else if (IconName == "02d")
//   // todo: mostly sunny
//   else if (IconName == "02n")
//   // todo: mostly moon
//   else if (IconName == "03d")
//   // todo: mostly cloudy
//   else if (IconName == "03n")
//   // todo: mostly cloudy moon
//   else if (IconName == "04d" || IconName == "04n")
//   // todo: cloudy
//   else if (IconName == "11d" || IconName == "11n")
//   // todo: storm
//   else if (IconName == "50d" || IconName == "50n")
//   // todo: fog
//   else  // todo: no data
// }

// void obtain_forecast(String forecast_type)
// {

//   char RxBuf[bufferSize]; // made local char, rather than static
//   String request;

//   request = "GET /data/2.5/" + forecast_type + "?q=" + City + "," + Country + "&APPID=" + API_key + "&mode=" + DataMode + "&units=" + Units + "&lang=" + LangCode + "&cnt=" + RowCount + " HTTP/1.1\r\n";
//   request += F("User-Agent: Weather Webserver");
//   request += F("\r\n");
//   request += F("Accept: */*\r\n");
//   request += "Host: " + String(owmserver) + "\r\n";
//   request += F("Connection: close\r\n");
//   request += F("\r\n");
//   DPRINTLN(request);
//   DPRINT(F("Connecting to "));
//   DPRINTLN(owmserver);
//   WiFiClient httpclient;
//   //WiFi.status(); // ?
//   if (!httpclient.connect(owmserver, 80))
//   {
//     Serial.println(F("connection failed initially"));
//     httpclient.flush();
//     httpclient.stop();

//     return;
//   }
//   DPRINT(request);
//   httpclient.print(request); //send the http request to the server
//   // Collect http response headers and content from Weather Underground, discarding HTTP headers, the content is JSON formatted and will be returned in RxBuf.
//   uint16_t respLen = 0;
//   bool skip_headers = true;
//   String rx_line;
//   while (httpclient.connected() || httpclient.available())
//   {
//     if (skip_headers)
//     {
//       rx_line = httpclient.readStringUntil('\n');
//       DPRINTLN(rx_line);
//       if (rx_line.length() <= 1)
//       { // a blank line denotes end of headers
//         skip_headers = false;
//       }
//     }
//     else
//     {
//       int bytesIn;
//       bytesIn = httpclient.read((uint8_t *)&RxBuf[respLen], sizeof(RxBuf) - respLen);
//       //Serial.print(F("bytesIn ")); Serial.println(bytesIn);
//       if (bytesIn > 0)
//       {
//         respLen += bytesIn;
//         if (respLen > sizeof(RxBuf))
//           respLen = sizeof(RxBuf);
//       }
//       else if (bytesIn < 0)
//       {
//         DPRINT(F("read error "));
//         DPRINTLN(bytesIn);
//       }
//     }
//     delay(1);
//   }
//   httpclient.flush();
//   httpclient.stop();

//   if (respLen >= sizeof(RxBuf))
//   {
//     Serial.print(F("RxBuf overflow "));
//     Serial.println(respLen);
//     delay(1000);
//     return;
//   }

//   RxBuf[respLen++] = '\0'; // Terminate the C string
//   DPRINT(F("respLen "));
//   DPRINTLN(respLen);
//   DPRINTLN(RxBuf);
//   if (forecast_type == "forecast")
//   {
//     compileWeather_forecast(RxBuf);
//   }
//   if (forecast_type == "weather")
//   {
//     compileWeather_actual(RxBuf);
//   }
// }

// bool compileWeather_actual(char *json)
// {

//   StaticJsonBuffer<1 * bufferSize> jsonBuffer;

//   char *jsonstart = strchr(json, '{'); // Skip characters until first '{' found
//   DPRINT(F("jsonstart "));
//   DPRINTLN(jsonstart);
//   if (jsonstart == NULL)
//   {
//     Serial.println(F("JSON data missing"));
//     return false;
//   }
//   json = jsonstart;
//   // Parse JSON
//   JsonObject &root = jsonBuffer.parseObject(json);
//   if (!root.success())
//   {
//     Serial.println(F("jsonBuffer.parseObject() failed"));
//     return false;
//   }
//   // Extract weather info from parsed JSON

//   tempa = (const char *)root["main"]["temp"];
//   temp_lowa = (const char *)root["main"]["temp_min"];
//   temp_higha = (const char *)root["main"]["temp_max"];
//   icona = (const char *)root["weather"][0]["icon"];
//   conditionsa = (const char *)root["weather"][0]["description"];
//   pressurea = (const char *)root["main"]["pressure"];
//   humiditya = (const char *)root["main"]["humidity"];

//   jsonBuffer.clear();
//   return true;
// }

// bool compileWeather_forecast(char *json)
// {
//   DynamicJsonBuffer jsonBuffer(bufferSize);

//   char *jsonstart = strchr(json, '{');
//   DPRINT(F("jsonstart "));
//   DPRINTLN(jsonstart);
//   if (jsonstart == NULL)
//   {
//     Serial.println(F("JSON data missing"));
//     return false;
//   }
//   json = jsonstart;

//   // Parse JSON
//   JsonObject &root = jsonBuffer.parseObject(json);
//   if (!root.success())
//   {
//     Serial.println(F("jsonBuffer.parseObject() failed"));
//     return false;
//   }

//   JsonArray &list = root["list"];
//   JsonObject &threehourf = list[1];
//   JsonObject &sixhourf = list[2];

//   // [0] is last forecast close to actual time, [1] is +3h, [2] is +6h
//   // to get weather for +6h and +12h, we need to look into [2] and [4]

//   icon0 = (const char *)threehourf["weather"][0]["icon"];
//   temp0 = (const char *)threehourf["main"]["temp"];
//   humidity0 = (const char *)threehourf["main"]["humidity"];
//   pressure0 = (const char *)threehourf["main"]["pressure"];

//   // get forecast timestamps

//   time_t forecasttime = threehourf["dt"];
//   datetime0 = ctime(&forecasttime);

//   DPRINTLN(F("3h forecast: "));
//   DPRINTLN(icon0 + " " + high0 + " " + low0 + " " + temp0 + " " + humidity0 + " " + pressure0 + " " + datetime0);

//   icon1 = (const char *)sixhourf["weather"][0]["icon"];
//   temp1 = (const char *)sixhourf["main"]["temp"];
//   humidity1 = (const char *)sixhourf["main"]["humidity"];
//   pressure1 = (const char *)sixhourf["main"]["pressure"];

//   // get forecast timestamps
//   forecasttime = sixhourf["dt"];
//   datetime1 = ctime(&forecasttime);

//   DPRINTLN(F("6h forecast: "));
//   DPRINTLN(icon1 + " " + high1 + " " + low1 + " " + temp1 + " " + humidity1 + " " + pressure1 + " " + datetime1);

//   jsonBuffer.clear();
//   return true;
// }

void syncCalendar()
{
  // Fetch Google Calendar events for 1 week ahead

  DPRINT(F("Free heap at sync start .. "));
  DPRINTLN(ESP.getFreeHeap());

  HTTPSRedirect* client = nullptr; // Setup a new HTTPS client

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(dstPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  DPRINT("Connecting to ");
  DPRINTLN(dstHost);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < GoogleServerMaxRetry; i++)
  {
    int retval = client->connect(dstHost, dstPort);
    if (retval == 1)
    {
      flag = true;
      break;
    }
    else
      Serial.println(F("Connection failed. Retrying..."));
  }

  if (!flag)
  {
    Serial.print(F("Could not connect to server: "));
    Serial.println(dstHost);
    Serial.println("Exiting...");
    delete client;
    client = nullptr;
    return;
  }

  DPRINTLN(F("GET: Fetch Google Calendar Data:"));

  // Sometimes fetching data hangs here, causing high battery drain which renders the system useless

  // Trigger HW watchdog on to long delay
  ESP.wdtDisable();

  // fetch spreadsheet data
  client->GET(dstPath, dstHost);
  String googleCalData = client->getResponseBody();

  DPRINT(F("data fetched from google: "));
  DPRINTLN(googleCalData);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(googleCalData);
  if (root.success())
  {
    String status = root["status"];
    if (status == "success")
    {
     DPRINTLN(F("success"));
    }
    else
    {
      DPRINT(F("Unsuccessful response: "));
      DPRINTLN(status);
    }
  }
  else
  {
    DPRINTLN(F("Failed to parse JSON"));
  }

  // delete HTTPSRedirect object
  delete client;
  client = nullptr;
  DPRINT(F("Free heap at sync end .. "));
  DPRINTLN(ESP.getFreeHeap());
}

// get UTC time referenced to 1970 by NTP
time_t getNTP_UTCTime1970()
{
  bNTPStarted = false; // invalidate; time-lib functions crash, if not initalized poperly
  unsigned long t = getNTPTimestamp();
  if (t == 0)
    return (0);

  // scale to 1970
  // may look like back & forth with ntp code; wrote it to make needed conversions more clear
  return (t + 946684800UL);
}

//###########################################################################
