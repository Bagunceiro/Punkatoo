#include <WiFiMulti.h>
#include <LittleFS.h>

#include <ArduinoJson.h>
#include "config.h"
#include "devices.h"
#include "networks.h"
#include "crypt.h"

networkList configuredNets;
networkList scannedNets;

WiFiMulti wifimulti;

networkList &scanNetworks()
{
    scannedNets.clear();
    int n = WiFi.scanNetworks();
    serr.printf("%d networks found\n", n);
    scannedNets.clear();
    for (int i = 0; i < n; ++i)
    {
        // Print SSID and RSSI for each network found
        const String &netname = WiFi.SSID(i);
        WiFiNetworkDef network(netname);
        network.openNet = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
        network.rssi = WiFi.RSSI(i);
        scannedNets.push_back(network);

        /*
            serr.print(i + 1);
            serr.print(": ");
            serr.print(WiFi.SSID(i));
            serr.print(" (");
            serr.print(WiFi.RSSI(i));
            serr.print(")");
            serr.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
        */
    }
    std::sort(scannedNets.begin(), scannedNets.end(),
              [](WiFiNetworkDef i, WiFiNetworkDef j)
              { return (i.rssi > j.rssi); });
    return scannedNets;
}

networkList &networkConfRead()
{
    bool unencrypted = false; // indicates there is an unencrypted psk in the file
    File netsFile = LittleFS.open("/etc/networks.json", "r");
    if (!netsFile)
    {
        perror("");
        serr.println("Network file open for read failed");
    }
    else
    {
        configuredNets.clear();
        StaticJsonDocument<1500> doc;
        DeserializationError error = deserializeJson(doc, netsFile);
        if (error)
        {
            serr.println(F("Failed to read network file"));
        }
        else
        {
        }
        JsonArray array = doc.as<JsonArray>();
        for (JsonObject net : array)
        {
            const char *ssid = (const char *)net["ssid"];
            char *psk = (char*)net["psk"].as<const char*>();
            const char *pske = (const char*)net["pske"];

            if (psk == NULL)
            {
                String psks = pdecrypt64(pske);
                psk = (char*)psks.c_str();
            }
            else
            {
                unencrypted = true;
            }

            WiFiNetworkDef network(ssid, psk);
            configuredNets.push_back(network);
        }
        netsFile.close();
        if (unencrypted)
        { 
            Serial.println("Rewrite networks file");
            networkConfWrite(configuredNets);
        }
    }
    return configuredNets;
}

bool networkConfWrite(networkList &networks)
{
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.to<JsonArray>();

    File netsFile = LittleFS.open("/etc/networks.json", "w");
    if (!netsFile)
    {
        perror("");
        serr.println("Network file open for write failed");
    }
    else
    {
        for (unsigned int i = 0; i < networks.size(); i++)
        {
            JsonObject object = array.createNestedObject();
            object["ssid"] = networks[i].ssid;
            String psk = networks[i].psk;
            object["pske"] = pencrypt64(psk.c_str());
        }
        serializeJson(doc, netsFile);
        netsFile.close();
    }
    return true;
}

void addNetwork(networkList &netlist, const String &ssid)
{
    String psk;
    // Don't clobber the PSK if we already know it
    for (uint16_t i = 0; i < configuredNets.size(); i++)
    {
        if (configuredNets[i].ssid == ssid)
        {
            psk = configuredNets[i].psk;
        }
    }
    WiFiNetworkDef d(ssid,psk);
    netlist.push_back(d);
}

void updateWiFiDef(WiFiNetworkDef &net)
{
    for (uint16_t i = 0; i < configuredNets.size(); i++)
    {
        if (configuredNets[i].ssid == net.ssid)
        {
            // Get rid of the old version
            configuredNets.erase(configuredNets.begin() + i);
        }
    }
    configuredNets.push_back(net);
    networkConfWrite(configuredNets);
}

void updateWiFiDef(String &ssid, String &psk)
{
    WiFiNetworkDef def(ssid, psk);
    updateWiFiDef(def);
}

void connectToWiFi()
{
    static long then = 0;
    long now = millis();
    if ((then == 0) || ((now - then) >= WIFI_CONNECT_ATTEMPT_INT))
    {
        then = now;
        networkConfRead();
        unsigned int numNets = configuredNets.size();
        if (numNets > 0)
        {
            for (unsigned int i = 0; i < numNets; i++)
            {
                // Serial.printf("Connect to %s/%s\n", configuredNets[i].ssid.c_str(), configuredNets[i].psk.c_str());
                Serial.printf("Connect to %s\n", configuredNets[i].ssid.c_str());
                wifimulti.addAP(configuredNets[i].ssid.c_str(), configuredNets[i].psk.c_str());
            }
            delay(500);
            wifimulti.run();
        }
        else
        {
            // dev.configurator.start();
        }
    }
}