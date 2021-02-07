#include <WiFiMulti.h>
#include <LITTLEFS.h>

#include <ArduinoJson.h>
#include "configurator.h"
#include "networks.h"
#include "WiFiSerial.h"

networkList configuredNets;
networkList scannedNets;

#define LittleFS LITTLEFS

WiFiMulti wifimulti;

extern Configurator configurator;

networkList &scanNetworks()
{
    scannedNets.clear();
    int n = WiFi.scanNetworks();
    if (n == 0)
    {
        WSerial.println("no networks found");
    }
    else
    {
        WSerial.print(n);
        WSerial.println(" networks found");
        scannedNets.clear();
        for (int i = 0; i < n; ++i)
        {
            // Print SSID and RSSI for each network found
            WiFiNetworkDef network(WiFi.SSID(i));
            network.openNet = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
            network.rssi = WiFi.RSSI(i);
            scannedNets.push_back(network);

            WSerial.print(i + 1);
            WSerial.print(": ");
            WSerial.print(WiFi.SSID(i));
            WSerial.print(" (");
            WSerial.print(WiFi.RSSI(i));
            WSerial.print(")");
            WSerial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
        }
    }
    WSerial.println("");
    return scannedNets;
}

networkList &networkConfRead()
{
    LittleFS.begin();

    File netsFile = LittleFS.open("/networks.json", "r");
    if (!netsFile)
    {
        perror("");
        Serial.println("Config file open for read failed");
        // TEMPORARY !!!!
            WiFiNetworkDef network("asgard_2g", "enaLkraP");
            configuredNets.push_back(network);
            networkConfWrite(configuredNets);
    }
    else
    {
        configuredNets.clear();
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, netsFile);
        if (error)
        {
            Serial.println(F("Failed to read network file"));
        }
        else
        {
        }
        JsonArray array = doc.as<JsonArray>();
        for (JsonObject net : array)
        {
            const char *ssid = (const char *)net["ssid"];
            const char *psk = (const char *)net["psk"];
            Serial.printf("Configured network: %s/%s\n", ssid, psk);

            WiFiNetworkDef network(ssid, psk);
            configuredNets.push_back(network);
        }
    }
    LittleFS.end();
    return configuredNets;
}

bool networkConfWrite(networkList &networks)
{
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.to<JsonArray>();

    LittleFS.begin();

    File netsFile = LittleFS.open("/networks.json", "w");
    if (!netsFile)
    {
        perror("");
        Serial.println("Config file open for write failed");
    }
    else
    {
        for (unsigned int i = 0; i < networks.size(); i++)
        {
            JsonObject object = array.createNestedObject();
            object["ssid"] = networks[i].ssid;
            object["psk"] = networks[i].psk;
        }
        serializeJson(doc, netsFile);
        netsFile.close();
    }
    LittleFS.end();
    return true;
}

void addNetwork(networkList &netlist, const String &ssid)
{
    bool added = false;
    for (uint16_t i = 0; i < netlist.size(); i++)
    {
        if (netlist[i].ssid == ssid)
        {
            Serial.printf("Already added %s\n", ssid.c_str());
            return;
        }
    }
    for (uint16_t i = 0; i < configuredNets.size(); i++)
    {
        if (configuredNets[i].ssid == ssid)
        {
            Serial.printf("Adding existing %s\n", ssid.c_str());
            WiFiNetworkDef d = configuredNets[i];
            netlist.push_back(d);
            added = true;
            break;
        }
    }
    if (!added)
    {
        Serial.printf("Adding new %s\n", ssid.c_str());

        WiFiNetworkDef d(ssid);
        netlist.push_back(d);
    }
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

void updateWiFiDef(String& ssid, String& psk)
{
    WiFiNetworkDef def(ssid, psk);
    updateWiFiDef(def);
}

void connectToWiFi()
{
    networkConfRead();
    unsigned int numNets = configuredNets.size();
    if (numNets > 0)
    {
        for (unsigned int i = 0; i < numNets; i++)
        {
            Serial.printf("Connect to %s/%s\n", configuredNets[i].ssid.c_str(), configuredNets[i].psk.c_str());
            wifimulti.addAP(configuredNets[i].ssid.c_str(), configuredNets[i].psk.c_str());
        }
        // In case of emergency break glass:
        // wifimulti.addAP("asgard_2g", "enaLkraP");
        wifimulti.run();
    }
    else
    {
        configurator.start();
    }
}