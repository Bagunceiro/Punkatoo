#pragma once

#include <HTTPUpdate.h>

#include "mqtt.h"

class Updater
{
public:
    Updater(const String &devName = "updater");
    virtual ~Updater();
    t_httpUpdate_return systemUpdate(const String &server, const uint16_t port, const String &image, bool ready);
    t_httpUpdate_return systemUpdate();
    void onStart(void (*callback)());
    void onEnd(void (*callback)());
    void onNone(void (*callback)());
    void onFail(void (*callback)());
    void setRemote(const String &s, const uint16_t p, const String &i, bool r)
    {server = s; port = p; image = i; ready = r; }

private:
    void (*startCallback)();
    void (*endCallback)();
    void (*nullCallback)();
    void (*failCallback)();

private:
    String server;
    int port;
    String image;
    bool ready;
};