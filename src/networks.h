#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

/**
 * @struct WiFiNetworkDef
 * @brief Holds information about a WiFi network
 */
struct WiFiNetworkDef
{
    /**
     * @param s SSID
     * @param p PSK
     */
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

/**
 * @brief Write the list of networks to the network configuration file
 * @param list The list of networks
 * @return success
 */
extern bool networkConfWrite(networkList& list);
/**
 * @brief Read list of networks from the network configuration file
 * @return List of known networks
 */
extern networkList& networkConfRead();
/**
 * @brief Do a wifi scan for available networks
 * @return List of found networks
 */
extern networkList& scanNetworks();
extern void addNetwork(networkList& netlist, const String& ssid);
extern void updateWiFiDef(WiFiNetworkDef&);
extern void connectToWiFi();