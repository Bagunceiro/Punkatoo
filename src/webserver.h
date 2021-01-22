#pragma once

#ifdef ESP32
#include <WebServer.h>
#else
#include <ESP8266WebServer.h>
#define WebServer ESP8266WebServer
#endif

class FanConWebServer : public WebServer
{
public:
    FanConWebServer(unsigned int port) : WebServer(port) {}
    void init();
};

extern FanConWebServer webServer;
