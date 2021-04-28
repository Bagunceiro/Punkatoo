#pragma once

#include <HTTPUpdate.h>

#include "mqtt.h"

class Updater : public P2Task
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
    // void systemUpdate(const String &s, const uint16_t p, const String &i, updateType t);
    bool operator()() override;
    void onStart(void (*callback)(const char*, void *), void * = NULL);
    void onEnd(void (*callback)(const char*, void *), void * = NULL);
    void onNone(void (*callback)(const char*, void *), void * = NULL);
    void onFail(void (*callback)(const char*, void *), void * = NULL);
    void onProgress(void (*callback)(size_t completed, size_t total, void *), void * = NULL);
    void setRemote(updateType t, const char *s, const uint16_t p, const char *f, const char *tgt)
    {
        server = s;
        port = p;
        source = f;
        uType = t;
        target = tgt;
    }
    static void progcb(size_t completed, size_t total) { pThis->doprogcb(completed, total); }

private:
    void doprogcb(size_t completed, size_t total);
    void systemUpdate();
    void configUpdate();

    void (*startCallback)(const char*, void *);
    void (*endCallback)(const char*, void *);
    void (*nullCallback)(const char*, void *);
    void (*failCallback)(const char*, void *);
    void (*progCallback)(size_t completed, size_t total, void *);
    void *startcbdata;
    void *endcbdata;
    void *nullcbdata;
    void *failcbdata;
    void *progcbdata;
    char messageBuffer[32];

    String server;
    int port;
    String source;
    String target;
    updateType uType;
    static Updater *pThis;
};