#include "WebServer.hpp"
#include "DataLogger.hpp"
#include "FlightController.hpp"
#include <FS.h>


const char *ssid     = "Altimeter1";
const char *password = "password";

const String HtmlHtml       = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" /></head>";
const String HtmlHtmlClose  = "</html>";
const String HtmlTitle      = "<h1>Alitmeter 1</h1><br/>\n";
const String doubleLine     = "<br/><br/>";

const char* testURL     = "/test";
const char* flightsURL  = "/flights";
const char* resetAllURL = "/resetAll";
const char* resetURL    = "/reset";
const char* statusURL   = "/status";
const char* settingsURL = "/settings";
const char* configURL   = "/config";


WebServer::WebServer()
{
    server = new ESP8266WebServer(80);
}

 WebServer::~WebServer()
 {
   delete server;
 }

void WebServer::handleClient()
{
  server->handleClient();
}


void WebServer::start(const IPAddress& ipAddress)
{
  Serial.println(String("IP Set to:" + ipAddress.toString()));
  this->ipAddress = ipAddress;
  WiFi.softAP(ssid, password);

  IPAddress apip = WiFi.softAPIP();

  Serial.print("Actual Server Address: ");
  Serial.println(apip);

  server->on("/", std::bind(&WebServer::handleRoot, this));
  server->on(resetURL, std::bind(&WebServer::handleReset, this));
  server->on(statusURL, std::bind(&WebServer::handleStatus, this));
  server->on(testURL, std::bind(&WebServer::handleTest, this));
  server->on(flightsURL, std::bind(&WebServer::handleFlights, this));
  server->on(resetAllURL, std::bind(&WebServer::handleResetAll, this));
  server->on(configURL, std::bind(&WebServer::handleConfig, this));
  server->serveStatic(settingsURL, SPIFFS,"/settings.html");

  bindSavedFlights();
  server->begin();
  Serial.println("HTTP server initialized");
}

void WebServer::bindFlight(int index)
{
  String fname = String(FLIGHTS_DIR) + String("/") + String(index);
  server->on( fname.c_str(), std::bind(&WebServer::handleFlight, this)) ;
}

void WebServer::bindSavedFlights()
{
  Dir dir = SPIFFS.openDir(FLIGHTS_DIR);
  while (dir.next()) {
     server->on(dir.fileName().c_str(), std::bind(&WebServer::handleFlight, this));
  }
}

String WebServer::savedFlightLinks()
{
  String ret;
  Dir dir = SPIFFS.openDir(FLIGHTS_DIR);
  while (dir.next()) {
    ret += "<a href=\"" + dir.fileName() + "\">"+dir.fileName()+"</a><br>";
  }
  return ret;
}

void WebServer::handleFlights()
{
  pageBuilder.reset("Saved Flights");
  pageBuilder.startPageStream(server);
  pageBuilder.sendHeaders(server);
  pageBuilder.sendTagedChunk(s, "body", savedFlightLinks() );
  pageBuilder.closePageStream();
}

void WebServer::handleFlight()
{
  String path = server->uri();
  DataLogger::log("Reading " + path);

  pageBuilder.reset("Flight :" + path + "<br>\n");
  pageBuilder.startPageStream(server);
  pageBuilder.sendHeaders(server);
  pageBuilder.sendBodyChunk(server, "", true, false);
  pageBuilder.sendFilePretty(path);
  pageBuilder.sendBodyChunk(server, "", false, true);
  pageBuilder.closePageStream(server);
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
  pageBuilder.startPageStream(server);
  pageBuilder.sendHeaders(server);

  String body = FlightController::shared().getStatus();

  SensorData d;
  FlightController::shared().readSensorData(&d);
  body += d.toString();

  pageBuilder.sendTaggedChunk(s, String("body"), body);
  pageBuilder.closePageStream();
}

void WebServer::handleConfig()
{
  for (int i = 0; i < server->args(); i++) {
    String argName  = server->argName(i);
    String argVal  = server->arg(i);
    handleConfigSetting(argName, argVal);
  }
  handleStatus();
}

void WebServer::handleConfigSetting(String &arg, String &val)
{
  if(arg == String("deplAlt")) {
    int deplAlt = val.toInt();
    FlightController::shared().setDeploymentAltitude(deplAlt);
  }
  //Add other form elements here....
}


void WebServer::handleRoot()
{
  pageBuilder.reset("Altimeter 1");
  pageBuilder.startPageStream(server);
  pageBuilder.sendHeaders(server);
  pageBuilder.sendBodyChunk(server, "", true, false);

  String body;

  body += PageBuilder::makeLink(String(settingsURL), "Configure<br/>");
  body += PageBuilder::makeLink(String(flightsURL), "Flight List<br/>");
  body += PageBuilder::makeLink(String(resetURL), "Set To Ready State<br/>");
  body += PageBuilder::makeLink(String(statusURL), "Show Status<br/>");
  body += PageBuilder::makeLink(String(testURL), "Run Flight Test<br/>");
  body += doubleLine + PageBuilder::makeLink(String(resetAllURL), "Full Reset") + doubleLine );
  body += "STATUS : <br>";
  body += FlightController::shared().getStatus();
  body += FlightController::shared().checkMPUSettings();
  body += doubleLine;

  pageBuilder.sendBodyChunk(server, body, false, false);
  pageBuilder.sendFilePretty("/flights.txt");
  pageBuilder.closePageStream(server);
}


///////////////////////////////////////////////////////////////////////////////////////
// PageBuilder


void PageBuilder::startPageStream(ESP8266WebServer *s)
{
  server = s;
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);   //Enable Chunked Transfer
}

void PageBuilder::sendFileRaw(const String &path)
{
  const size_t buffer_size = 256;
  File f = SPIFFS.open(path, "r");
  char buffer[buffer_size];

  if(f) {
    while(f.available()) {
      size_t read = f.readBytes(buffer, buffer_size-1);
      buffer[read+1] = 0;
      sendRawText(String(buffer));
    }
    f.close();
  }
}

void PageBuilder::sendFilePretty(const String &path)
{
  File f = SPIFFS.open(path, "r");
  if(f) {
    while(f.available()) {
      String line = f.readStringUntil('\n');
      sendRawText(line + "<br/>", false, false);
    }
    f.close();
  }
}

void PageBuilder::sendHeaders()
{
  String header = HtmlHtml + String("\n<h1>" + title + "</h1><br/>\n<");
  server->send(200, "text/html", header);
}


void PageBuilder::sendTagedChunk(const String &tag, const String &chunk)
{
  String toSend = String("<" + tag ">" + chunk + "</" + tag + ">\n");
  sendRawText(server, toSend);
}

void PageBuilder::sendBodyChunk(const String &chunk, bool addStartTag, bool addClosingTag)
{
  String toSend = addStartTag ? "<body>" + chunk : chunk;
  toSend = addClosingTag ? toSend + "</body>" : toSend;
  sendRawText(server, toSend);
}


void PageBuilder::sendScript(const String &script)
{
  sendTagedChunk(server, String("script"), script);
}

void PageBuilder::sendRawText(const String &rawText)
{
 server->sendContent(rawText);
}


void PageBuilder::closePageStream()
{
  sendRawText(s, HtmlHtmlClose);
  server->sendContent("");
  server->client().stop();
  server = nullptr;
}

String PageBuilder::makeLink(const String &link, const String &string)
{
  return "<a href=\"" + link + "\">" + string + "</a>\n";
}


String PageBuilder::makeDiv(const String &name, const String &contents)
{
    return "<div name=\"" + name + "\">\n" + contents + "\n</div>\n";
}


