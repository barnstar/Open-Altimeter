#include "WebServer.hpp"
#include "DataLogger.hpp"
#include <FS.h>

WebServer::WebServer()
{
}

 WebServer::~WebServer()
 {
   delete server;  
 }

const char *ssid = "Altimeter1";
const char *password = "password";
int stateLED = LOW;


const String HtmlHtml = "<html><head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" /></head>";
const String HtmlHtmlClose = "</html>";
const String HtmlTitle = "<h1>Rocket Status</h1><br/>\n";

void WebServer::setAddress(const IPAddress& ipAddress)
{
  Serial.println(String("IP Set to:" + ipAddress.toString()));
  this->ipAddress = ipAddress;
  
  server = new ESP8266WebServer(80);
  
  WiFi.softAP(ssid, password);
  
  IPAddress apip = WiFi.softAPIP();
  Serial.print("visit: \n");
  Serial.println(apip);
  
  server->on("/", std::bind(&WebServer::handleRoot, this));
  server->on("/settings", std::bind(&WebServer::handleSettings, this));
  server->serveStatic("/stats", SPIFFS, "/stats.html");
  server->begin();
  Serial.println("HTTP server initialized");
}

void WebServer::handleClient() {
  server->handleClient();
}

void WebServer::handleSettings()
{
  //getArgs and update settings
  //Send settings screen.
}

static String *input;
void concatenateStrings(const String &s)
{
   *input += s;
}

void WebServer::handleRoot() {
  String htmlRes = HtmlHtml + HtmlTitle;
  input = &htmlRes;
  DataLogger::sharedLogger().readFlightData(concatenateStrings);
  htmlRes += HtmlHtmlClose;
  server->send(200, "text/html", htmlRes);
}



