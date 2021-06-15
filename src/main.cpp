#include <Arduino.h>
#include <FreeRTOS.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <time.h>
#include <esp_wps.h>
#include <TimeLib.h>
#include <LITTLEFS.h>

#include "wifiserial.h"
#include "config.h"
#include "devices.h"
#include "spdt.h"
#include "networks.h"
#include "cli.h"

WiFiSerialClient serr;
Devices dev;
CLITask clitask("CLI");
const char* const telegramBotToken = "1831001601:AAEIn25ouJXhznXa0IQvTHtdgH8FHU4IOi8";

/*
 * Status colours
 */
const IndicatorLed::Colour indicate_0 = IndicatorLed::BLACK;
const IndicatorLed::Colour indicate_awake = IndicatorLed::RED;
const IndicatorLed::Colour indicate_network = IndicatorLed::BLUE;
const IndicatorLed::Colour indicate_mqtt = IndicatorLed::GREEN;

const IndicatorLed::Colour indicate_update = IndicatorLed::CYAN;
const IndicatorLed::Colour indicate_configurator = IndicatorLed::YELLOW;
const IndicatorLed::Colour indicate_wps = IndicatorLed::MAGENTA;

extern void wpsInit();
extern void updateWiFiDef(String &ssid, String &psk);

void WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
  String ssid;
  String psk;

  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    serr.println("Station Mode Started");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    serr.println("Connected to : " + String(WiFi.SSID()));
    serr.print("Got IP: ");
    serr.println(WiFi.localIP());
    dev.p2sys.enterState(P2System::STATE_NETWORK);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    serr.println("Disconnected from station");
    dev.p2sys.enterState(P2System::STATE_AWAKE);
    WiFi.reconnect();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    ssid = WiFi.SSID();
    psk = WiFi.psk();
    serr.println("WPS Successful : " + ssid + "/" + psk);
    dev.p2sys.enterState(P2System::STATE_NETWORK);
    esp_wifi_wps_disable();
    updateWiFiDef(ssid, psk);
    delay(10);
    WiFi.begin();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    serr.println("WPS Failed");
    dev.p2sys.enterState(P2System::STATE_AWAKE);
    esp_wifi_wps_disable();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    serr.println("WPS Timed out");
    dev.p2sys.enterState(P2System::STATE_AWAKE);
    esp_wifi_wps_disable();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    break;
  default:
    break;
  }
}

void initWiFi()
{
  WiFi.onEvent(WiFiEvent);
  connectToWiFi();
  MDNS.begin(config[controllername_n].c_str());
  MDNS.addService("http", "tcp", 80);
}

void wpsInit()
{
  esp_wps_config_t wpsconfig;

  wpsconfig.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
  wpsconfig.wps_type = WPS_TYPE_PBC;
  strcpy(wpsconfig.factory_info.manufacturer, "PA");
  strcpy(wpsconfig.factory_info.model_number, "1");
  strcpy(wpsconfig.factory_info.model_name, "Punkatoo");
  strcpy(wpsconfig.factory_info.device_name, config[controllername_n].c_str());
  // strcpy(wpsconfig.factory_info.device_name, config[controllername_n]);
  esp_wifi_wps_enable(&wpsconfig);
  esp_wifi_wps_start(0);
  serr.println("WPS started");
  dev.p2sys.enterState(P2System::STATE_WPS);
}

void i2cscan()
{
  for (byte address = 0; address <= 127; address++)
  {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (address == 0)
    {
      serr.print("\n    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
    }
    if (address % 16 == 0)
    {
      serr.println("");
    }
    serr.print(" ");
    if (error == 2)
      serr.print("  ");
    else
    {
      if (error < 0x10)
        serr.print("0");
      serr.print(error, HEX);
    }
  }
  serr.println();
}

bool startWPS = false;

void IRAM_ATTR startwps()
{
  startWPS = true;
}

char compDateTime[32] = "";

void parseCompileDate()
{
  const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  struct tm tmstr;
  for (int m = 0; m < 12; m++)
  {
    if (strncmp(compDate, months[m], 3) == 0)
    {
      tmstr.tm_mon = m;
      break;
    }
  }
  sscanf(compDate + 4, "%02d %04d", &tmstr.tm_mday, &tmstr.tm_year);
  tmstr.tm_year -= 1900;
  sscanf(compTime, "%02d:%02d:%02d", &tmstr.tm_hour, &tmstr.tm_min, &tmstr.tm_sec);
  tmstr.tm_isdst = 0;
  tmstr.tm_wday = 0;
  tmstr.tm_yday = 0;

  strftime(compDateTime, sizeof(compDateTime) - 1, "%H:%M:%S %d/%m/%y", &tmstr);
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  parseCompileDate();

  WiFi.mode(WIFI_STA);

  LITTLEFS.begin();

  if (config.readFile() == false)
    config.writeFile();

  serr.println();

  serr.println(gitrevision);
  serr.printf("Compiled at: %s\n", compDateTime);

  Event ev1;
  String sm("Starting ");
  sm += gitrevision;
  ev1.enqueue(sm.c_str());
  dev.build();

  dev.start();

  dev.p2sys.enterState(P2System::STATE_0);

  pinMode(WPS_PIN, INPUT_PULLUP);
  attachInterrupt(WPS_PIN, startwps, FALLING);

  // Ready to go (switch and IR). But network has not been initialised yet

  dev.p2sys.enterState(P2System::STATE_AWAKE);
  Event e2;
  e2.enqueue("Startup complete");

  initWiFi();
  clitask.init();
}

unsigned long long startedAt = 0;

void ntpUpdated(NTPClient *c)
{
  serr.println("NTPUpdate");
  startedAt = c->getEpochMillis();
  c->setUpdateCallback(NULL);
}

void loop()
{
  static bool wifiWasConnected = false;
  static bool ntpstarted = false;

  dev.p2sys.routine();

  if (WiFi.status() == WL_CONNECTED)
  {
    if (!wifiWasConnected)
    {
      wifiWasConnected = true;
      serr.begin("Punkatoo");
      serr.println("WiFi connected");
      dev.p2sys.enterState(P2System::STATE_NETWORK);
    }

    if (!ntpstarted)
    {
      timeClient.setUpdateCallback(ntpUpdated);
      timeClient.begin();
      timeClient.setUpdateInterval(3600000);
      timeClient.setTimeOffset(TZ * 60 * 60);
      ntpstarted = true;
    }
    // in NTPClient_Generic false return is not (necessarily) a failure - it just means
    // not updated, which happens most spins of the loop because no attempt is made.
    // if (!timeClient.update()) serr.println("NTP failure");
    timeClient.update();

    dev.mqtt.poll();

    serr.loop();
  }
  else
  {
    if (wifiWasConnected)
    {
      serr.println("WiFi connection lost");
      wifiWasConnected = false;
    }
    // connectToWiFi();
  }

  if (startWPS)
  {
    wpsInit();
    startWPS = false;
  }

  if (dev.indicators.size() > 0)
  {
    dev.indicators[0].poll();
  }

  static unsigned long eventsthen = 0;
  static unsigned long pingthen = 0;

  unsigned long now = millis();
  if ((now - eventsthen) >= 1 * 1000)
  {
    dev.webServer.sendEvents();
    eventsthen = now;
  }
  if ((now - pingthen) >= 60 * 1000)
  {
    char buffer[64];
    sprintf(buffer, R"!({ "id": "%s", "timeout": 125})!", config[controllername_n].c_str());
    dev.mqtt.publish("ping", buffer);
    pingthen = now;
  }
}
