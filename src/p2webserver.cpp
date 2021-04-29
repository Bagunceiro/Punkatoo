#include <LITTLEFS.h>
#include <HTTPClient.h>
#include "config.h"
#include "p2webserver.h"
#include "devices.h"

P2WebServer *P2WebServer::pThis;

// const char *P2WebServer::pageRoot = "/";
// const char *P2WebServer::pageGen = "/config.gen";
const char *P2WebServer::pageGenUpdate = "/config.update";
const char *P2WebServer::pageWiFi = "/config.net";
const char *P2WebServer::pageWiFiNet = "/config.netedit";
const char *P2WebServer::pageWiFiNetAdd = "/config.addnet";
const char *P2WebServer::pageReset = "/reset";
const char *P2WebServer::pageSystemUpdate = "/system.update";
const char *P2WebServer::pageDoUpdate = "/system.update.do";
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
    Serial.printf("File %s requested\n", file.c_str());
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

    String data();
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response = request->beginResponseStream("text/html");
    serializeJson(doc, *response);
    // response->print(data);
    Serial.printf("Root data requested\n");
    request->send(response);
}

void P2WebServer::genData(AsyncWebServerRequest *request)
{
    StaticJsonDocument<512> doc;

    // doc["ctlr"] = config[controllername_n];
    doc["controllername"] = config[controllername_n];
    doc["mqtthost"] = config[mqtthost_n];
    doc["mqttport"] = config[mqttport_n];
    doc["mqttuser"] = config[mqttuser_n];
    doc["mqttpwd"] = config[mqttpwd_n];
    doc["mqttroot"] = config[mqttroot_n];
    doc["mqtttopic"] = config[mqtttopic_n];
    doc["ind"] = "1";

    String data();
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response = request->beginResponseStream("text/html");
    serializeJson(doc, *response);
    // response->print(data);
    Serial.printf("Root data requested\n");
    request->send(response);
}


void P2WebServer::wifiData(AsyncWebServerRequest *request)
{

    String confnets;
    String discnets;
    networkList &cnetworks = networkConfRead();
    networkList &snetworks = scanNetworks();
    std::sort(snetworks.begin(), snetworks.end(), [](WiFiNetworkDef i, WiFiNetworkDef j) { return (i.rssi > j.rssi); });

    listNetworks(confnets, cnetworks, true);
    listNetworks(discnets, snetworks, false);
    
    StaticJsonDocument<1024> doc;

    // doc["ctlr"] = config[controllername_n];
    doc["controllername"] = config[controllername_n];
    doc["discnets"] = discnets;
    doc["confnets"] = confnets;


    String data();
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response = request->beginResponseStream("text/html");
    serializeJson(doc, *response);
    // response->print(data);
    Serial.printf("Root data requested\n");
    request->send(response);
}

void P2WebServer::init()
{
    // HTTP_ANY for now. should be HTTP_GET etc?
    // on(pageGen, HTTP_ANY, handleGenConfig);
    on(pageGenUpdate, HTTP_ANY, handleGenUpdate);
    on(pageWiFi, HTTP_ANY, handleNetConfig);
    on(pageWiFiNet, HTTP_ANY, handleNetEdit);
    on(pageWiFiNetAdd, HTTP_ANY, handleNewNet);
    on(pageReset, HTTP_ANY, handleReset);
    on(pageSystemUpdate, HTTP_ANY, handleSystemUpdate);
    on(pageDoUpdate, HTTP_ANY, handleDoUpdate);

    on("/rootdata.json", HTTP_GET, getRootData);
    on("/gendata.json", HTTP_GET, getGenData);
    on("/wifidata.json", HTTP_GET, getWifiData);
    on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LITTLEFS, wwwpath + "/index.html", "text/html");
    });

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

void P2WebServer::resetMessagePage(AsyncWebServerRequest *req, const char *reason)
{
    messagePage(req, reason);
    dev.p2sys.reset();
}

void P2WebServer::blankResetMessagePage(AsyncWebServerRequest *req)
{
    resetMessagePage(req, "Reseting, please wait");
}

void P2WebServer::genUpdatePage(AsyncWebServerRequest *req)
{
    if (req->method() == HTTP_POST)
    {
        config[indicator_n] = "0";

        for (uint8_t i = 0; i < req->args(); i++)
        {
            const String argN = req->argName(i);
            if (argN == indicator_n)
            {
                config[argN] = "1";
            }
            else
                config[argN] = req->arg(i);
        }
        config.dump(serr);
        config.writeFile();

        messagePage(req, "Configuration Updated");
    }
}

/*
void P2WebServer::genConfigPage(AsyncWebServerRequest *req)
{
    sendPage(req, HEAD, TITLE, "Punkatoo Message", END_TITLE, STYLES,
             END_HEAD,
             BODY,
             DIV_HEADER, BUTTON_HOME, BUTTON_RESET, BUTTON_UPDATE, END_DIV,
             DIV_CONTENT,

             "<BR><B>General Configuration: ", config[controllername_n].c_str(),
             "<FORM id=theform method=post action=\"", pageGenUpdate, "\">",
             TABLE,
             R"!(<tr><td><label for=ctlrname>Controller Name:</label></td><td><input type=text name=")!",
             controllername_n, "\" value=\"", config[controllername_n].c_str(), R"!("></td></tr>)!",
             R"!(<tr><td><label for=mqtthost>MQTT Broker:</label></td><td><input type=text name=")!",
             mqtthost_n, "\" value=\"", config[mqtthost_n].c_str(), R"!("></td></tr>)!",
             R"!(<tr><td><label for=mqttport>MQTT Port:</label></td><td><input type=text name=")!",
             mqttport_n, "\" value=\"", config[mqttport_n].c_str(), R"!("></td></tr>)!",
             R"!(<tr><td><label for=mqttuser>MQTT User:</label></td><td><input type=text name=")!",
             mqttuser_n, "\" value=\"", config[mqttuser_n].c_str(), R"!("></td></tr>)!",
             R"!(<tr><td><label for=mqttuser>MQTT Password:</label></td><td><input type=text name=")!",
             mqttpwd_n, "\" value=\"", config[mqttpwd_n].c_str(), R"!("></td></tr>)!",
             R"!(<tr><td><label for=mqttroot>MQTT Topic root:</label></td><td><input type=text name=")!",
             mqttroot_n, "\" value=\"", config[mqttroot_n].c_str(), R"!("></td></tr>)!",
             R"!(<tr><td><label for=mqtttopic>MQTT Topic:</label></td><td><input type=text name=")!",
             mqtttopic_n, "\" value=\"", config[mqtttopic_n].c_str(), R"!("></td></tr>)!",
             R"!(<tr><td><label for="ind">Status Indicator:</label></td><td><input type=checkbox id="ind" name=")!",
             indicator_n, "\"", (config[indicator_n] == "1" ? " checked" : ""), R"!(/><label for=ind>&nbsp;</label></td></tr>)!",
             END_TABLE,
             "</FORM>",
             END_DIV,
             END_BODY,
             NULL);
}
*/

String &P2WebServer::listNetworks(String &body, networkList &networks, bool selected)
{
    for (unsigned int i = 0; i < networks.size(); i++)
    {
        body += String(R"=====(
<tr>
<td>
<input type=checkbox)=====") +
                String(selected ? " checked" : "") + String(" id=") + String(selected ? "cf" : "ds") + String(i) + String(" name=conf") + String(R"=====(></input>
<label for=)=====") +
                String(selected ? "cf" : "ds") + i + String(R"=====(>&nbsp;</label>
<input type=hidden name=ssid value=")=====") +
                networks[i].ssid + String(R"=====("/>
</td>
<td>)=====") +
                String(networks[i].openNet ? "🔓" : "🔒") +
                (selected ? (String(" <a href=\"") +
                             P2WebServer::pageWiFiNet + "?ssid=" +
                             networks[i].ssid + "\">")
                          : "") +
                networks[i].ssid + String(selected ? "</a>" : "") + String(R"=====(
</td>
</tr>
)=====");
    }
    return body;
}

void P2WebServer::netConfigPage(AsyncWebServerRequest *req)
{
    if (req->method() == HTTP_POST)
    {
        serr.println("Network Update");
        networkList newlist;

        bool usenext = false;
        for (uint8_t i = 0; i < req->args(); i++)
        {
            const String argN = req->argName(i);
            const String value = req->arg(i);

            if (usenext && (argN == "ssid"))
            {
                // serr.println(value);
                addNetwork(newlist, value);
            }
            // serr.println(argName.c_str());
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
    }

    String confnets;
    String discnets;
    networkList &cnetworks = networkConfRead();
    networkList &snetworks = scanNetworks();
    std::sort(snetworks.begin(), snetworks.end(), [](WiFiNetworkDef i, WiFiNetworkDef j) { return (i.rssi > j.rssi); });

    listNetworks(confnets, cnetworks, true);
    listNetworks(discnets, snetworks, false);

    sendPage(req, HEAD, TITLE, "WiFi Networks", END_TITLE, STYLES,
             END_HEAD,
             BODY,
             DIV_HEADER, BUTTON_HOME, BUTTON_RESET, BUTTON_UPDATE, END_DIV,
             DIV_CONTENT,
             "<BR><B>WiFi Configuration: ", config[controllername_n].c_str(),
             "</B>",
             "<FORM id=theform method=post action=/config.net>",
             TABLE,
             "<TR><TH colspan=2>Configured Networks</TH></TR>",
             confnets.c_str(),
             "<TR><TD>+</td><td><input name=newnet /></td></tr>",
             END_TABLE,
             TABLE,
             "<TR><TH colspan=2>Discovered Networks</TH></TR>",
             discnets.c_str(),
             END_TABLE,
             "</FORM>",
             END_DIV,
             END_BODY,
             NULL);
}

void P2WebServer::newNetPage(AsyncWebServerRequest *req)
{
    WiFiNetworkDef net("");
    net.openNet = true;

    if (req->method() == HTTP_POST)
    {

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
    }
    serr.printf("Edited network %s\n", net.ssid.c_str());
    updateWiFiDef(net);

    sendPage(req, HEAD, TITLE, "WiFi Network", END_TITLE, STYLES,
             END_HEAD,
             BODY,
             DIV_HEADER, BUTTON_HOME, BUTTON_WIFI, BUTTON_RESET, END_DIV,
             DIV_CONTENT,
             "<br><B>WiFi Network Edit: ",
             config[controllername_n].c_str(),
             "</B><br><br>", net.ssid.c_str(), " Updated",
             END_DIV,
             END_BODY,
             NULL);
}

void P2WebServer::netEditPage(AsyncWebServerRequest *req)
{
    String ssid;
    for (int i = 0; i < req->args(); i++)
    {
        if (req->argName(i) == "ssid")
        {
            ssid = req->arg(i);
            break;
        }
    }

    sendPage(req, HEAD, TITLE, "WiFi Network", END_TITLE, STYLES,
             END_HEAD,
             BODY,
             DIV_HEADER, BUTTON_HOME, BUTTON_WIFI, BUTTON_RESET, BUTTON_UPDATE, END_DIV,
             DIV_CONTENT,
             "<br><B>WiFi Network Edit: ", config[controllername_n].c_str(), "</B>",
             "<FORM id=theform method=post action=", pageWiFiNetAdd, ">",
             TABLE,
             "<tr><td>SSID:</td><td><INPUT name=ssid value=", ssid.c_str(), "/></td></tr>",
             "<tr><td>PSK:</td><td><INPUT type=password name=psk value=\"\"/></td></tr>",
             END_TABLE,
             "</FORM>",
             END_DIV,
             END_BODY,
             NULL);
}

void P2WebServer::systemUpdatePage(AsyncWebServerRequest *req)
{
    sendPage(req, HEAD, TITLE, "Punkatoo System Update", END_TITLE, STYLES,
             END_HEAD,
             BODY,
             DIV_HEADER, BUTTON_HOME, BUTTON_RESET, BUTTON_UPDATE, END_DIV,
             DIV_CONTENT,
             "<br><B>System Update: ", config[controllername_n].c_str(), "</B>",
             "<FORM id=theform method=post action=", pageDoUpdate, ">",
             TABLE,
             "<tr><td><label for=server>Update server:</label></td><td><input type=text name=server value=\"", config[mqtthost_n].c_str(), "\"></td></tr>",
             "<tr><td><label for=port>Port:</label></td><td><input type=text name=port value=80></td></tr>",
             "<tr><td><label for=image>Source file:</label></td><td><input type=text name=image value=\"/bin/punkatoo.bin\"></td></tr>",
             "<tr><td><label for=target>Target file:</label></td><td><input type=text name=target value=\"\"></td></tr>",
             END_TABLE,
             "</FORM>",
             END_DIV,
             END_BODY,
             NULL);
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
        pThis->event("progress", (String(progress) + "%").c_str());
        oldPhase = phase;
    }
}

void P2WebServer::updateInfo(const char *message, void *data)
{
    if (message != NULL)
    {
        pThis->event("progress", message);
    }
    else
    {
        pThis->event("progress", "No Message");
    }
}

void P2WebServer::doUpdatePage(AsyncWebServerRequest *req)
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
    dev.updater.onEnd(updateInfo);
    dev.updater.onFail(updateInfo);
    dev.updater.onProgress(progressCB);
    dev.updater.setRemote(Updater::UPD_SYS, server, port, image, image);

    sendPage(req, HEAD, TITLE, "Punkatoo", END_TITLE, STYLES,
             END_HEAD,
             BODY,
             DIV_HEADER, "<span id=buttons></span>", END_DIV,
             DIV_CONTENT,
             "<br><B>System Updating: ", config[controllername_n].c_str(), "</B>",
             R"+++(<BR><BR>
Progress: <span id="progress"></span>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);

 source.addEventListener('progress', function(e) {
  console.log("progress", e.data);
  if (e.data == "OK") {
  document.getElementById("buttons").innerHTML = ")+++",
             BUTTON_HOME, BUTTON_RESET,
             R"+++(";
  document.getElementById("progress").innerHTML = "Update Available, reset device to load";
  }
  else {
    document.getElementById("progress").innerHTML = e.data;
  }
 }, false);
}
 </script>)+++",
             END_DIV,
             END_BODY,
             NULL);
}

void P2WebServer::doUpdateConfPage(AsyncWebServerRequest *req, const char *server, const int port, const char *src, const char *target)
{
    dev.updater.onEnd(updateInfo);
    dev.updater.onFail(updateInfo);
    dev.updater.onProgress(progressCB);
    dev.updater.setRemote(Updater::UPD_CONF, server, port, src, target);

    sendPage(req, HEAD, TITLE, "Punkatoo", END_TITLE, STYLES,
             END_HEAD,
             BODY,
             DIV_HEADER, BUTTON_HOME, BUTTON_RESET, BUTTON_UPDATE, END_DIV,
             DIV_CONTENT,
             "<br><B>System Updating: ", config[controllername_n].c_str(), "</B>",
             "<BR><BR>File: ", target, "<BR>Progress: <span id=\"progress\"></span>",
             R"!(<script src="/eventlstnrs.js"></script>)!",
             R"+++(<script>{
 source.addEventListener('progress', function(e) {
  console.log("progress", e.data);
  document.getElementById("progress").innerHTML = e.data;
 }, false);
}</script>)+++",
             END_DIV,
             END_BODY,
             NULL);
}

void P2WebServer::event(const char *name, const char *content)
{
    events->send(content, name, eventid++);
}