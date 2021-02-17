#include <Arduino.h>
#include "config.h"
#include "webserver.h"
#include "networks.h"
#include "updater.h"
#include "ldr.h"
#include "tempSensor.h"

extern BMESensor bme;

FanConWebServer webServer(80);

const String pageRoot         = "/";
const String pageGen          = "/config.gen";
const String pageGenUpdate    = "/config.update";
const String pageWiFi         = "/config.net";
const String pageWiFiNet      = "/config.netedit";
const String pageWiFiNetAdd   = "/config.addnet";
const String pageReset        = "/reset";
const String pageSystemUpdate = "/system.update";
const String pageDoUpdate     = "/system.update.do";

const String style = R"!(
<script>
function gohome()      {location.assign(")!" + pageRoot + R"!(");}
function gogenconf()   {location.assign(")!" + pageGen  + R"!(");}
function gowificonf()  {location.assign(")!" + pageWiFi + R"!(");}
function goreset()     {location.assign(")!" + pageReset + R"!(");}
function gosysupdate() {location.assign(")!" + pageSystemUpdate + R"!(");}
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

const String head1(R"!(
<HEAD><meta http-equiv="content-type" charset="UTF-8"
content="text/html, width=device-width, initial-scale=1">
<TITLE>
)!");

const String head2("</TITLE>");
const String headEnd(R"!(
</HEAD>
)!");

const String body1(R"!(
<BODY>
<div class="header" id="myHeader">
<button onclick="gohome()">Home</button>
<button onclick=gogenconf()>General</button>
<button onclick=gowificonf()>WiFi</button>
-)!");

const String tail("");

void sendPage(const String &s1,
              const String &s2,
              const String &s3,
              const String &s4,
              const String &s5,
              const String &s6,
              const String &s7,
              const String &s8)
{
  int contentLength = s1.length() + s2.length() + s3.length() + s4.length() + s5.length() + s6.length() + s7.length() + s8.length() + tail.length();

  webServer.setContentLength(contentLength);
  webServer.send(200, "text/html", s1);
  webServer.sendContent(s2);
  webServer.sendContent(s3);
  webServer.sendContent(s4);
  webServer.sendContent(s5);
  webServer.sendContent(s6);
  webServer.sendContent(s7);
  webServer.sendContent(s8);
  webServer.sendContent(tail);
}

String tempSensorData()
{
  String data;
  if (bme.running())
  {
/*
    data = 
R"!(<TR><TD>Temperature</TD><TD>)!"   + String((tempSensor.readTemperature() *10)/10) + R"!(°C</TD></TR>
<TR><TD>Humidity</TD><TD>)!"          + String((tempSensor.readHumidity() *10)/10) + R"!(%</TD></TR>
<TR><TD>Pressure</TD><TD>)!"          + String((tempSensor.readPressure() *10)/1000) + R"!( mBar</TD></TR>
)!";
*/
    data = 
R"!(<TR><TD>Ambient</TD><TD>)!" + String((bme.readTemperature() *10)/10) + R"!(°C</TD>
<TD>)!" + String((bme.readHumidity() *10)/10) + R"!(%</TD>
<TD>)!" + String((bme.readPressure() *10)/1000) + R"!( mBar</TD></TR>
)!";

  }
  return data;
}


#include "lamp.h"
/*
extern Lamp lamp;
<TR><TD>Light Switch</TD><TD colspan=3>)!"  + (lamp.switchstate() ? "Open" : "Closed") + R"!(</TD></TR>
*/

void handleRoot()
{
  const String title("Punkatoo");
  const String head3("");
  time_t now = timeClient.getEpochTime();
  time_t lastUpdate = persistant[persistant.updateTime_n].toInt();
  extern LDR ldr;
  extern Lamp lamp;

  String body2(R"!(
<button onclick=goreset()>Reset</button>
<button onclick=gosysupdate()>System Update</button>
</div>
<div class=content>
<BR><B>Controller: )!" + persistant[persistant.controllername_n] + R"!(</B>
<TABLE>
<TR><TD>Time now</TD><TD colspan=3 >)!"      + ctime(&now) + R"!(</TD></TR>
)!" + tempSensorData() + R"!(
<TR><TD>Light level</TD><TD colspan=3>)!" + ldr.getStatus() + R"!(</TD></TR>
<TR><TD>Version</TD><TD colspan=3 >)!"       + appVersion + " (" + compTime + " " + compDate + R"!()</TD></TR>
<TR><TD>MAC Address</TD><TD colspan=3 >)!"   + WiFi.macAddress() + R"!(</TD></TR>
<TR><TD>Last update</TD><TD colspan=3 >)!"   + (lastUpdate != 0 ? ctime(&lastUpdate) : "N/A") + R"!(</TD></TR>
<TR><TD>Uptime</TD><TD colspan=3>)!"        + upTime() + R"!(</TD></TR>
<TR><TD>WiFi SSID</TD><TD colspan=3>)!"     + WiFi.SSID() + R"!(</TD></TR>
<TR><TD>Light Switch</TD><TD colspan=3>)!"  + (lamp.switchstate() ? "Open" : "Closed") + R"!(</TD></TR>
</TABLE><BR>
</DIV>
</BODY>
)!");

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

void messagePage(const String& message)
{
  const String title("Punkatoo Message");
  const String head3 = "<meta http-equiv=\"refresh\" content=\"15;url=/\" />";
  String body2(R"!(
  <div class=content>
  <BODY>
  <br><B>Controller: )!" + persistant[persistant.controllername_n] + "</B><br><br>" + message + R"!(
  </div>
  </BODY>
  )!");

  sendPage(head1, title, head2, style, head3, headEnd, "", body2);
    delay(1000);
}

void resetMessage(const String& reason)
{
  messagePage(reason + "<br><br>Resetting, please wait");
  delay(1000);
  #ifdef ESP32
  ESP.restart();
  #else
  ESP.reset();
  #endif
}

void blankResetMessage()
{
  resetMessage("");
}

void handleGenUpdate()
{
  if (webServer.method() == HTTP_POST)
  {

    for (uint8_t i = 0; i < webServer.args(); i++)
    {
      const String argName = webServer.argName(i);
      persistant[argName] = webServer.arg(i);
    }
    persistant.dump(serr);
    persistant.writeFile();
    serr.println("Resetting");

    delay(3000);
    resetMessage("");
  }
}

void handleGenConfig()
{
  String title("Punkatoo Configuration");
  String head3("");
  String body2((String) R"!(
<button type=submit form=theform>Save and Reset</button>
</div>
<div class=content>
<BR><B>General Configuration: )!" + persistant[persistant.controllername_n] + R"!(</B>
<FORM id=theform method=post action=")!" + pageGenUpdate + R"!(")>
<table>
<tr><td><label for=ctlrname>Controller Name:</label></td>
<td><input type=text name=")!" + persistant.controllername_n + "\" value=\"" + persistant[persistant.controllername_n] + R"!("></td></tr>
<tr><td><label for=mqtthost>MQTT Broker:</label></td>
<td><input type=text name=")!" + persistant.mqtthost_n + "\" value=\"" + persistant[persistant.mqtthost_n] + R"!("></td></tr>
<tr><td><label for=mqttport>MQTT Port:</label></td>
<td><input type=text name=")!" + persistant.mqttport_n + "\" value=\"" + persistant[persistant.mqttport_n] + R"!("></td></tr>
<tr><td><label for=mqttuser>MQTT User:</label></td>
<td><input type=text name=")!" + persistant.mqttuser_n + "\" value=\"" + persistant[persistant.mqttuser_n] + R"!("></td></tr>
<tr><td><label for=mqttuser>MQTT Password:</label></td>
<td><input type=text name=")!" + persistant.mqttpwd_n + "\" value=\"" + persistant[persistant.mqttpwd_n] + R"!("></td></tr>
<tr><td><label for=mqttroot>MQTT Topic root:</label></td>
<td><input type=text name=")!" + persistant.mqttroot_n + "\" value=\"" + persistant[persistant.mqttroot_n] + R"!("></td></tr>
<tr><td><label for=mqtttopic>MQTT Topic:</label></td>
<td><input type=text name=")!" + persistant.mqtttopic_n + "\" value=\"" + persistant[persistant.mqtttopic_n] + R"!("></td></tr>
</table>
</FORM>
</div>
</BODY>
)!");

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

String &listNetworks(String &body, networkList &networks, bool selected)
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
            String(networks[i].openNet ? "🔓" : "🔒") + (selected ? (String(" <a href=\"") + pageWiFiNet + "?ssid=" + networks[i].ssid + "\">") : "") + networks[i].ssid + String(selected ? "</a>" : "") + String(R"=====(
</td>
</tr>
)=====");
  }
  return body;
}

bool sortByRSSI(WiFiNetworkDef i, WiFiNetworkDef j)
{
  return (i.rssi > j.rssi);
}

void handleNetConfig()
{
  if (webServer.method() == HTTP_POST)
  {
    serr.println("Network Update");
    networkList newlist;

    bool usenext = false;
    for (uint8_t i = 0; i < webServer.args(); i++)
    {
      const String argName = webServer.argName(i);
      const String value = webServer.arg(i);

      if (usenext && (argName == "ssid"))
      {
        // serr.println(value);
        addNetwork(newlist, value);
      }
      // serr.println(argName.c_str());
      if (argName == "conf")
        usenext = true;
      else
      {
        usenext = false;
        if (argName == "newnet")
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
               String(persistant[persistant.controllername_n]) + String(R"=====(</B>
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
  std::sort(snetworks.begin(), snetworks.end(), sortByRSSI);
  listNetworks(body2, snetworks, false);
  body2 += R"=====(
  </TABLE></FORM></DIV>
  </BODY>
  )=====";

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

void handleNewNet()
{
  WiFiNetworkDef net("");
  net.openNet = true;

  if (webServer.method() == HTTP_POST)
  {

    for (uint8_t i = 0; i < webServer.args(); i++)
    {
      const String argName = webServer.argName(i);

      if (argName == "ssid")
      {
        net.ssid = webServer.arg(i);
      }
      else if (argName == "psk")
      {
        net.psk = webServer.arg(i);
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
               persistant[persistant.controllername_n] + R"====(</B><br><br>
)====" + net.ssid +
               R"====( Updated
</div>
</BODY>
)====");

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

void handleNetEdit()
{
  String ssid;
  for (int i = 0; i < webServer.args(); i++)
  {
    if (webServer.argName(i) == "ssid")
    {
      ssid = webServer.arg(i);
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
          persistant[persistant.controllername_n] + R"=====("</B>
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

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}



void handleSystemUpdate()
{
  String title("Punkatoo System Update");
  String head3("");
  String body2((String) R"=====(
<button type=submit form=theform>Update</button>
</div>
<div class=content>
<BR><B>System Update: )=====" +
               persistant[persistant.controllername_n] + R"=====(</B>
<FORM id=theform method=post action=")=====" +
               pageDoUpdate + R"=====(")>
<table>
<tr><td><label for=server>Update server:</label></td>
<td><input type=text name=server value=")=====" + persistant[persistant.mqtthost_n] + R"=====("></td></tr>
<tr><td><label for=port>Port:</label></td>
<td><input type=text name=port value=80></td></tr>
<tr><td><label for=image>Update image file:</label></td>
<td><input type=text name=image value=""></td></tr>
</table>
</FORM>
</div>
</BODY>
)=====");
  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

void handleDoUpdate()
{
  const String title("Punkatoo");
  String server;
  String port;
  String image;

  for (int i = 0; i < webServer.args(); i++)
  {
    const String argName = webServer.argName(i);
    if (argName == "server") server = webServer.arg(i);
    else if (argName == "port") port = webServer.arg(i);
    else if (argName == "image") image = webServer.arg(i);
  }
  
  Updater updater("updater");
  
  t_httpUpdate_return ret = updater.systemUpdate(server, port.toInt(), image);

  switch (ret) {
      case HTTP_UPDATE_FAILED:
        messagePage("Update Failed");
        break;
      case HTTP_UPDATE_NO_UPDATES:
        messagePage("No Update Available");
        break;
      case HTTP_UPDATE_OK:
        resetMessage("Update Successful");
        break;
      default:
        messagePage("Default case");
    }
}


void FanConWebServer::init()
{
  webServer.on(pageRoot,         handleRoot);
  webServer.on(pageGen,          handleGenConfig);
  webServer.on(pageGenUpdate,    handleGenUpdate);
  webServer.on(pageWiFi,         handleNetConfig);
  webServer.on(pageWiFiNet,      handleNetEdit);
  webServer.on(pageWiFiNetAdd,   handleNewNet);
  webServer.on(pageReset,        blankResetMessage);
  webServer.on(pageSystemUpdate, handleSystemUpdate);
  webServer.on(pageDoUpdate,     handleDoUpdate);

  serr.println("Web Server");
  webServer.begin();
}
