#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "OpenWeatherMap.h"

//------------------------------------------ Universal weather request parcer for opewWeatherMap site ---------
void OWMrequest::doUpdate(String url, byte maxForecasts) {
  JsonStreamingParser parser;
  parser.setListener(this);

  HTTPClient http;

  http.begin(url);
  bool isBody = false;
  char c;
  int size;
  int httpCode = http.GET();
  if(httpCode > 0) {
    WiFiClient * client = http.getStreamPtr();
    while(client->connected()) {
      while((size = client->available()) > 0) {
        c = client->read();
        if (c == '{' || c == '[') {
          isBody = true;
        }
        if (isBody) {
          parser.parse(c);
        }
      }
    }
  }
}

void OWMrequest::init(void) {
  for (byte i = 0; i < OWM_max_layers; ++i)
    p_key[i] = "";
  currentParent = currentKey = "";
}

void OWMrequest::startObject() {
  for (byte i = OWM_max_layers-1; i > 0; --i) {
    p_key[i] = p_key[i-1];
  }
  p_key[0] = currentParent;
  currentParent = currentKey;
}

void OWMrequest::endObject() {
  currentParent = p_key[0];
  for (byte i = 0; i < OWM_max_layers-1; ++i) {
    p_key[i] = p_key[i+1];
  }
  currentKey = "";
}

//------------------------------------------ Current weather conditions from openweatrhermap.org --------------
void OWMconditions::updateConditions(OWM_conditions *conditions, String apiKey, String country, String city, String units, String language) {
  this->conditions = conditions;
  OWMrequest::init();

  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + country;
  if (language != "") url += "&lang="  + language;
  if (units != "")    url += "&units=" + units;
  url +=  + "&appid=" + apiKey;
  doUpdate(url);
  this->conditions = nullptr;
}

void OWMconditions::value(String value) {
  if (currentKey == "description") {
    conditions->description = value;
  } else
  if (currentKey == "icon") {
    conditions->icon = value;
  } else
  if (currentKey == "temp") {
    conditions->temp = value;
  }
}

//------------------------------------------ Five day forecast from openweatrhermap.org -----------------------
byte OWMfiveForecast::updateForecast(OWM_fiveForecast *forecasts, byte maxForecasts, String apiKey, String country, String city, String units, String language) {
  this->forecasts = forecasts;
  this->max_forecasts = maxForecasts;
  OWMrequest::init();
  index = 0;
  timestamp = 0;

  String url = "http://api.openweathermap.org/data/2.5/forecast?q=" + city + "," + country;
  if (language != "") url += "&lang="  + language;
  if (units != "")    url += "&units=" + units;
  url +=  + "&appid=" + apiKey;
  doUpdate(url, maxForecasts);

  this->forecasts = nullptr;
  return index;
}

void OWMfiveForecast::value(String value) {
  if (currentKey == "dt") {
    if (timestamp == 0) {
      index = 0;
      forecasts[index].dt = value;
      timestamp = value.toInt();
    } else {
      uint32_t t = value.toInt();
      if (t > timestamp) {                          // new forecast time
	      if (index < max_forecasts - 1) {
    	    ++index;
          timestamp = t;
    	    forecasts[index].dt = value;
    	  }
      }
    }
  } else
  if (currentKey == "temp") {
    forecasts[index].temp = value;
  } else
  if (currentParent == "weather" && currentKey == "description") {
    forecasts[index].description = value;
  }
}
