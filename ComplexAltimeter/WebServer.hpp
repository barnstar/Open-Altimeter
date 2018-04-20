#ifndef webserver_h
#define webserver_h

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

class WebServer;

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

  String title;
  String body;
  String script;

  void startPageStream(ESP8266WebServer *s);
  void sendHeaders(ESP8266WebServer *s);
  void sendBodyChunk(ESP8266WebServer *s, const String &chunk, bool addStartTag, bool addClosingTag);
  void sendScript(ESP8266WebServer *s, const String &script);
  void sendRawText(ESP8266WebServer *s, const String &rawText);
  void closePageStream(ESP8266WebServer *s);

  void appendToBody(const String &html);
  void appendScriptLink(const String &link);
  void appendScript(const String &script);

  static String makeLink(const String &link, const String &string);
  static String makeDiv(const String &name, const String &contents);


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
  void handleStatus();
  void handleFlights();
  void handleFlight();
  void handleConfig();

  void handleReset();
  void handleResetAll();
  void handleConfigSetting(String &arg, String &val);
  
  void handleTest();
};




#endif //webserver_h
