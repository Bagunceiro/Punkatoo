/**
 * @file punkatoo.cpp
 * @brief Lighting and fan controller
 *
 * @author Paul Abraham
 */
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <time.h>
#include <esp_wps.h>
#include <TimeLib.h>
#include <LittleFS.h>

#include "config.h"
#include "devices.h"
#include "spdt.h"
#include "networks.h"
#include "cli.h"
#include "crypt.h"
#include "b64.h"

Devices dev;
CLITask clitask("CLI");
ConfBlk config("/etc/config.json");
StatusIndicator statusInd;

/* @brief "Uptime" baseline. uptime is counted from first NTP update */
unsigned long long startedAt = 0;

/** @brief Set this to true to have the system reboot in a controlled way */
bool resetFlag = false;

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

/**
 * @brief Callback for reporting of wifi events
 * @param event The event in question
 */
void WiFiEvent(WiFiEvent_t event)
{
  String ssid;
  String psk;

  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    Serial.println("Station Mode Started");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println("Connected to : " + String(WiFi.SSID()));
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());
    statusInd.enterState(StatusIndicator::STATE_NETWORK);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("Disconnected from station");
    statusInd.enterState(StatusIndicator::STATE_AWAKE);
    WiFi.reconnect();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    ssid = WiFi.SSID();
    psk = WiFi.psk();
    Serial.println("WPS Successful : " + ssid + "/" + psk);
    statusInd.enterState(StatusIndicator::STATE_NETWORK);
    esp_wifi_wps_disable();
    updateWiFiDef(ssid, psk);
    delay(10);
    WiFi.begin();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    Serial.println("WPS Failed");
    statusInd.enterState(StatusIndicator::STATE_AWAKE);
    esp_wifi_wps_disable();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    Serial.println("WPS Timed out");
    statusInd.enterState(StatusIndicator::STATE_AWAKE);
    esp_wifi_wps_disable();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    break;
  default:
    break;
  }
}

/**
 * @brief Initialise the WiFi
 */
void initWiFi()
{
  WiFi.onEvent(WiFiEvent);
  connectToWiFi();
  MDNS.begin(config[controllername_n].c_str());
  MDNS.addService("http", "tcp", 80);
}

/**
 * WPS (WiFi Protected Setup)
 *
 * This can be used to configure the Wifi network if the router is capable.
 * Punkatoo allows this to be switched on by pressing the "flash" button (on GPIO0)
 */

/**
 * main loop polls this to decide whether to start WPS.
 * Avoid the start code being in the interrupt handler
 */
bool startWPS = false;
/**
 * @brief Interrupt handler - sets a flag for the main loop to pick up
 *        telling it to start up WPS
 */
void IRAM_ATTR startwps()
{
  startWPS = true;
}

/**
 * @brief Start up WPS
 */
void wpsInit()
{
  esp_wps_config_t wpsconfig;

  wpsconfig.wps_type = WPS_TYPE_PBC;
  strcpy(wpsconfig.factory_info.manufacturer, "PA");
  strcpy(wpsconfig.factory_info.model_number, "1");
  strcpy(wpsconfig.factory_info.model_name, "Punkatoo");
  strcpy(wpsconfig.factory_info.device_name, config[controllername_n].c_str());
  esp_wifi_wps_enable(&wpsconfig);
  esp_wifi_wps_start(0);
  Serial.println("WPS started");
  statusInd.enterState(StatusIndicator::STATE_WPS);
}

/**
 * @brief Scan the I2C bus and print out a map of addresses that respond
 *
 * This is a utility function - normally not required in a running system
 */
void i2cscan()
{
  for (byte address = 0; address <= 127; address++)
  {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (address == 0)
    {
      Serial.print("\n    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
    }
    if (address % 16 == 0)
    {
      Serial.println("");
    }
    Serial.print(" ");
    if (error == 2)
      Serial.print("  ");
    else
    {
      if (error < 0x10)
        Serial.print("0");
      Serial.print(error, HEX);
    }
  }
  Serial.println();
}

char compDateTime[32] = ""; // somewhere to hold the compiled date (see parseCompileDate())
/**
 * @brief Convert the C++ preprocessor compile date (MON DD YYYY HH:MM:SS) to a better form for logging (HH:MM:SS DD/MM/YY)
 */
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

/**
 * @brief Callback for the TimeClient to report connection to NTP and set an "uptime" baseline
 */
void ntpUpdated(NTPClient *c)
{
  Serial.println("NTPUpdate");
  if (startedAt == 0)
    startedAt = c->getEpochMillis();
  // c->setUpdateCallback(NULL);
}

/**
 * @brief Arduino style entry point
 */
void setup()
{
  Serial.begin(115200);

  extern const char *privateKey();

  LittleFS.begin();

  dev.build();

  parseCompileDate();

  WiFi.mode(WIFI_STA);

  config.setFileName("/etc/config.json");
  config.readFile();

  Serial.println();

  Serial.println(gitrevision);
  Serial.printf("Compiled at: %s\n", compDateTime);

  Event ev1;
  String sm("Starting ");

  sm += gitrevision;
  ev1.enqueue(sm.c_str());

  statusInd.enterState(StatusIndicator::STATE_0);

  pinMode(WPS_PIN, INPUT_PULLUP);
  attachInterrupt(WPS_PIN, startwps, FALLING);

  // Ready to go (switch and IR). But network has not been initialised yet

  statusInd.enterState(StatusIndicator::STATE_AWAKE);
  Event e2;
  e2.enqueue("Startup complete");

  initWiFi();
  dev.start();
  clitask.init();
}

/**
 * @brief Arduino style main loop
 */
void loop()
{
  static bool wifiWasConnected = false;
  static bool ntpstarted = false;

  if (resetFlag)
  {
    Event{"System restart requested"};
    delay(1000);
    ESP.restart();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    if (!wifiWasConnected)
    {
      wifiWasConnected = true;
      Serial.println("WiFi connected");
      statusInd.enterState(StatusIndicator::STATE_NETWORK);
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

    timeClient.update();

    dev.mqtt.poll();
  }
  else
  {
    if (wifiWasConnected)
    {
      Serial.println("WiFi connection lost");
      wifiWasConnected = false;
    }
    connectToWiFi();
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
  dev.poll();
}
