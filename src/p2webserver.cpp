#include <Arduino.h>
#include <LITTLEFS.h>
#include <HTTPClient.h>
#include "config.h"
#include "p2webserver.h"
#include "devices.h"

P2WebServer *P2WebServer::pThis;

const String P2WebServer::pageRoot = "/";
const String P2WebServer::pageGen = "/config.gen";
const String P2WebServer::pageGenUpdate = "/config.update";
const String P2WebServer::pageWiFi = "/config.net";
const String P2WebServer::pageWiFiNet = "/config.netedit";
const String P2WebServer::pageWiFiNetAdd = "/config.addnet";
const String P2WebServer::pageReset = "/reset";
const String P2WebServer::pageSystemUpdate = "/system.update";
const String P2WebServer::pageDoUpdate = "/system.update.do";

const String P2WebServer::style = R"!(
<script>
function gohome()      {location.assign(")!" +
                                  P2WebServer::pageRoot + R"!(");}
function gogenconf()   {location.assign(")!" +
                                  P2WebServer::pageGen + R"!(");}
function gowificonf()  {location.assign(")!" +
                                  P2WebServer::pageWiFi + R"!(");}
function goreset()     {location.assign(")!" +
                                  P2WebServer::pageReset + R"!(");}
function gosysupdate() {location.assign(")!" +
                                  P2WebServer::pageSystemUpdate + R"!(");}
</script>
<style>
body {font-family:Arial, Sans-Serif; font-size: 4vw; background-color: lavender;}
table {font-family: arial, sans-serif; border-collapse: collapse; margin-left: 5%%; width: 100%%;}
th, td, input {font-size: 4vw; border: 1px solid #dddddd;white-space:nowrap; text-align: left; padding: 8px; }
tr:nth-child(even) { background-color: thistle;}
button {font-size: 3vw; background: indigo; color: white; }
input[type=checkbox] {display:none;}
input[type=checkbox] + label:before {content:"☐  ";}
input:checked + label:before {content:"☑  ";}
.sticky {
  position: fixed;
  top: 0;
  width: 100%;
}
.header {
  position: fixed;
  top: 0;
  width: 100%;}
</style>
)!";

const String P2WebServer::head1(R"!(
<HEAD><meta http-equiv="content-type" charset="UTF-8"
content="text/html, width=device-width, initial-scale=1">
<TITLE>
)!");

const String P2WebServer::head2("</TITLE>");
const String P2WebServer::headEnd(R"!(
</HEAD>
)!");

const String P2WebServer::body1(R"!(
<BODY>
<div class="header" id="myHeader">
<button onclick="gohome()">Home</button>
<button onclick=gogenconf()>General</button>
<button onclick=gowificonf()>WiFi</button>
-)!");

void P2WebServer::sendPage(const int noItems, ...)
{
  va_list args;
  va_start(args, noItems);

  int contentLength = 0;
  for (int i = 0; i < noItems; i++)
  {
    char *s = va_arg(args, char *);
    contentLength += strlen(s);
  }

  va_end(args);

  setContentLength(contentLength);
  va_start(args, noItems);
  send(200, "text/html", va_arg(args, char *));
  for (int i = 1; i < noItems; i++)
  {
    sendContent(va_arg(args, char *));
  }
}

const String P2WebServer::bmeData()
{
  String data;

  for (BME &bme : dev.bmes)
  {
    if (bme.running())
    {
      data +=
          R"!(<TR><TD>Ambient</TD><TD>)!" + String((bme.readTemperature() * 10) / 10) + R"!(°C</TD>
<TD>)!" +
          String((bme.readHumidity() * 10) / 10) + R"!(%</TD>
<TD>)!" +
          String((bme.readPressure() * 10) / 1000) + R"!( mBar</TD></TR>
)!";
    }
  }
  return data;
}

const String P2WebServer::lightLevels()
{
  String s;
  for (LDR &ldr : dev.ldrs)
  {
    s += "<TR><TD>Light level</TD><TD colspan=3>" + ldr.mqttGetStatus() + "</TD></TR>\n";
  }
  return s;
}

void P2WebServer::rootPage()
{
  const String title("Punkatoo");
  const String head3("");
  // time_t now = timeClient.getEpochTime();
  // time_t lastUpdate = config[updateTime_n].toInt();
  // extern LDR ldr;
  // extern Lamp lamp;

  String body2(R"!(
<button onclick=goreset()>Reset</button>
<button onclick=gosysupdate()>System Update</button>
</div>
<div class=content>
<BR><B>Controller: )!" +
               config[controllername_n] + R"!(</B>
<TABLE>
<TR><TD>Time now</TD><TD colspan=3 >)!" +
               nowTime() + R"!(</TD></TR>
)!" + bmeData() +
               lightLevels() + R"!(
<TR><TD>Version</TD><TD colspan=3 >)!" +
               appVersion + " (" + compDate + " " + compTime + R"!()</TD></TR>
<TR><TD>MAC Address</TD><TD colspan=3 >)!" +
               WiFi.macAddress() + R"!(</TD></TR>
<TR><TD>Last OTA update</TD><TD colspan=3 >)!" +
               lastUpdateTime() + R"!(</TD></TR>
<TR><TD>Started At</TD><TD colspan=3 >)!" +
               startTime() + R"!(</TD></TR>
<TR><TD>Uptime</TD><TD colspan=3>)!" +
               upTime() + R"!(</TD></TR>
<TR><TD>WiFi SSID</TD><TD colspan=3>)!" +
               WiFi.SSID() + R"!(</TD></TR>
</TABLE><BR>
</DIV>
</BODY>
)!");

  sendPage(8,
           head1.c_str(),
           title.c_str(),
           head2.c_str(),
           style.c_str(),
           head3.c_str(),
           headEnd.c_str(),
           body1.c_str(),
           body2.c_str());
}

void P2WebServer::messagePage(const String &message)
{
  const String title("Punkatoo Message");
  const String head3 = "<meta http-equiv=\"refresh\" content=\"15;url=/\" />";
  String body2(R"!(
  <div class=content>
  <BODY>
  <br><B>Controller: )!" +
               config[controllername_n] + "</B><br><br>" + message + R"!(
  </div>
  </BODY>
  )!");

  sendPage(8,
           head1.c_str(),
           title.c_str(),
           head2.c_str(),
           style.c_str(),
           head3.c_str(),
           headEnd.c_str(),
           body1.c_str(),
           body2.c_str());
  delay(1000);
}

void P2WebServer::resetMessagePage(const String &reason)
{
  messagePage(reason + "<br><br>Resetting, please wait");
  delay(1000);
  ESP.restart();
}

void P2WebServer::blankResetMessagePage()
{
  resetMessagePage("");
}

void P2WebServer::genUpdatePage()
{
  if (method() == HTTP_POST)
  {
    config[indicator_n] = "0";

    for (uint8_t i = 0; i < args(); i++)
    {
      const String argN = argName(i);
      if (argN == indicator_n)
      {
        config[argN] = "1";
      }
      else
        config[argN] = arg(i);
    }
    config.dump(serr);
    config.writeFile();

    // indicator.setColour(indicate_update, true);

    messagePage("Configuration Updated");
  }
}

void P2WebServer::genConfigPage()
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

  sendPage(8,
           head1.c_str(),
           title.c_str(),
           head2.c_str(),
           style.c_str(),
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

void P2WebServer::netConfigPage()
{
  if (method() == HTTP_POST)
  {
    serr.println("Network Update");
    networkList newlist;

    bool usenext = false;
    for (uint8_t i = 0; i < args(); i++)
    {
      const String argN = argName(i);
      const String value = arg(i);

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

  sendPage(8,
           head1.c_str(),
           title.c_str(),
           head2.c_str(),
           style.c_str(),
           head3.c_str(),
           headEnd.c_str(),
           body1.c_str(),
           body2.c_str());
}

void P2WebServer::newNetPage()
{
  WiFiNetworkDef net("");
  net.openNet = true;

  if (method() == HTTP_POST)
  {

    for (uint8_t i = 0; i < args(); i++)
    {
      const String argN = argName(i);

      if (argN == "ssid")
      {
        net.ssid = arg(i);
      }
      else if (argN == "psk")
      {
        net.psk = arg(i);
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

  sendPage(8,
           head1.c_str(),
           title.c_str(),
           head2.c_str(),
           style.c_str(),
           head3.c_str(),
           headEnd.c_str(),
           body1.c_str(),
           body2.c_str());
}

void P2WebServer::netEditPage()
{
  String ssid;
  for (int i = 0; i < args(); i++)
  {
    if (argName(i) == "ssid")
    {
      ssid = arg(i);
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

  sendPage(8,
           head1.c_str(),
           title.c_str(),
           head2.c_str(),
           style.c_str(),
           head3.c_str(),
           headEnd.c_str(),
           body1.c_str(),
           body2.c_str());
}

void P2WebServer::systemUpdatePage()
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
  sendPage(8,
           head1.c_str(),
           title.c_str(),
           head2.c_str(),
           style.c_str(),
           head3.c_str(),
           headEnd.c_str(),
           body1.c_str(),
           body2.c_str());
}

void P2WebServer::doUpdatePage()
{
  const String title("Punkatoo");
  String server;
  String port;
  String image;

  for (int i = 0; i < args(); i++)
  {
    const String argN = argName(i);
    if (argN == "server")
      server = arg(i);
    else if (argN == "port")
      port = arg(i);
    else if (argN == "image")
      image = arg(i);
  }

  String ext;
  int extidx = image.lastIndexOf('.');
  if (extidx >= 0)
    ext = image.substring(extidx + 1);

  if (ext == "bin")
  {
    t_httpUpdate_return ret = dev.updater.systemUpdate(server, port.toInt(), image);

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      messagePage("Update Failed");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      messagePage("No Update Available");
      break;
    case HTTP_UPDATE_OK:
      resetMessagePage("Update Successful");
      break;
    default:
      messagePage("Default case");
    }
  }
  else if (ext == "json")
  {
    HTTPClient http;
    String url = "http://" + server + ":" + port + "/" + image;
    serr.printf("getting %s\n", url.c_str());
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      extern fs::LITTLEFSFS &LittleFS;
      LittleFS.begin();

      int baseidx = image.lastIndexOf('/');
      String fname = image.substring(baseidx + 1);
      serr.printf("Opening %s\n", fname.c_str());
      File configFile = LittleFS.open(String('/') + image.substring(baseidx + 1), "w+");
      if (!configFile)
      {
        perror("");
        serr.println("Config file open for write failed");
      }
      else
      {
        serr.printf("writing:\n %s", payload.c_str());
        configFile.print(payload);
      }
      configFile.close();
      LittleFS.end();
    }
    else
    {
      serr.printf("[HTTP] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    messagePage("Configuration file");
  }
  else
    messagePage("Unknown file type");
}

void P2WebServer::init()
{
  on(pageRoot, handleRoot);
  on(pageGen, handleGenConfig);
  on(pageGenUpdate, handleGenUpdate);
  on(pageWiFi, handleNetConfig);
  on(pageWiFiNet, handleNetEdit);
  on(pageWiFiNetAdd, handleNewNet);
  on(pageReset, blankResetMessage);
  on(pageSystemUpdate, handleSystemUpdate);
  on(pageDoUpdate, handleDoUpdate);

  begin();
}