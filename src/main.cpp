#include <Arduino.h>
#include <FreeRTOS.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <WiFiSerial.h>
#include <time.h>
#include <esp_wps.h>

const char *appVersion = "Punkatoo 0.1";
const char *compDate = __DATE__;
const char *compTime = __TIME__;

#include "config.h"
#include "spdt.h"
#include "mqtt.h"
#include "infrared.h"
#include "webserver.h"
#include "configurator.h"
#include "networks.h"
#include "lamp.h"
#include "fan.h"
#include "ldr.h"
#include "updater.h"
#include "rgbled.h"
#include "tempSensor.h"

Stream &serr = Serial;

WiFiClient mqttWifiClient;
WiFiClient updWifiClient;

PubSubClient mqttClient(mqttWifiClient);

RGBLed indicator(LED_RED, LED_BLUE, LED_GREEN);
IRController irctlr;
Lamp lamp("light");
Fan fan("fan");
Updater updater("updater");
LDR ldr("LDR", LDR_PIN);
TempSensor tempSensor;
Configurator configurator;

/*
 * Status LED colours
 */
RGBLed::Colour indicateStarting = RGBLed::RED;
RGBLed::Colour indicateNoNet = RGBLed::YELLOW;
RGBLed::Colour indicateNet = RGBLed::GREEN;
RGBLed::Colour indicateUpdate = RGBLed::BLUE;
RGBLed::Colour indicateConfig = RGBLed::CYAN;
RGBLed::Colour indicateWPS = RGBLed::MAGENTA;

extern esp_wps_config_t wpsconfig;
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
    indicator.setColour(indicateNet, 60);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    serr.println("Disconnected from station");
    indicator.setColour(indicateNoNet, 60);
    WiFi.reconnect();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    ssid = WiFi.SSID();
    psk = WiFi.psk();
    serr.println("WPS Successful : " + ssid + "/" + psk);
    indicator.setColour(indicateNet, 60);
    esp_wifi_wps_disable();
    updateWiFiDef(ssid, psk);
    delay(10);
    WiFi.begin();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    serr.println("WPS Failed");
    indicator.setColour(indicateNoNet, 60);
    esp_wifi_wps_disable();
    /*
    esp_wifi_wps_enable(&wpsconfig);
    esp_wifi_wps_start(0);
    */
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    serr.println("WPS Timedout");
    indicator.setColour(indicateNoNet, 60);
    esp_wifi_wps_disable();
    /*
    esp_wifi_wps_enable(&wpsconfig);
    esp_wifi_wps_start(0);
    */
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
  MDNS.begin(persistant[persistant.controllername_n].c_str());
  MDNS.addService("http", "tcp", 80);
}

void updateStarted()
{
  serr.println("HTTP update process started");
  indicator.setColour(indicateUpdate);
  fan.setSpeed(0);
  lamp.sw(0);
}

void updateCompleted()
{
  indicator.off();
  time_t now = timeClient.getEpochTime();
  serr.printf("HTTP update process complete at %s\n", ctime(&now));

  persistant[persistant.updateTime_n] = String(now);
  persistant.writeFile();
}

void updateNone()
{
  indicator.off();
}

void updateFail()
{
  indicator.off();
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
      serr.print(address, HEX);
      serr.print(":");
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

void setup()
{
  WiFi.mode(WIFI_STA);

  startup(); // set start time

  Serial.begin(9600);
  indicator.setColour(indicateStarting);

  serr.println("");
  serr.println(appVersion);
  if (persistant.readFile() == false)
    persistant.writeFile();
  persistant.dump(serr);

  /*
   * Start up the lamp
   */
  SwitchList sl;
  sl.push_back(LIGHT_SWITCH_PIN);
  lamp.init(sl, LIGHT_RELAY_PIN);
  lamp.registerIR(irctlr);
  lamp.start(5);

  /*
   * Start up the Infra red controller
   */
  irctlr.start(4);

  /*
   * Start up the fan
   */
  fan.init(DIR_RELAY1_PIN, DIR_RELAY2_PIN, SPD_RELAY1_PIN, SPD_RELAY2_PIN);
  fan.registerIR(irctlr);

  /*
   * Ready to go (switch and IR). But network has not been initialised yet
   */
  indicator.setColour(indicateNoNet, 60);

  pinMode(WPS_PIN, INPUT_PULLUP);
  attachInterrupt(WPS_PIN, startwps, FALLING);

  initWiFi();

  Wire.begin();
  if (!tempSensor.start(0x76, &Wire))
  {
    serr.println("Could not find a valid BME280 sensor");
  }

  /*
  * Enable the network configurator and OTA updater
  */
  configurator.registerIR(irctlr);
  updater.onStart(updateStarted);
  updater.onEnd(updateCompleted);

  /*
   * Set up the Web server
   */
  webServer.init();
}

void loop()
{
  static bool wifiWasConnected = false;
  static bool ntpstarted = false;
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!wifiWasConnected)
    {
      wifiWasConnected = true;
      WSerial.begin("Punkatoo");
      serr = WSerial;
      serr.println("WiFi connected");
      indicator.setColour(indicateNet);
    }

    if (mqttClient.connected())
      mqttClient.loop();
    else if (initMQTT())
    {
      indicator.off();
    }

    if (!ntpstarted)
    {
      timeClient.begin();
      timeClient.setUpdateInterval(3600000);
      ntpstarted = true;
      timeClient.update();
      timeClient.setTimeOffset(TZ * 60 * 60);
    }
    else
    {
      // in NTPClient_Generic false return is not (necessarily) a failure - it just means
      // not updated, which happens most spins of the loop becasue no attempt is made.
      // if (!timeClient.update()) serr.println("NTP failure");
      timeClient.update();
    }

    webServer.handleClient();
    WSerial.loop();
  }
  else
  {
    if (wifiWasConnected)
    {
      serr.println("WiFi connection lost");
      wifiWasConnected = false;
    }
  }

  configurator.poll();

  static unsigned long then = 0;

  unsigned long now = millis();
  if ((now - then) > (1 * 60 * 1000))
  {
    then = now;
    tempSensor.sendStatus();
  }
  indicator.poll();

  if (startWPS)
  {
    wpsInit();
    indicator.setColour(indicateWPS);
    startWPS = false;
  }
}
