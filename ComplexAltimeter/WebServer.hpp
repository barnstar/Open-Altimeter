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

  void bindSavedFlights();
  String savedFlightLinks();
  void response();

  void handleRoot();
  void handleSettings();
  void handleStatus();
  void handleFlights();
  
  void handleReset();
  void handleResetAll();
  
  void handleTest();
};

#endif //webserver_h
