#pragma once

#include <WebServer.h>

class FanConWebServer : public WebServer
{
public:
    FanConWebServer(unsigned int port) : WebServer(port) {}
    void init();
};

extern FanConWebServer webServer;
