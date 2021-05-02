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

    //static void handleRoot(AsyncWebServerRequest *r) { pThis->rootPage(r); }
    // static void handleGenConfig(AsyncWebServerRequest *r) { pThis->genConfigPage(r); }
    // static void handleGenUpdate(AsyncWebServerRequest *r) { pThis->genUpdatePage(r); }
    // static void handleNetConfig(AsyncWebServerRequest *r) { pThis->netConfigPage(r); }
    // static void handleNetEdit(AsyncWebServerRequest *r) { pThis->netEditPage(r); }
    // static void handleNewNet(AsyncWebServerRequest *r) { pThis->newNetPage(r); }
    // static void handleReset(AsyncWebServerRequest *r) { pThis->blankResetMessagePage(r); }
    // static void handleSystemUpdate(AsyncWebServerRequest *r) { pThis->systemUpdatePage(r); }
    // static void handleDoUpdate(AsyncWebServerRequest *r) { pThis->doUpdatePage(r); }

    static void progressCB(size_t completed, size_t total, void * data);

private:
    static P2WebServer *pThis;

    // void rootPage(AsyncWebServerRequest *r);
    // void genConfigPage(AsyncWebServerRequest *r);
    // void genUpdatePage(AsyncWebServerRequest *r);
    // void netConfigPage(AsyncWebServerRequest *r);
    // void netEditPage(AsyncWebServerRequest *r);
    
    // void resetMessagePage(AsyncWebServerRequest *r, const char* reason = "");
    // void blankResetMessagePage(AsyncWebServerRequest *r);
    // void systemUpdatePage(AsyncWebServerRequest *r);
    // void doUpdatePage(AsyncWebServerRequest *r);
    void doUpdateSysPage(AsyncWebServerRequest *r, const char* srv, const int port, const char* src);
    void doUpdateConfPage(AsyncWebServerRequest *r,  const char* srv, const int port, const char* src, const char* target);
    // void messagePage(AsyncWebServerRequest *r, const char* message);
    // void sendPage(AsyncWebServerRequest *r, ...);
    
    static void updateInfo(const char*, void*);

    void rootData(AsyncWebServerRequest *r);
    void genData(AsyncWebServerRequest *r);
    void genDataRecd(AsyncWebServerRequest *r);
    void wifiData(AsyncWebServerRequest *r, const char* name, networkList& n);
    void wifiConfData(AsyncWebServerRequest *r);
    void wifiDiscData(AsyncWebServerRequest *r);
    void wifiDataRecd(AsyncWebServerRequest *r);
    void netEditRecd(AsyncWebServerRequest *r);
    void sysupdData(AsyncWebServerRequest *r);
    void sysupdRecd(AsyncWebServerRequest *r);
    void sysReset(AsyncWebServerRequest *r);

    static void getRootData(AsyncWebServerRequest *r) { pThis->rootData(r);}
    static void getGenData(AsyncWebServerRequest *r) { pThis->genData(r);}
    static void getWifiConfData(AsyncWebServerRequest *r) { pThis->wifiConfData(r);}
    static void getWifiDiscData(AsyncWebServerRequest *r) { pThis->wifiDiscData(r);}
    static void postGenData(AsyncWebServerRequest *r) { pThis->genDataRecd(r);}
    static void postWifiData(AsyncWebServerRequest *r) { pThis->wifiDataRecd(r);}
    static void postNetEdit(AsyncWebServerRequest *r) { pThis->netEditRecd(r);}
    static void getSysupdData(AsyncWebServerRequest *r) { pThis->sysupdData(r);}
    static void postSysupd(AsyncWebServerRequest *r) { pThis->sysupdRecd(r);}
    static void doSysReset(AsyncWebServerRequest *r) { pThis->sysReset(r);}

    // String &listNetworks(String &body, networkList &networks, bool selected);
    void addNetworks(JsonArray& array, networkList& list);
  
/*
    static const char *pageRoot;
    static const char *pageGen;
    static const char *pageGenUpdate;
    static const char *pageWiFi;
    static const char *pageWiFiNet;
    static const char *pageWiFiNetAdd;
    static const char *pageReset;
    static const char *pageSystemUpdate;
    static const char *pageDoUpdate;
*/

    AsyncEventSource *events;
    unsigned long eventid;
};