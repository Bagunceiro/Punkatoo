
#include <LITTLEFS.h>
#include <HTTPClient.h>
#include "config.h"
#include "p2ws.h"
#include "devices.h"

P2WebServer *P2WebServer::pThis;
// AsyncEventSource events("/events");

const char *P2WebServer::pageRoot = "/";
const char *P2WebServer::pageGen = "/config.gen";
const char *P2WebServer::pageGenUpdate = "/config.update";
const char *P2WebServer::pageWiFi = "/config.net";
const char *P2WebServer::pageWiFiNet = "/config.netedit";
const char *P2WebServer::pageWiFiNetAdd = "/config.addnet";
const char *P2WebServer::pageReset = "/reset";
const char *P2WebServer::pageSystemUpdate = "/system.update";
const char *P2WebServer::pageDoUpdate = "/system.update.do";


void P2WebServer::init()
{
    // HTTP_ANY for now. should be HTTP_GET etc
    on(pageRoot, HTTP_ANY, handleRoot);
    on(pageGen, HTTP_ANY, handleGenConfig);
    on(pageGenUpdate, HTTP_ANY, handleGenUpdate);
    on(pageWiFi, HTTP_ANY, handleNetConfig);
    on(pageWiFiNet, HTTP_ANY, handleNetEdit);
    on(pageWiFiNetAdd, HTTP_ANY, handleNewNet);
    on(pageReset, HTTP_ANY, blankResetMessage);
    on(pageSystemUpdate, HTTP_ANY, handleSystemUpdate);
    on(pageDoUpdate, HTTP_ANY, handleDoUpdate);
    on("/punkatoo.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LITTLEFS, "/punkatoo.css", "text/css");
    });
    on("/navfuncs.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LITTLEFS, "/navfuncs.js", "application/javascript");
    });
    on("/eventlstnrs.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LITTLEFS, "/eventlstnrs.js", "application/javascript");
    });

    events->onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId())
        {
            serr.println("Client reconnected");
        }
    });
    addHandler(events);

    begin();
}

void P2WebServer::sendPage(AsyncWebServerRequest *req, const int noItems, ...)
{
    va_list args;
    va_start(args, noItems);

    AsyncResponseStream *response = req->beginResponseStream("text/html");
    for (int i = 0; i < noItems; i++)
    {
        char *s = va_arg(args, char *);
        response->print(s);
    }

    va_end(args);
    req->send(response);
}

const String P2WebServer::head1 = R"!(
<HEAD><meta http-equiv="content-type" charset="UTF-8"
content="text/html, width=device-width, initial-scale=1">
<TITLE>
)!";

const String P2WebServer::head2 = R"!(
</TITLE>
<script src="/navfuncs.js"></script>
<link rel="stylesheet" href="/punkatoo.css">
)!";


/*
const String P2WebServer::head2("</TITLE>");

const String P2WebServer::style = R"!(
<script src="/navfuncs.js"></script>
<link rel="stylesheet" href="/punkatoo.css">
)!";
*/

const String P2WebServer::headEnd(R"!(
</HEAD>
)!");

const String P2WebServer::body1(R"!(
<BODY>
<div class="header" id="myHeader">
<button onclick="gohome()">Home</button>
<button onclick=gogenconf()>General</button>
<button onclick=gowificonf()>WiFi</button>
)!");

void P2WebServer::rootPage(AsyncWebServerRequest *req)
{
    const String title("Punkatoo");
    const String head3("");

    String body2(R"!(
<button onclick=goreset()>Reset</button>
<button onclick=gosysupdate()>System Update</button>
</div>
<div class=content>
<BR><B>Controller: )!" +
                 config[controllername_n] + R"!(</B>
<TABLE>
<TR><TD>Time Now</TD><TD colspan=3 ><span id="nowtime">)!" +
                 nowTime() + R"!(</span></TD></TR>
<TR><TD>Git Revision</TD><TD colspan=3 >)!" +
                 gitrevision + R"!(</TD></TR>
<TR><TD>Compilation</TD><TD colspan=3 >)!" +
                 compDateTime + R"!(</TD></TR>
<TR><TD>MAC Address</TD><TD colspan=3 >)!" +
                 WiFi.macAddress() + R"!(</TD></TR>
<TR><TD>Started At</TD><TD colspan=3 >)!" +
                 startTime() + R"!(</TD></TR>
<TR><TD>Uptime</TD><TD colspan=3><span id="uptime">)!" +
                 upTime() + R"!(</span></TD></TR>
<TR><TD>WiFi SSID</TD><TD colspan=3>)!" +
                 WiFi.SSID() + R"!(</TD></TR>
<TR><TD>Ambient</TD><TD><span id="temperature"></span>°C</TD>
<TD><span id="humidity"></span>%</TD>
<TD><span id="pressure"></span> mBar</TD></TR>
<TR><TD>Light level</TD><TD colspan=3><span id="light"></TD></TR>
</TABLE><BR>
</DIV>
<script src="/eventlstnrs.js"></script>
</BODY>
)!");
    sendPage(req, 7,
             head1.c_str(),
             title.c_str(),
             head2.c_str(),
             head3.c_str(),
             headEnd.c_str(),
             body1.c_str(),
             body2.c_str());
}

void P2WebServer::messagePage(AsyncWebServerRequest *req, const String &message)
{
    const String title("Punkatoo Message");
    const String head3 = "<meta http-equiv=\"refresh\" content=\"15;url=/\" />";
    String body2(R"!(
</div>
  <div class=content>
  <br><B>Controller: )!" +
                 config[controllername_n] + "</B><br><br>" + message + R"!(
  </div>
  </BODY>
  )!");

    sendPage(req, 7,
             head1.c_str(),
             title.c_str(),
             head2.c_str(),
             head3.c_str(),
             headEnd.c_str(),
             body1.c_str(),
             body2.c_str());
    delay(1000);
}

void P2WebServer::resetMessagePage(AsyncWebServerRequest *req, const String &reason)
{
    messagePage(req, reason + "<br><br>Resetting, please wait");
    dev.p2sys.reset();
}

void P2WebServer::blankResetMessagePage(AsyncWebServerRequest *req)
{
    resetMessagePage(req, "");
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

        // indicator.setColour(indicate_update, true);

        messagePage(req, "Configuration Updated");
    }
}

void P2WebServer::genConfigPage(AsyncWebServerRequest *req)
{
    String title("Punkatoo Configuration");
    String head3("");
    String body2((String) R"!(
<button type=submit form=theform>Save</button>
</div>
<div class=content>
<BR><B>General Configuration: )!" +
                 config[controllername_n] + R"!(</B>
<FORM id=theform method=post action=")!" +
                 pageGenUpdate + R"!(")>
<table>
<tr><td><label for=ctlrname>Controller Name:</label></td>
<td><input type=text name=")!" +
                 controllername_n + "\" value=\"" + config[controllername_n] + R"!("></td></tr>
<tr><td><label for=mqtthost>MQTT Broker:</label></td>
<td><input type=text name=")!" +
                 mqtthost_n + "\" value=\"" + config[mqtthost_n] + R"!("></td></tr>
<tr><td><label for=mqttport>MQTT Port:</label></td>
<td><input type=text name=")!" +
                 mqttport_n + "\" value=\"" + config[mqttport_n] + R"!("></td></tr>
<tr><td><label for=mqttuser>MQTT User:</label></td>
<td><input type=text name=")!" +
                 mqttuser_n + "\" value=\"" + config[mqttuser_n] + R"!("></td></tr>
<tr><td><label for=mqttuser>MQTT Password:</label></td>
<td><input type=text name=")!" +
                 mqttpwd_n + "\" value=\"" + config[mqttpwd_n] + R"!("></td></tr>
<tr><td><label for=mqttroot>MQTT Topic root:</label></td>
<td><input type=text name=")!" +
                 mqttroot_n + "\" value=\"" + config[mqttroot_n] + R"!("></td></tr>
<tr><td><label for=mqtttopic>MQTT Topic:</label></td>
<td><input type=text name=")!" +
                 mqtttopic_n + "\" value=\"" + config[mqtttopic_n] + R"!("></td></tr>
<tr><td><label for="ind">Status Indicator:</label></td>
<td><input type=checkbox id="ind" name=")!" +
                 indicator_n + "\"" + (config[indicator_n] == "1" ? " checked" : "") + R"!(/><label for=ind>&nbsp;</label></td></tr>
</table>
</FORM>
</div>
</BODY>
)!");

    sendPage(req, 7,
             head1.c_str(),
             title.c_str(),
             head2.c_str(),
             head3.c_str(),
             headEnd.c_str(),
             body1.c_str(),
             body2.c_str());
}

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
                    // serr.printf("newnet, value = %s\n", value.c_str());
                    if (value.length() != 0)
                    {
                        addNetwork(newlist, value);
                    }
                }
            }
        }
        networkConfWrite(newlist);
    }
    networkList &cnetworks = networkConfRead();
    String title("WiFi Networks");
    String head3("");
    String body2(String(R"=====(
<button type=submit form=theform>Update</button>
</div>
<div class=content>
<BR><B>WiFi Configuration: )=====") +
                 String(config[controllername_n]) + String(R"=====(</B>
<FORM id=theform method=post action=/config.net>
<TABLE>
<TR><TH colspan=2>Configured Networks</TH></TR>
)====="));

    listNetworks(body2, cnetworks, true);
    body2 += R"=====(
<TR><TD>+</td><td><input name=newnet /></td></tr>
</TABLE>
<TABLE>
<TR><TH colspan=2>Discovered Networks</TH></TR>
)=====";
    networkList &snetworks = scanNetworks();
    //  std::sort(snetworks.begin(), snetworks.end(), sortByRSSI);
    std::sort(snetworks.begin(), snetworks.end(), [](WiFiNetworkDef i, WiFiNetworkDef j) { return (i.rssi > j.rssi); });
    listNetworks(body2, snetworks, false);
    body2 += R"=====(
  </TABLE></FORM></DIV>
  </BODY>
  )=====";

    sendPage(req, 7,
             head1.c_str(),
             title.c_str(),
             head2.c_str(),
             head3.c_str(),
             headEnd.c_str(),
             body1.c_str(),
             body2.c_str());
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
    String title("WiFi Network");
    String head3("");
    String body2(R"====(
</div>
<div class=content>
<br><B>WiFi Network Edit: )====" +
                 config[controllername_n] + R"====(</B><br><br>
)====" + net.ssid +
                 R"====( Updated
</div>
</BODY>
)====");

    sendPage(req, 7,
             head1.c_str(),
             title.c_str(),
             head2.c_str(),
             head3.c_str(),
             headEnd.c_str(),
             body1.c_str(),
             body2.c_str());
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
    String title("WiFi Network");
    String head3("");
    String body2;
    body2 = (String) R"=====(
<button type=submit form=theform>Update</button>
</div>
<div class=content>
<BR><B>WiFi Network Edit: ")=====" +
            config[controllername_n] + R"=====("</B>
<FORM id=theform method=post action=)=====" +
            pageWiFiNetAdd + R"=====(>
<table>
<tr>
<td>SSID:</td>
<td><INPUT name=ssid value=")=====" +
            ssid + R"====("/></td>
</tr>
<tr>
<td>PSK:</td>
<td><INPUT type=password name=psk value=""/></td>
</tr>
</table>
</FORM>
</div>
</BODY>
)====";

    sendPage(req, 7,
             head1.c_str(),
             title.c_str(),
             head2.c_str(),
             head3.c_str(),
             headEnd.c_str(),
             body1.c_str(),
             body2.c_str());
}

void P2WebServer::systemUpdatePage(AsyncWebServerRequest *req)
{
    String title("Punkatoo System Update");
    String head3("");
    String body2((String) R"=====(
<button type=submit form=theform>Update</button>
</div>
<div class=content>
<BR><B>System Update: )=====" +
                 config[controllername_n] + R"=====(</B>
<FORM id=theform method=post action=")=====" +
                 pageDoUpdate + R"=====(")>
<table>
<tr><td><label for=server>Update server:</label></td>
<td><input type=text name=server value=")=====" +
                 config[mqtthost_n] + R"=====("></td></tr>
<tr><td><label for=port>Port:</label></td>
<td><input type=text name=port value=80></td></tr>
<tr><td><label for=image>Update image file:</label></td>
<td><input type=text name=image value="/bin/punkatoo.bin"></td></tr>
</table>
</FORM>
</div>
</BODY>
)=====");
    sendPage(req, 7,
             head1.c_str(),
             title.c_str(),
             head2.c_str(),
             head3.c_str(),
             headEnd.c_str(),
             body1.c_str(),
             body2.c_str());
}

void P2WebServer::progressCB(size_t completed, size_t total, void * data)
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

void P2WebServer::updateCompleted(void *)
{
    Serial.println("updateCompleted called");
    pThis->event("progress", "Complete. Reset device to load");
}

void P2WebServer::doUpdatePage(AsyncWebServerRequest *req)
{
    const String title("Punkatoo");
    String server;
    String port;
    String image;

    for (int i = 0; i < req->args(); i++)
    {
        const String argN = req->argName(i);
        if (argN == "server")
            server = req->arg(i);
        else if (argN == "port")
            port = req->arg(i);
        else if (argN == "image")
            image = req->arg(i);
    }

    /*
    String ext;
    int extidx = image.lastIndexOf('.');
    if (extidx >= 0)
        ext = image.substring(extidx + 1);
*/
    serr.printf("Update %s, %ld, %s\n", server.c_str(), port.toInt(), image.c_str());

    dev.updater.onEnd(updateCompleted);
    dev.updater.onProgress(progressCB);
    dev.updater.setRemote(server, port.toInt(), image, Updater::UPD_SYS);

    String body2((String) R"=====(
</div>
<div class=content>
<BR><BR><B>System Updating: )=====" +
                 config[controllername_n] + R"=====(</B>
<BR><BR>
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
  document.getElementById("progress").innerHTML = e.data;
 }, false);
}
 </script>
</BODY>
)=====");
    String head3("");

    sendPage(req, 7,
             head1.c_str(),
             title.c_str(),
             head2.c_str(),
             head3.c_str(),
             headEnd.c_str(),
             body1.c_str(),
             body2.c_str());
}

void P2WebServer::event(const char *name, const char *content)
{
    events->send(content, name, eventid++);
}

void P2WebServer::event(const char *name, const long content)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer) - 1, "%ld", content);
    event(name, buffer);
}

void P2WebServer::event(const char *name, const double content)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer) - 1, "%.2lf", content);
    event(name, buffer);
}