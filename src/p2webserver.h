#pragma once

#include <WebServer.h>
#include "networks.h"

class P2WebServer : public WebServer
{
    /*
     * The Punkatoo website
     */
public:
    P2WebServer(unsigned int port=80) : WebServer(port) { pThis = this; }
    void init();

    private:
    void rootPage();
    void genConfigPage();
    void genUpdatePage();
    void netConfigPage();
    void netEditPage();
    void newNetPage();
    void resetMessagePage(const String& reason);
    void blankResetMessagePage();
    void systemUpdatePage();
    void doUpdatePage();
    void messagePage(const String& message);
    void sendPage(const int noItems, ...);

    String &listNetworks(String &body, networkList &networks, bool selected);
    const String bmeData();
    const String lightLevels();

    static P2WebServer* pThis;

    static void handleRoot()         { pThis->rootPage(); }
    static void handleGenConfig()    { pThis->genConfigPage(); }
    static void handleGenUpdate()    { pThis->genUpdatePage(); }
    static void handleNetConfig()    { pThis->netConfigPage(); }
    static void handleNetEdit()      { pThis->netEditPage(); }
    static void handleNewNet()       { pThis->newNetPage(); }
    static void blankResetMessage()  { pThis->blankResetMessagePage(); }
    static void handleSystemUpdate() { pThis->systemUpdatePage(); }
    static void handleDoUpdate()     { pThis->doUpdatePage(); }

    static const String pageRoot;
    static const String pageGen;
    static const String pageGenUpdate;
    static const String pageWiFi;
    static const String pageWiFiNet;
    static const String pageWiFiNetAdd;
    static const String pageReset;
    static const String pageSystemUpdate;
    static const String pageDoUpdate;

    static const String style;

    static const String head1;
    static const String head2;
    static const String headEnd;
    static const String body1;
};