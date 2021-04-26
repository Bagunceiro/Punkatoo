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
    void onStart(void (*callback)(void*), void* = NULL);
    void onEnd(void (*callback)(void*), void* = NULL);
    void onNone(void (*callback)(void*), void* = NULL);
    void onFail(void (*callback)(void*), void* = NULL);
    void setRemote(const String &s, const uint16_t p, const String &i, bool r)
    {server = s; port = p; image = i; ready = r; }

private:
    void (*startCallback)(void*);
    void (*endCallback)(void*);
    void (*nullCallback)(void*);
    void (*failCallback)(void*);
    void* startcbdata;
    void* endcbdata;
    void* nullcbdata;
    void* failcbdata;


private:
    String server;
    int port;
    String image;
    bool ready;
};