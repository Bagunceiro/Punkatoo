#include <LITTLEFS.h>
#include <HTTPClient.h>
#include "config.h"
#include "p2webserver.h"
#include "devices.h"

P2WebServer *P2WebServer::pThis;

const String wwwpath = "/www";

void serveFile(AsyncWebServerRequest *request)
{
    String file = request->url();
    const char *mimetype;
    if (file.endsWith(".js"))
        mimetype = "application/javascript";
    else if (file.endsWith(".css"))
        mimetype = "text/css";
    else if (file.endsWith(".html") || (file.endsWith(".htm")))
        mimetype = "text/html";
    else
        mimetype = "application/octet-stream";
    request->send(LITTLEFS, wwwpath + file, mimetype);
}

void P2WebServer::rootData(AsyncWebServerRequest *request)
{
    StaticJsonDocument<512> doc;

    doc["ctlr"] = config[controllername_n];
    doc["revision"] = gitrevision;
    doc["comptime"] = compDateTime;
    doc["mac"] = WiFi.macAddress();
    doc["ssid"] = WiFi.SSID();
    doc["bssid"] = WiFi.BSSIDstr();
    doc["starttime"] = startTime();

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(doc, *response);
    request->send(response);
}

void P2WebServer::genData(AsyncWebServerRequest *request)
{
    StaticJsonDocument<512> doc;

    doc["controllername"] = config[controllername_n];
    doc["mqtthost"] = config[mqtthost_n];
    doc["mqttport"] = config[mqttport_n];
    doc["mqttuser"] = config[mqttuser_n];
    doc["mqttpwd"] = config[mqttpwd_n];
    doc["mqttroot"] = config[mqttroot_n];
    doc["mqtttopic"] = config[mqtttopic_n];
    doc["selection"] = String("indic_") + config[indicator_n];

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(doc, *response);
    request->send(response);
}

void P2WebServer::wifiData(AsyncWebServerRequest *request, const char *name, networkList &n)
{
    StaticJsonDocument<1566> doc;
    StaticJsonDocument<1566> doca;

    JsonArray array = doca.to<JsonArray>();

    addNetworks(array, n);

    doc["controllername"] = config[controllername_n];
    doc[name] = doca;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(doc, *response);
    request->send(response);
}

void P2WebServer::wifiConfData(AsyncWebServerRequest *request)
{
    networkList &cnetworks = networkConfRead();
    wifiData(request, "confnets", cnetworks);
}
void P2WebServer::wifiDiscData(AsyncWebServerRequest *request)
{
    networkList &snetworks = scanNetworks();
    wifiData(request, "discnets", snetworks);
}

void P2WebServer::addNetworks(JsonArray &array, networkList &list)
{
    for (unsigned int i = 0; i < list.size(); i++)
    {
        StaticJsonDocument<100> doc;
        JsonObject obj = doc.to<JsonObject>();
        obj["ssid"] = list[i].ssid;
        obj["open"] = list[i].openNet;
        obj["rssi"] = list[i].rssi;
        array.add(doc);
    }
}

void P2WebServer::init()
{
    on("/rootdata.json", HTTP_GET, getRootData);
    on("/gendata.json", HTTP_GET, getGenData);
    on("/wificonfdata.json", HTTP_GET, getWifiConfData);
    on("/wifidiscdata.json", HTTP_GET, getWifiDiscData);
    on("/general.html", HTTP_POST, postGenData);
    on("/wifi.html", HTTP_POST, postWifiData);
    on("/netedit.html", HTTP_POST, postNetEdit);
    on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LITTLEFS, wwwpath + "/index.html", "text/html");
    });
    on("/reset.html", HTTP_ANY, doSysReset);

    onNotFound(serveFile);

    events->onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId())
        {
            serr.println("Client reconnected");
        }
    });
    addHandler(events);

    begin();
}

void P2WebServer::sysReset(AsyncWebServerRequest *req)
{
    dev.p2sys.reset();
    serveFile(req);
}

void P2WebServer::genDataRecd(AsyncWebServerRequest *req)
{
    for (uint8_t i = 0; i < req->args(); i++)
    {
        const String argN = req->argName(i);
        const String value = req->arg(i);

        if (argN == "controllername")
            config[controllername_n] = value;
        else if (argN == "mqtthost")
            config[mqtthost_n] = value;
        else if (argN == "mqttport")
            config[mqttport_n] = value;
        else if (argN == "mqttuser")
            config[mqttuser_n] = value;
        else if (argN == "mqttpwd")
            config[mqttpwd_n] = value;
        else if (argN == "mqttroot")
            config[mqttroot_n] = value;
        else if (argN == "mqtttopic")
            config[mqtttopic_n] = value;
        else if (argN == "indicator")
        if (value.length() > 0)
            config[indicator_n] = value;
        config.writeFile();
    }

    serveFile(req);
}

void P2WebServer::wifiDataRecd(AsyncWebServerRequest *req)
{
    if (req->method() != HTTP_POST)
    {
        Serial.printf("In wifiDataRecd with wrong method (%d)\n", req->method());
    }
    serr.println("Network Update");
    networkList newlist;

    bool usenext = false;
    for (uint8_t i = 0; i < req->args(); i++)
    {
        const String argN = req->argName(i);
        const String value = req->arg(i);

        if (usenext && (argN == "ssid"))
        {
            addNetwork(newlist, value);
        }
        if (argN == "conf")
            usenext = true;
        else
        {
            usenext = false;
            if (argN == "newnet")
            {
                if (value.length() != 0)
                {
                    addNetwork(newlist, value);
                }
            }
        }
    }
    networkConfWrite(newlist);
    serveFile(req);
}

void P2WebServer::netEditRecd(AsyncWebServerRequest *req)
{
    WiFiNetworkDef net("");
    net.openNet = true;

    for (uint8_t i = 0; i < req->args(); i++)
    {
        const String argN = req->argName(i);

        if (argN == "ssid")
        {
            net.ssid = req->arg(i);
        }
        else if (argN == "psk")
        {
            net.psk = req->arg(i);
            net.openNet = false;
        }
    }
    serr.printf("Edited network %s\n", net.ssid.c_str());
    updateWiFiDef(net);
    serveFile(req);
}

void P2WebServer::event(const char *name, const char *content)
{
    events->send(content, name, eventid++);
}