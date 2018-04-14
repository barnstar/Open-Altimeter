#include "WebServer.hpp"
#include "DataLogger.hpp"
#include "FlightController.hpp"
#include <FS.h>


WebServer::WebServer()
{
    server = new ESP8266WebServer(80);
}

 WebServer::~WebServer()
 {
   delete server;  
 }

const char *ssid = "Altimeter1";
const char *password = "password";
int stateLED = LOW;


const String HtmlHtml       = "<html><head>"
                               "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" /></head>";
const String HtmlHtmlClose  = "</html>";
const String HtmlTitle      = "<h1>Alitmeter 1</h1><br/>\n";

const String flightsLink      = "<br><a href=\"/flights\">Flights</a>";
const String resetLink      = "<br><a href=\"/reset\">Reset Controller</a>";
const String statusLink     = "<br><a href=\"/status\">Controller Status</a>";
const String settingsLink   = "<br><a href=\"/settings\">Controller Settings</a>";
const String testLink       = "<br><a href=\"/test\">Run Test</a>";
const String resetAllLink   = "<br><a href=\"/resetAll\">Reset ALL</a>";

void WebServer::setAddress(const IPAddress& ipAddress)
{ 
  Serial.println(String("IP Set to:" + ipAddress.toString()));
  this->ipAddress = ipAddress; 
  WiFi.softAP(ssid, password);
  
  IPAddress apip = WiFi.softAPIP();
  Serial.print("Server Address: ");
  Serial.println(apip);
  
  server->on("/", std::bind(&WebServer::handleRoot, this));
  server->on("/settings", std::bind(&WebServer::handleSettings, this));
  server->on("/reset", std::bind(&WebServer::handleReset, this));
  server->on("/status", std::bind(&WebServer::handleStatus, this));
  server->on("/test", std::bind(&WebServer::handleTest, this));
  server->on("/flights", std::bind(&WebServer::handleFlights, this));
  server->on("/resetAll", std::bind(&WebServer::handleResetAll, this));

  server->serveStatic("/stats", SPIFFS, "/stats.html");
  bindSavedFlights();
  server->begin();
  Serial.println("HTTP server initialized");
}

void WebServer::bindSavedFlights()
{
  Dir dir = SPIFFS.openDir("/flights");
  while (dir.next()) {
     server->serveStatic(dir.fileName().c_str(), SPIFFS, dir.fileName().c_str());
  }
}

String WebServer::savedFlightLinks()
{
  String ret;
  Dir dir = SPIFFS.openDir("/flights");
  while (dir.next()) {
    ret += "<a href=\"" + dir.fileName() + "\">"+dir.fileName()+"</a><br>";
  }
  return ret;
}

void WebServer::handleClient() {
  server->handleClient();
}

void WebServer::handleFlights()
{
    String htmlRes = HtmlHtml + HtmlTitle;
    htmlRes += savedFlightLinks();
    htmlRes+="<br><br>";
    htmlRes += HtmlHtmlClose;
   server->send(200, "text/html", htmlRes);
}

void WebServer::handleSettings()
{
    server->send(200, "text/html", "TODO");
}

static String *input;
void concatenateStrings(const String &s)
{
   *input += s + "<br/>";
}

void WebServer::handleResetAll()
{
   FlightController::shared().resetAll();
   handleStatus();
}

void WebServer::handleReset() 
{
  FlightController::shared().reset();
  handleStatus();
}

void WebServer::handleTest() 
{
  FlightController::shared().reset();
  handleStatus();
  FlightController::shared().runTest();
}

void WebServer::handleStatus()
{
  String htmlRes = HtmlHtml + HtmlTitle;
  htmlRes += "STATUS<br>";
  htmlRes += FlightController::shared().getStatus();
  htmlRes += HtmlHtmlClose;
  server->send(200, "text/html", htmlRes);
}

void WebServer::handleRoot() 
{
  String htmlRes = HtmlHtml + HtmlTitle;

  htmlRes += flightsLink;
  htmlRes += resetLink;
  htmlRes += statusLink;
  htmlRes += settingsLink;
  htmlRes += testLink;
  htmlRes += "<br><br>";
  htmlRes += resetAllLink;

  htmlRes += "<br><br>";

  htmlRes += "STATUS<br>";
  htmlRes += FlightController::shared().getStatus();
  htmlRes += "<br><br>";

  input = &htmlRes;
  DataLogger::sharedLogger().readFlightData(concatenateStrings);
  htmlRes+="<br><br>";
  htmlRes += HtmlHtmlClose;
  server->send(200, "text/html", htmlRes);
}



