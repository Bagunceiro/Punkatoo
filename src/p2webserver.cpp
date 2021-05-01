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
    doc["ssid"] = WiFi.macAddress();
    doc["starttime"] = WiFi.macAddress();

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
    doc["ind"] = "1";

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(doc, *response);
    request->send(response);
}

void P2WebServer::wifiData(AsyncWebServerRequest *request, const char *name, networkList &n)
{
    StaticJsonDocument<1024> doc;
    StaticJsonDocument<1024> doca;

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
        StaticJsonDocument<64> doc;
        JsonObject obj = doc.to<JsonObject>();
        obj["ssid"] = list[i].ssid;
        obj["open"] = list[i].openNet;
        array.add(doc);
    }
}

void P2WebServer::sysupdData(AsyncWebServerRequest *request)
{
    StaticJsonDocument<512> doc;

    doc["controllername"] = config[controllername_n];
    doc["server"] = dev.updater.getServer();
    doc["port"] = String(dev.updater.getPort());
    doc["image"] = dev.updater.getSource();
    doc["target"] = dev.updater.getTarget();

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(doc, *response);
    request->send(response);
}

void P2WebServer::init()
{
    on("/rootdata.json", HTTP_GET, getRootData);
    on("/gendata.json", HTTP_GET, getGenData);
    on("/wificonfdata.json", HTTP_GET, getWifiConfData);
    on("/wifidiscdata.json", HTTP_GET, getWifiDiscData);
    on("/wifi.html", HTTP_POST, postWifiData);
    on("/netedit.html", HTTP_POST, postNetEdit);
    on("/sysupd.html", HTTP_POST, postSysupd);
    on("/sysupddata.json", HTTP_GET, getSysupdData);
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

/*
void P2WebServer::sendPage(AsyncWebServerRequest *req, ...)
{
    va_list args;
    va_start(args, req);

    AsyncResponseStream *response = req->beginResponseStream("text/html");
    char *s;
    for (int i = 0; (s = va_arg(args, char *)) != NULL; i++)
    {
        response->print(s);
    }
    va_end(args);
    req->send(response);
}
*/
/*
const char *HEAD = R"!(<HEAD><meta http-equiv="content-type" charset="UTF-8"
content="text/html, width=device-width, initial-scale=1">)!";
const char *TITLE = "<TITLE>";
const char *STYLES = R"!(<script src="/navfuncs.js"></script>
<link rel="stylesheet" href="/punkatoo.css">)!";
const char *END_TITLE = "</TITLE>";
const char *END_HEAD = "</HEAD>";
const char *BODY = "<BODY>";
const char *END_BODY = "</BODY>";
const char *DIV_HEADER = R"!(<div class="header" id="myHeader">)!";
const char *END_DIV = "</div>";
const char *BUTTON_HOME = "<button onclick=gohome()>Home</button>";
const char *BUTTON_GENERAL = "<button onclick=gogenconf()>General</button>";
const char *BUTTON_WIFI = "<button onclick=gowificonf()>WiFi</button>";
const char *BUTTON_RESET = "<button onclick=goreset()>Reset</button>";
const char *BUTTON_SYSUPDATE = "<button onclick=gosysupdate()>System Update</button>";
const char *BUTTON_UPDATE = "<button type=submit form=theform>Update</button>";

const char *DIV_CONTENT = "<div class=content>";
const char *TABLE = "<TABLE>";
const char *END_TABLE = "</TABLE>";
*/
/*
void P2WebServer::messagePage(AsyncWebServerRequest *req, const char *message)
{
    sendPage(req, HEAD, TITLE, "Punkatoo Message", END_TITLE, STYLES,
             "<meta http-equiv=\"refresh\" content=\"15;url=/\" />",
             END_HEAD,
             BODY,
             DIV_HEADER, BUTTON_HOME, BUTTON_RESET, END_DIV,
             DIV_CONTENT,
             "<BR><B>Controller: ",
             config[controllername_n].c_str(),
             "</B><br><br>",
             message,
             END_DIV,
             END_BODY,
             NULL);
}
*/

void P2WebServer::sysReset(AsyncWebServerRequest *req)
{
    dev.p2sys.reset();
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

void P2WebServer::progressCB(size_t completed, size_t total, void *data)
{
    extern AsyncEventSource events;
    static int oldPhase = 1;
    int progress = (completed * 100) / total;

    int phase = (progress / 5) % 2; // report at 5% intervals

    if (phase != oldPhase)
    {
        if (dev.indicators.size() > 0)
        {
            if (phase)
                dev.indicators[0].off();
            else
                dev.indicators[0].setColour(indicate_update, true);
        }
        serr.printf("Progress: %d%% (%d/%d)\n", progress, completed, total);
        char buff[32];
        snprintf(buff, sizeof(buff) - 1, "Progress: %d%% (%u/%u)", progress, completed, total);
        updateInfo(buff, NULL);

        oldPhase = phase;
    }
}

void P2WebServer::updateInfo(const char *message, void *)
{
    if (message != NULL)
    {
        pThis->event("progress", message);
    }
    else
    {
        pThis->event("progress", "No Message");
    }
    StaticJsonDocument<512> doc;
    doc["message"] = message;
    String jmsg;
    serializeJson(doc, jmsg);
    pThis->event("updprog", jmsg.c_str());
}

void P2WebServer::sysupdRecd(AsyncWebServerRequest *req)
{
    String server;
    String port;
    String image;
    String target;

    for (int i = 0; i < req->args(); i++)
    {
        const String argN = req->argName(i);
        if (argN == "server")
            server = req->arg(i);
        else if (argN == "port")
            port = req->arg(i);
        else if (argN == "image")
            image = req->arg(i);
        else if (argN == "target")
            target = req->arg(i);
    }

    String ext;
    int extidx = image.lastIndexOf('.');
    if (extidx >= 0)
        ext = image.substring(extidx + 1);

    serr.printf("Update %s, %ld, %s\n", server.c_str(), port.toInt(), image.c_str());

    if (ext == "bin")
    {
        doUpdateSysPage(req, server.c_str(), port.toInt(), image.c_str());
    }
    else
    {
        doUpdateConfPage(req, server.c_str(), port.toInt(), image.c_str(), target.c_str());
    }
}

void P2WebServer::doUpdateSysPage(AsyncWebServerRequest *req, const char *server, const int port, const char *image)
{
    dev.updater.onStart(updateInfo);
    dev.updater.onEnd(updateInfo);
    dev.updater.onFail(updateInfo);
    dev.updater.onProgress(progressCB);
    dev.updater.setRemote(Updater::UPD_SYS, server, port, image, "");
    serveFile(req);
}

void P2WebServer::doUpdateConfPage(AsyncWebServerRequest *req, const char *server, const int port, const char *src, const char *target)
{
    dev.updater.onEnd(updateInfo);
    dev.updater.onFail(updateInfo);
    dev.updater.onProgress(progressCB);
    dev.updater.setRemote(Updater::UPD_CONF, server, port, src, target);
    serveFile(req);
}

void P2WebServer::event(const char *name, const char *content)
{
    events->send(content, name, eventid++);
}