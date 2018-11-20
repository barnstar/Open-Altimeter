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
  ESP8266WebServer *server;

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
