#ifndef NETWORKS_H
#define NETWORKS_H

#include <Arduino.h>
#ifdef ESP32

#include <WiFi.h>
#include <WiFiMulti.h>

#else

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#endif

struct WiFiNetworkDef
{
    WiFiNetworkDef(const String& s, const String& p = "")
    {
        ssid = s; psk = p; openNet = (psk.length() == 0);
    }
    String ssid;
    String psk;
    int rssi;
    bool openNet;
};

typedef std::vector<WiFiNetworkDef> networkList;

extern bool networkConfWrite(networkList& list);
extern networkList& networkConfRead();
extern networkList& scanNetworks();
extern void addNetwork(networkList& netlist, const String& ssid);
extern void updateWiFiDef(WiFiNetworkDef&);
extern void connectToWiFi();

// extern networkList& configuredNetworks();

#endif // NETWORKS_H