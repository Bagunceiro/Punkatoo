#include <Arduino.h>
#include <FreeRTOS.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <WiFiSerial.h>
#include <time.h>

const char *appVersion = "Punkatoo 0.1";
const char *compDate   = __DATE__;
const char *compTime   = __TIME__;

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

extern Configurator configurator;

int wifiattemptcount = 0;

void initWiFi()
{
  static unsigned long lastAttempt = 0;
  unsigned long now = millis();
  unsigned long pause = WIFI_CONNECT_ATTEMPT_PAUSE;

  if (wifiattemptcount < 20)
    pause = 500;

  if ((lastAttempt == 0) || ((now - lastAttempt) > pause))
  {
    wifiattemptcount++;

    connectToWiFi();

    lastAttempt = now;
    //    MDNS.begin(persistant[persistant.controllername_n].c_str());
    //    MDNS.addService("http", "tcp", 80);
  }
}

void updateStarted()
{
  serr.println("HTTP update process started");
  indicator.setColour(RGBLed::BLUE);
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

TaskHandle_t lampSwTask;

void lampSwLoop(void *)
{
  int hwm = uxTaskGetStackHighWaterMark(lampSwTask);
  while (true)
  {
    lamp.pollSwitch();
    delay(10);
    int hwmnow = uxTaskGetStackHighWaterMark(lampSwTask);
    if (hwmnow < hwm)
    {
      hwm = hwmnow;
      serr.printf("lampSwTask HWM = %d\n", hwm);
    }
  }
}

TaskHandle_t irTask;

void irLoop(void *)
{
  int hwm = uxTaskGetStackHighWaterMark(irTask);
  while (true)
  {
    irctlr.newpoll();
    delay(10);
    int hwmnow = uxTaskGetStackHighWaterMark(irTask);
    if (hwmnow < hwm)
    {
      hwm = hwmnow;
      serr.printf("irTask HWM = %d\n", hwm);
    }
  }
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

void setup()
{
  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Punkatoo Starting");
  indicator.setColour(RGBLed::RED);
  Wire.begin();

  if (!tempSensor.start(0x76, &Wire))
  {
    Serial.println("Could not find a valid BME280 sensor");
  }

  startup();

  if (persistant.readFile() == false)
  {
    persistant.writeFile();
  }
  persistant.dump(serr);

  Serial.println(appVersion);

  SwitchList sl;
  sl.push_back(LIGHT_SWITCH_PIN);
  lamp.init(sl, LIGHT_RELAY_PIN);

  fan.init(DIR_RELAY1_PIN, DIR_RELAY2_PIN, SPD_RELAY1_PIN, SPD_RELAY2_PIN);

  lamp.registerIR(irctlr);
  fan.registerIR(irctlr);
  configurator.registerIR(irctlr);

  updater.onStart(updateStarted);
  updater.onEnd(updateCompleted);

  xTaskCreate(
      lampSwLoop,   /* Function to implement the task */
      "lampSwTask", /* Name of the task */
      4000,         /* Stack size in words */
      NULL,         /* Task input parameter */
      5,            /* Priority of the task */
      &lampSwTask); /* Task handle. */

  xTaskCreate(
      irLoop,       /* Function to implement the task */
      "irTask",     /* Name of the task */
      4000,         /* Stack size in words */
      NULL,         /* Task input parameter */
      4,            /* Priority of the task */
      &irTask);     /* Task handle. */

  webServer.init();
  indicator.setColour(RGBLed::YELLOW);
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
      WSerial.begin("FanCon");
      serr = WSerial;
      serr.println("WiFi connected");
      MDNS.begin(persistant[persistant.controllername_n].c_str());
      MDNS.addService("http", "tcp", 80);
      indicator.setColour(RGBLed::GREEN);
    }
    wifiattemptcount = 0;

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
      Serial.println("WiFi connection lost");
      wifiWasConnected = false;
    }
    initWiFi();
  }

  configurator.poll();

  static time_t then = 0;
 
  time_t now;
  time(&now);
  if ((now - then) > 60)
  {
    then = now;
    tempSensor.sendStatus();
  }
}
