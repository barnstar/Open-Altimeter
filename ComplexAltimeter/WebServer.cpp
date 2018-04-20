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


void WebServer::setAddress(const IPAddress& ipAddress)
{
  Serial.println(String("IP Set to:" + ipAddress.toString()));
  this->ipAddress = ipAddress;
  WiFi.softAP(ssid, password);

  IPAddress apip = WiFi.softAPIP();

  Serial.print("Server Address: ");
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
  //server->serveStatic(fname.c_str(), SPIFFS,fname.c_str());
}

void WebServer::bindSavedFlights()
{
  Dir dir = SPIFFS.openDir(FLIGHTS_DIR);
  while (dir.next()) {
     server->on(dir.fileName().c_str(), std::bind(&WebServer::handleFlight, this));
     //server->serveStatic(dir.fileName().c_str(), SPIFFS, dir.fileName().c_str());
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
  pageBuilder.appendToBody(savedFlightLinks());
  server->send(200, "text/html", pageBuilder.build());
}

void WebServer::handleFlight()
{
  String path = server->uri();
  DataLogger::log("Reading " + path);
  
  pageBuilder.reset("Flight :" + path + "<br>\n");
  pageBuilder.startPageStream(server);
  pageBuilder.sendHeaders(server);
  pageBuilder.sendBodyChunk(server, "", true, false);
  File f = SPIFFS.open(path, "r");
  if(f) {
    f.seek(0);
    while(f.available()) {
      String line = f.readStringUntil('\n');
      pageBuilder.sendBodyChunk(server, line + "<br/>", false, false);
    }
    f.close();
  }
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
  pageBuilder.appendToBody(FlightController::shared().getStatus());

  SensorData d;
  FlightController::shared().readSensorData(&d);
  pageBuilder.appendToBody(d.toString());

  server->send(200, "text/html", pageBuilder.build());
  pageBuilder.reset("");
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
  pageBuilder.appendToBody( PageBuilder::makeLink(String(settingsURL), "Configure<br/>") );
  pageBuilder.appendToBody( PageBuilder::makeLink(String(flightsURL), "Flight List<br/>") );
  pageBuilder.appendToBody( PageBuilder::makeLink(String(resetURL), "Set To Ready State<br/>") );
  pageBuilder.appendToBody( PageBuilder::makeLink(String(statusURL), "Show Status<br/>") );
  pageBuilder.appendToBody( PageBuilder::makeLink(String(testURL), "Run Flight Test<br/>") );
  pageBuilder.appendToBody( doubleLine + PageBuilder::makeLink(String(resetAllURL), "Full Reset") + doubleLine );
  pageBuilder.appendToBody(  "STATUS : <br>");
  pageBuilder.appendToBody( FlightController::shared().getStatus());
  pageBuilder.appendToBody( FlightController::shared().checkMPUSettings());
  pageBuilder.appendToBody( doubleLine);

  String flightData;
  input = &flightData;
  DataLogger::sharedLogger().readFlightData(concatenateStrings);
  pageBuilder.appendToBody(flightData);
  pageBuilder.appendToBody(doubleLine);
  server->send(200, "text/html", pageBuilder.build());
  pageBuilder.reset("");
}


///////////////////////////////////////////////////////////////////////////////////////
// PageBuilder

PageBuilder::PageBuilder()
{
  reset("");
}

PageBuilder::~PageBuilder() {}

//Strings can't conatin the entire body so we need some kind of streaming

void PageBuilder::startPageStream(ESP8266WebServer *s)
{
  s->setContentLength(CONTENT_LENGTH_UNKNOWN);   //Enable Chunked Transfer
}

void PageBuilder::sendHeaders(ESP8266WebServer *s)
{
  String header = String("<h1>" + title + "</h1><br/>\n");
  s->send(200, "text/html", header);
}

void PageBuilder::sendBodyChunk(ESP8266WebServer *s, const String &chunk, bool addStartTag, bool addClosingTag)
{
  String toSend = addStartTag ? "<body>" + chunk : chunk;
  toSend = addClosingTag ? toSend + "</body>" : toSend;
  sendRawText(s, toSend);
}


void PageBuilder::sendScript(ESP8266WebServer *s, const String &script)
{
  String toSend = String("<script>") + script + String("</script>");
  sendRawText(s, toSend);
}

void PageBuilder::sendRawText(ESP8266WebServer *s, const String &rawText)
{
 s->sendContent(rawText); 
}


void PageBuilder::closePageStream(ESP8266WebServer *s)
{
   s->sendContent(""); 
   s->client().stop();
}


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


