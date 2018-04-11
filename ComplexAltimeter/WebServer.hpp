#ifndef webserver_h
#define webserver_h

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

class WebServer
{
  public:
  WebServer();
  ~WebServer();

  void setAddress(const IPAddress& ipAddress);
  void handleClient();

  private:
  IPAddress ipAddress;
  ESP8266WebServer *server;

  void response();
  void handleSettings();
  void handleRoot();
};

#endif //webserver_h
