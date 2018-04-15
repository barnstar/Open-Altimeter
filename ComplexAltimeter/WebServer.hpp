#ifndef webserver_h
#define webserver_h

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


class PageBuilder
{
  public:
  PageBuilder();
  ~PageBuilder();

  String build();
  void reset(const String &title) {
    this->title = title;
    this->body = " ";
    this->script = " ";
  };

  void appendToBody(const String &html);
  void appendScriptLink(const String &link);
  void appendScript(const String &script);

  static String makeLink(const String &link, const String &string);
  static String makeDiv(const String &name, const String &contents);

  private:
  String title;
  String body;
  String script;
};


class WebServer
{
public:
  WebServer();
  ~WebServer();

  void setAddress(const IPAddress& ipAddress);
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
  void handleSettings();
  void handleStatus();
  void handleFlights();
  void handleFlight();
  void handleConfig();

  void handleReset();
  void handleResetAll();

  void handleTest();
};




#endif //webserver_h
