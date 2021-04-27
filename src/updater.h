#pragma once

#include <HTTPUpdate.h>

#include "mqtt.h"

class Updater
{
public:
    enum updateType
    {
        UPD_NONE,
        UPD_SYS,
        UPD_CONF
    };
    Updater(const String &devName = "updater");
    virtual ~Updater();
    void systemUpdate(const String &s, const uint16_t p, const String &i, updateType t);
    void systemUpdate();
    void onStart(void (*callback)(void *), void * = NULL);
    void onEnd(void (*callback)(void *), void * = NULL);
    void onNone(void (*callback)(void *), void * = NULL);
    void onFail(void (*callback)(void *), void * = NULL);
    void onProgress(void (*callback)(size_t completed, size_t total, void *), void* = NULL);
    void setRemote(const String &s, const uint16_t p, const String &i, updateType t)
    {
        server = s;
        port = p;
        image = i;
        uType = t;
    }
    static void progcb(size_t completed, size_t total);

private:
    void (*startCallback)(void *);
    void (*endCallback)(void *);
    void (*nullCallback)(void *);
    void (*failCallback)(void *);
    void (*progCallback)(size_t completed, size_t total, void *);
    void *startcbdata;
    void *endcbdata;
    void *nullcbdata;
    void *failcbdata;
    void *progcbdata;

    String server;
    int port;
    String image;
    updateType uType;
    static Updater* pThis;
};