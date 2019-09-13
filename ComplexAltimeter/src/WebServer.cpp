/*********************************************************************************
 * Open Altimeter
 *
 * Mid power rocket avionics software for altitude recording and dual deployment
 *
 * Copyright 2018, Jonathan Nobels
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **********************************************************************************/

#include "WebServer.hpp"
#include <FS.h>
#include "DataLogger.hpp"
#include "FlightController.hpp"

#define RUN_AS_ACCESS_POINT 0

#if RUN_AS_ACCESS_POINT
const char *ssid     = "Altimeter1";
const char *password = "password";
#else
const char *ssid     = "CSIS Surveillance Van #3";
const char *password = "978Descanso";
#endif

const String HtmlHtml =
    "<html><head><meta name=\"viewport\" content=\"width=device-width, "
    "initial-scale=1\" /></head>";
const String HtmlHtmlClose = "</html>";
const String HtmlTitle     = "<h1>Open Altimeter</h1><br/>\n";
const String doubleLine    = "<br/><br/>";

const char *testURL     = "/test";
const char *flightsURL  = "/flights";
const char *resetAllURL = "/resetAll";
const char *resetURL    = "/reset";
const char *disarmURL   = "/disarm";
const char *statusURL   = "/status";
const char *settingsURL = "/settings";
const char *configURL   = "/config";

WebServer::WebServer() : server(80) {}

WebServer::~WebServer() {}

void WebServer::handleClient() { server.handleClient(); }

void WebServer::start(const IPAddress &ipAddress)
{
#if RUN_AS_ACCESS_POINT
  Serial.println(String("IP Set to:" + ipAddress.toString()));
  this->ipAddress = ipAddress;
  WiFi.softAP(ssid, password);
  IPAddress apip = WiFi.softAPIP();
  Serial.print("Actual Server Address: ");
  Serial.println(apip);
#else
  WiFi.begin(ssid, password);
#endif

  server.on("/", std::bind(&WebServer::handleRoot, this));
  server.on(resetURL, std::bind(&WebServer::handleReset, this));
  server.on(disarmURL, std::bind(&WebServer::handleDisarm, this));
  server.on(statusURL, std::bind(&WebServer::handleStatus, this));
  server.on(testURL, std::bind(&WebServer::handleTest, this));
  server.on(flightsURL, std::bind(&WebServer::handleFlights, this));
  server.on(resetAllURL, std::bind(&WebServer::handleResetAll, this));
  server.on(configURL, std::bind(&WebServer::handleConfig, this));
  server.serveStatic(settingsURL, SPIFFS, "/settings.html");

  bindSavedFlights();
  server.begin();
  Serial.println("HTTP server initialized");
}

String WebServer::getIPAddress() { return WiFi.localIP().toString(); }

void WebServer::bindFlight(int index)
{
  String fname = String(FLIGHTS_DIR) + String("/") + String(index);
  server.on(fname.c_str(), std::bind(&WebServer::handleFlight, this));
}

void WebServer::bindSavedFlights()
{
  Dir dir = SPIFFS.openDir(FLIGHTS_DIR);
  while (dir.next()) {
    server.on(dir.fileName().c_str(),
              std::bind(&WebServer::handleFlight, this));
  }
}

String WebServer::savedFlightLinks()
{
  String ret;
  Dir dir = SPIFFS.openDir(FLIGHTS_DIR);
  while (dir.next()) {
    ret += "<h2><a href=\"" + dir.fileName() + "\">" + dir.fileName() +
           "</a></h2><br>";
  }
  return ret;
}

void WebServer::handleFlights()
{
  pageBuilder.startPageStream(&server, "Saved Flights");
  pageBuilder.sendHeaders();
  pageBuilder.sendTaggedChunk("body", savedFlightLinks());
  pageBuilder.closePageStream();
}

void WebServer::handleFlight()
{
  String path = server.uri();
  DataLogger::log("Reading " + path);

  pageBuilder.startPageStream(&server, "");
  pageBuilder.sendRawText(HtmlHtml);
  // Send the flight data as a JSON object
  pageBuilder.sendRawText("<script>");
  pageBuilder.sendFileRaw(path);
  pageBuilder.sendRawText("</script>");
  // Send the graphing fragment
  pageBuilder.sendFileRaw("/graph.html");
  pageBuilder.closePageStream();
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

void WebServer::handleDisarm()
{
  FlightController::shared().stop();
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
  pageBuilder.startPageStream(&server, "Open Altimeter Status");
  pageBuilder.sendHeaders();

  String body = FlightController::shared().getStatus();

  SensorData d;
  FlightController::shared().readSensorData(&d);
  body += d.toString();

  pageBuilder.sendTaggedChunk(String("body"), body);
  pageBuilder.closePageStream();
}

void WebServer::handleConfig()
{
  for (int i = 0; i < server.args(); i++) {
    String argName = server.argName(i);
    String argVal  = server.arg(i);
    handleConfigSetting(argName, argVal);
  }
  handleStatus();
}

void WebServer::handleConfigSetting(String &arg, String &val)
{
  if (arg == String("deplAlt")) {
    int deplAlt = val.toInt();
    FlightController::shared().setDeploymentAltitude(deplAlt);
  } else if (arg == String("testChannel")) {
    uint channel = val.toInt();
    if (channel < 1 || channel > 4) {
      DataLogger::log(F("Invalid channel number"));
      return;
    };
    RecoveryDevice *d = FlightController::shared().getRecoveryDevice(channel);
    if (d->deployed) {
      d->disable();
    } else {
      d->enable();
    }
  } else if (arg == String("onAngle")) {
    uint angle = val.toInt();
    RecoveryDevice::setOnAngle(angle, true);
  } else if (arg == String("offAngle")) {
    uint angle = val.toInt();
    RecoveryDevice::setOffAngle(angle, true);
  }
}

void WebServer::handleRoot()
{
  pageBuilder.startPageStream(&server, "Altimeter 0");
  pageBuilder.sendHeaders();
  pageBuilder.sendBodyChunk("", true, false);

  String body;

  body += PageBuilder::makeLink(String(settingsURL), "Configure<br/>");
  body += PageBuilder::makeLink(String(flightsURL), "Flight List<br/>");
  body += PageBuilder::makeLink(String(statusURL), "Show Status<br/>");
  body += PageBuilder::makeLink(String(testURL), "Run Flight Test<br/>");

  body += doubleLine + PageBuilder::makeLink(String(resetURL), "Arm<br/>");
  body += PageBuilder::makeLink(String(disarmURL), "Disarm<br/>");

  body += doubleLine +
          PageBuilder::makeLink(String(resetAllURL), "Full Reset") + doubleLine;
  body += "STATUS : <br>";
  body += FlightController::shared().getStatus();
  body += doubleLine;

  pageBuilder.sendBodyChunk(body, false, false);
  pageBuilder.sendFilePretty("/flights.txt");
  pageBuilder.closePageStream();
}

///////////////////////////////////////////////////////////////////////////////////////
// PageBuilder

void PageBuilder::startPageStream(ESP8266WebServer *s, const String &title)
{
  this->title = title;
  server      = s;
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);  // Enable Chunked Transfer
}

void PageBuilder::sendFileRaw(const String &path)
{
  File f = SPIFFS.open(path, "r");
  if (f) {
    while (f.available()) {
      String line = f.readStringUntil('\n');
      sendRawText(line);
    }
    f.close();
  }
}

void PageBuilder::sendFilePretty(const String &path)
{
  File f = SPIFFS.open(path, "r");
  if (f) {
    while (f.available()) {
      String line = f.readStringUntil('\n');
      sendRawText(line + "<br/>");
    }
    f.close();
  }
}

void PageBuilder::sendHeaders()
{
  String header = HtmlHtml + String("\n<h1>" + title + "</h1><br/>\n");
  server->send(200, "text/html", header);
}

void PageBuilder::sendTaggedChunk(const String &tag, const String &chunk)
{
  String toSend = String("<" + tag + ">" + chunk + "</" + tag + ">\n");
  sendRawText(toSend);
}

void PageBuilder::sendBodyChunk(const String &chunk, bool addStartTag,
                                bool addClosingTag)
{
  String toSend = addStartTag ? "<body>" + chunk : chunk;
  toSend        = addClosingTag ? toSend + "</body>" : toSend;
  sendRawText(toSend);
}

void PageBuilder::sendScript(const String &script)
{
  sendTaggedChunk(String("script"), script);
}

void PageBuilder::sendRawText(const String &rawText)
{
  server->sendContent(rawText);
}

void PageBuilder::closePageStream()
{
  sendRawText(HtmlHtmlClose);
  server->sendContent("");
  server->client().stop();
  server = nullptr;
}

String PageBuilder::makeLink(const String &link, const String &string)
{
  return "<h2><a href=\"" + link + "\">" + string + "</a></h2>\n";
}

String PageBuilder::makeDiv(const String &name, const String &contents)
{
  return "<div name=\"" + name + "\">\n" + contents + "\n</div>\n";
}
