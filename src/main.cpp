#include <Arduino.h>
#include <FreeRTOS.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <time.h>
#include <esp_wps.h>
#include <TimeLib.h>

const char *appVersion = "Punkatoo 0.3";
const char *compDate = __DATE__;
const char *compTime = __TIME__;

#include "wifiserial.h"
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
#include "indicator.h"
#include "tempSensor.h"
#include "eventlog.h"
// #include "devices.h"

WiFiSerialClient serr;

/*
 * Physical Devices
 *  Note that for MQTT devices the names form part of the topic during publish
 */
IndicatorLed indicator("indicator", LED_RED, LED_BLUE, LED_GREEN);
IRController irctlr("IRrcv");
Lamp lamp("lamp");
Fan fan("fan");
LDR ldr("LDR", LDR_PIN);
BMESensor bme("bme");
IRLed irled("ir", IRLED_PIN);

/*
 * Pseudo Devices
 */
MQTTController mqttctlr;
Configurator configurator("configurator");
EventLogger Event::logger("event");

enum AppState appState;
enum AppState prevState;

/*
 * Status colours
 */
const IndicatorLed::Colour indicate_0            = IndicatorLed::BLACK;
const IndicatorLed::Colour indicate_awake        = IndicatorLed::RED;
const IndicatorLed::Colour indicate_network      = IndicatorLed::BLUE;
const IndicatorLed::Colour indicate_mqtt         = IndicatorLed::GREEN;

const IndicatorLed::Colour indicate_update       = IndicatorLed::CYAN;
const IndicatorLed::Colour indicate_configurator = IndicatorLed::YELLOW;
const IndicatorLed::Colour indicate_wps          = IndicatorLed::MAGENTA;

// extern esp_wps_config_t wpsconfig;
extern void wpsInit();
extern void updateWiFiDef(String &ssid, String &psk);

void enterState(enum AppState s)
{
  prevState = appState;
  switch (s)
  {
  case STATE_0:
    indicator.setColour(indicate_0);
    break;
  case STATE_AWAKE:
    indicator.setColour(indicate_awake);
    break;
  case STATE_NETWORK:
    indicator.setColour(indicate_network);
    break;
  case STATE_MQTT:
    indicator.setColour(indicate_mqtt);
    break;
  case STATE_WPS:
    indicator.setColour(indicate_wps);
    break;
  case STATE_CONFIGURATOR:
    indicator.setColour(indicate_configurator);
    break;
  case STATE_UPDATE:
    indicator.setColour(indicate_update);
    break;
  default:
    break;
  }
}

void revertState()
{
  prevState = appState;
}

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
    enterState(STATE_NETWORK);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    serr.println("Disconnected from station");
    enterState(STATE_AWAKE);
    WiFi.reconnect();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    ssid = WiFi.SSID();
    psk = WiFi.psk();
    serr.println("WPS Successful : " + ssid + "/" + psk);
    enterState(STATE_NETWORK);
    esp_wifi_wps_disable();
    updateWiFiDef(ssid, psk);
    delay(10);
    WiFi.begin();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    serr.println("WPS Failed");
    enterState(STATE_AWAKE);
    esp_wifi_wps_disable();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    serr.println("WPS Timed out");
    enterState(STATE_AWAKE);
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
  MDNS.begin(persistant[persistant.controllername_n].c_str());
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
  strcpy(wpsconfig.factory_info.device_name, persistant[persistant.controllername_n].c_str());
  esp_wifi_wps_enable(&wpsconfig);
  esp_wifi_wps_start(0);
  serr.println("WPS started");
  enterState(STATE_WPS);
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

void setup()
{
  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  delay(500);

  Event ev1;
  ev1.startLogger(mqttctlr);
  ev1.enqueue("Starting");

  enterState(STATE_0);

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
  lamp.registerMQTT(mqttctlr);
  lamp.start(5);
  lamp.sw(0);
  // delay(500);

  /*
   * Start up the fan
   */
  fan.init(DIR_RELAY1_PIN, DIR_RELAY2_PIN, SPD_RELAY1_PIN, SPD_RELAY2_PIN);
  fan.registerIR(irctlr);
  fan.registerMQTT(mqttctlr);
  fan.setSpeed(0);

  /*
   * Start up the Infra red controller
   */
  irctlr.start(4);

  irled.registerMQTT(mqttctlr);
  bme.registerMQTT(mqttctlr);

  /*
   * Ready to go (switch and IR). But network has not been initialised yet
   */
  enterState(STATE_AWAKE);
  Event e2;
  e2.enqueue("Startup complete");

  pinMode(WPS_PIN, INPUT_PULLUP);
  attachInterrupt(WPS_PIN, startwps, FALLING);

  initWiFi();

  Wire.begin();
  if (!bme.start(0x76, &Wire))
  {
    serr.println("Could not find a valid BME280 sensor");
  }

  /*
  * Enable the network configurator and OTA updater
  */
  configurator.registerIR(irctlr);

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
      serr.begin("Punkatoo");
      serr.println("WiFi connected");
      enterState(STATE_NETWORK);
    }

    mqttctlr.poll();

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
    serr.loop();
  }
  else
  {
    if (wifiWasConnected)
    {
      serr.println("WiFi connection lost");
      wifiWasConnected = false;
    }
    connectToWiFi();
  }

  configurator.poll();

  static unsigned long then = 0;

  unsigned long now = millis();
  if ((now - then) > (1 * 60 * 1000))
  {
    then = now;
    bme.sendStatus();
  }

  if (startWPS)
  {
    wpsInit();
    startWPS = false;
  }

  indicator.poll();
}