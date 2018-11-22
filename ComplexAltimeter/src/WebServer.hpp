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


#ifndef webserver_h
#define webserver_h

#include <Arduino.h>

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

class WebServer;

class PageBuilder
{
 public:
  PageBuilder(){};
  ~PageBuilder(){};

  String title;

  void startPageStream(ESP8266WebServer *s, const String &title);
  void sendHeaders();
  void sendTaggedChunk(const String &tag, const String &chunk);
  void sendBodyChunk(const String &chunk, bool addStartTag, bool addClosingTag);
  void sendScript(const String &script);
  void sendRawText(const String &rawText);

  void sendFileRaw(const String &path);
  void sendFilePretty(const String &path);

  void closePageStream();

  static String makeLink(const String &link, const String &string);
  static String makeDiv(const String &name, const String &contents);

 private:
  ESP8266WebServer *server = nullptr;
};

class WebServer
{
 public:
  WebServer();
  ~WebServer();

  void start(const IPAddress &ipAddress);
  void handleClient();

  void bindFlight(int index);

 private:
  IPAddress ipAddress;
  ESP8266WebServer server;

  PageBuilder pageBuilder;
  void bindSavedFlights();
  String savedFlightLinks();
  void response();

  void handleRoot();
  void handleStatus();
  void handleFlights();
  void handleFlight();
  void handleConfig();

  void handleReset();
  void handleResetAll();
  void handleConfigSetting(String &arg, String &val);

  void handleTest();
};

#endif  // webserver_h
