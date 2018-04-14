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

const char* testURL = "/test";
const char* flightsURL = "/flighs";
const char* resetAllURL = "/resetAll";
const char* resetURL = "/reset";
const char* statusURL = "/status";
const char* settingsURL = "/settings";

void WebServer::setAddress(const IPAddress& ipAddress)
{ 
  Serial.println(String("IP Set to:" + ipAddress.toString()));
  this->ipAddress = ipAddress; 
  WiFi.softAP(ssid, password);
  
  IPAddress apip = WiFi.softAPIP();
  Serial.print("Server Address: ");
  Serial.println(apip);
  
  server->on("/", std::bind(&WebServer::handleRoot, this));
  server->on(settingsURL, std::bind(&WebServer::handleSettings, this));
  server->on(resetURL, std::bind(&WebServer::handleReset, this));
  server->on(statusURL, std::bind(&WebServer::handleStatus, this));
  server->on(testURL, std::bind(&WebServer::handleTest, this));
  server->on(flightsURL, std::bind(&WebServer::handleFlights, this));
  server->on(resetAllURL, std::bind(&WebServer::handleResetAll, this));

  bindSavedFlights();
  server->begin();
  Serial.println("HTTP server initialized");
}

void WebServer::bindFlight(int index) 
{
  String fname = "/flights/" + String(index);
  //server->on( fname.c_str(), std::bind(&WebServer::handleFlight, this)) ;
  server->serveStatic(fname.c_str(), SPIFFS,fname.c_str());
}

void WebServer::bindSavedFlights()
{
  Dir dir = SPIFFS.openDir("/flights");
  while (dir.next()) {
     //server->on(dir.fileName().c_str(), std::bind(&WebServer::handleFlight, this));
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
  pageBuilder.reset("Saved Flights");
  pageBuilder.appendToBody(savedFlightLinks());
  server->send(200, "text/html", pageBuilder.build());
}

void WebServer::handleFlight()
{
  String path = server->uri();
  DataLogger::log("Reading flight " + path);
  pageBuilder.reset("Flight :" + path);
  pageBuilder.appendToBody("Here's the flight JSON Raw:<br><br>\n");
  File f = SPIFFS.open(path, "r");
  if(f) {
    f.seek(0);
    while(f.available()) {
      String line = f.readStringUntil('\n');
      pageBuilder.appendToBody(line + "<br/>");
    }
    f.close();
  }
  server->send(200, "text/html", pageBuilder.build());
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
  pageBuilder.reset("Altimeter 1: Status");
  pageBuilder.appendToBody(FlightController::shared().getStatus());
  server->send(200, "text/html", pageBuilder.build());
  pageBuilder.reset("");
}

void WebServer::handleRoot() 
{
  pageBuilder.reset("Altimeter 1");
  pageBuilder.appendToBody( PageBuilder::makeLink(String(flightsURL), "Flight List<br/>") );
  pageBuilder.appendToBody( PageBuilder::makeLink(String(resetURL), "Set To Ready State<br/>") );
  pageBuilder.appendToBody( PageBuilder::makeLink(String(statusURL), "Show Status<br/>") );
  pageBuilder.appendToBody( PageBuilder::makeLink(String(testURL), "Run Flight Test<br/>") );
  pageBuilder.appendToBody("<br><br>" + PageBuilder::makeLink(String(resetAllURL), "Full Reset") + "<br><br>" );
  pageBuilder.appendToBody("STATUS : <br>");
  pageBuilder.appendToBody(FlightController::shared().getStatus());
  pageBuilder.appendToBody("<br><br>"); 

  String flightData;
  input = &flightData;
  DataLogger::sharedLogger().readFlightData(concatenateStrings);
  pageBuilder.appendToBody(flightData);
  pageBuilder.appendToBody("<br><br>"); 
  server->send(200, "text/html", pageBuilder.build());
  pageBuilder.reset("");
}


PageBuilder::PageBuilder()
{
  reset("");  
}

PageBuilder::~PageBuilder() {}


String PageBuilder::build()
{
  String htmlRes = "" + HtmlHtml;
  
  htmlRes += "<h1>" + title + "</h1><br/>\n<body>";
  htmlRes += body + "\n</body>";
  htmlRes += script;
  htmlRes += HtmlHtmlClose;

  return htmlRes;
}


void PageBuilder::appendToBody(const String &html)
{
  body += html;
}


void PageBuilder::appendScriptLink(const String &link)
{
  String linkText = "<script src=\"" + link + "\"></script>";
  script += linkText;
}


void PageBuilder::appendScript(const String &scriptTxt)
{
  script += "<script>\n"+scriptTxt+"\n</script>";  
}


String PageBuilder::makeLink(const String &link, const String &string)
{
  return "<a href=\"" + link + "\">" + string + "</a>\n";
}

String PageBuilder::makeDiv(const String &name, const String &contents)
{
    return "<div name=\"" + name + "\">\n" + contents + "\n</div>\n";
}


