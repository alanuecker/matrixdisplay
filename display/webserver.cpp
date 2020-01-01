#include <stdlib.h>

#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WebSocketsServer.h>

#include <ArduinoJson.h>

#include "display.h"

// setup the webserver and websocket server
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
// object to store the data from the websocket connection
StaticJsonDocument<100> data;

// setup the spiffs memory handling
void startSPIFFS()
{
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())
    {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
    }
  }
}

// handle the websocket event
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
  {
    Serial.printf("[%u] Disconnected!\n", num);

    // switch back to clock mode
    clockMode = true;
  }
  break;
  case WStype_CONNECTED:
  {
    Serial.printf("Connected");

    // enable draw mode
    clockMode = false;
    display.clearDisplay();
  }
  break;
  case WStype_TEXT:
    Serial.printf("[%u] get Text: %s\n", num, payload);
    // check the payload type
    if (payload[0] == '{')
    {
      // read the json string
      DeserializationError error = deserializeJson(data, payload);
      if (error)
        break;
      // draw the changed pixel
      display.drawPixel(
          data["x"],
          data["y"],
          display.color565(data["r"], data["g"], data["b"]));
    }
    break;
  }
}

// start the websocket server
void startWebSocket()
{
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

// determin the type of a file in spiffs memory
String getContentType(String filename)
{
  if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".html.gz"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".css.gz"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".js.gz"))
    return "application/javascript";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  return "text/plain";
}

// read a file from spiffs memory and send it to the client
bool handleFileRead(String path)
{
  Serial.println("handleFileRead: " + path);
  // map a file to the given route
  if (path.endsWith("/"))
    path += "index.html";

  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    // open the file
    File file = SPIFFS.open(path, "r");
    // provide the file to the client
    size_t sent = server.streamFile(file, getContentType(path));
    // close the file
    file.close();
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;
}

// provide a 404 page if no file was found for the route
void handleNotFound()
{
  if (!handleFileRead(server.uri()))
  {
    server.send(404, "text/plain", "404: File Not Found");
  }
}

// setup the server
void startServer()
{
  server.onNotFound(handleNotFound);
  server.begin();
}

// update the server and websocket server
void loopWebserver()
{
  webSocket.loop();
  server.handleClient();
}

// setup all webserver related tasks
void setupWebserver()
{
  startSPIFFS();
  startWebSocket();
  startServer();
}