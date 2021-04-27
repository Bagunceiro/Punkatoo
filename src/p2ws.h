#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "networks.h"

class P2WebServer : public AsyncWebServer
{
    /*
     * The Punkatoo website
     */
public:
    P2WebServer(unsigned int port = 80) : AsyncWebServer(port)
    {
        pThis = this;
        events = new AsyncEventSource("/events");
        eventid = 0;
    }
    void init();
    void event(const char *name, const char *content);
    void event(const char *name, const long content);
    void event(const char *name, const double content);

    static void handleRoot(AsyncWebServerRequest *r) { pThis->rootPage(r); }
    static void handleGenConfig(AsyncWebServerRequest *r) { pThis->genConfigPage(r); }
    static void handleGenUpdate(AsyncWebServerRequest *r) { pThis->genUpdatePage(r); }
    static void handleNetConfig(AsyncWebServerRequest *r) { pThis->netConfigPage(r); }
    static void handleNetEdit(AsyncWebServerRequest *r) { pThis->netEditPage(r); }
    static void handleNewNet(AsyncWebServerRequest *r) { pThis->newNetPage(r); }
    static void blankResetMessage(AsyncWebServerRequest *r) { pThis->blankResetMessagePage(r); }
    static void handleSystemUpdate(AsyncWebServerRequest *r) { pThis->systemUpdatePage(r); }
    static void handleDoUpdate(AsyncWebServerRequest *r) { pThis->doUpdatePage(r); }

    static void progressCB(size_t completed, size_t total, void * data);

private:
    static P2WebServer *pThis;

    void rootPage(AsyncWebServerRequest *r);
    void genConfigPage(AsyncWebServerRequest *r);
    void genUpdatePage(AsyncWebServerRequest *r);
    void netConfigPage(AsyncWebServerRequest *r);
    void netEditPage(AsyncWebServerRequest *r);
    void newNetPage(AsyncWebServerRequest *r);
    void resetMessagePage(AsyncWebServerRequest *r, const String &reason);
    void blankResetMessagePage(AsyncWebServerRequest *r);
    void systemUpdatePage(AsyncWebServerRequest *r);
    void doUpdatePage(AsyncWebServerRequest *r);
    void messagePage(AsyncWebServerRequest *r, const String &message);
    void sendPage(AsyncWebServerRequest *r, const int noItems, ...);
    static void updateCompleted(void*);

    String &listNetworks(String &body, networkList &networks, bool selected);
  
    static const char *pageRoot;
    static const char *pageGen;
    static const char *pageGenUpdate;
    static const char *pageWiFi;
    static const char *pageWiFiNet;
    static const char *pageWiFiNetAdd;
    static const char *pageReset;
    static const char *pageSystemUpdate;
    static const char *pageDoUpdate;

    static const String style;

    static const String head1;
    static const String head2;
    static const String headEnd;
    static const String body1;
    AsyncEventSource *events;
    unsigned long eventid;
};