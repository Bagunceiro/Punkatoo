#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "networks.h"

/**
 * @brief The Punkatoo website
 */
class P2WebServer : public AsyncWebServer
{

public:
    P2WebServer(unsigned int port = 80) : AsyncWebServer(port)
    {
        pThis = this;
        events = new AsyncEventSource("/events");
        eventid = 0;
    }
    void init();
    void event(const char *name, const char *content);

    static void progressCB(size_t completed, size_t total, void *data);
    void sendEvents();

private:
    static P2WebServer *pThis;

    static void updateInfo(const char *, void *);

    void rootData(AsyncWebServerRequest *r);
    void genData(AsyncWebServerRequest *r);
    void genDataRecd(AsyncWebServerRequest *r);
    void wifiData(AsyncWebServerRequest *r, const char *name, networkList &n);
    void wifiConfData(AsyncWebServerRequest *r);
    void wifiDiscData(AsyncWebServerRequest *r);
    void wifiDataRecd(AsyncWebServerRequest *r);
    void netEditRecd(AsyncWebServerRequest *r);
    void sysReset(AsyncWebServerRequest *r);

    static void getRootData(AsyncWebServerRequest *r) { pThis->rootData(r); }
    static void getGenData(AsyncWebServerRequest *r) { pThis->genData(r); }
    static void getWifiConfData(AsyncWebServerRequest *r) { pThis->wifiConfData(r); }
    static void getWifiDiscData(AsyncWebServerRequest *r) { pThis->wifiDiscData(r); }
    static void postGenData(AsyncWebServerRequest *r) { pThis->genDataRecd(r); }
    static void postWifiData(AsyncWebServerRequest *r) { pThis->wifiDataRecd(r); }
    static void postNetEdit(AsyncWebServerRequest *r) { pThis->netEditRecd(r); }
    static void doSysReset(AsyncWebServerRequest *r) { pThis->sysReset(r); }


    void addNetworks(JsonArray &array, networkList &list);
    unsigned long eventid;

public:
    AsyncEventSource *events;
};