#include <Arduino.h>

#ifdef ESP32

#include <WiFi.h>
#include <ESPmDNS.h>
// #include <ESP32httpUpdate.h>

#else

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>

#endif

#include <PubSubClient.h>
#include <WiFiSerial.h>
#include <time.h>

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

Stream &serr = Serial;

WiFiClient mqttWifiClient;
WiFiClient updWifiClient;

PubSubClient mqttClient(mqttWifiClient);

RGBLed indicator(2,15,12);
Lamp lamp("light");
Fan fan("fan");
Updater updater("updater");
LDR ldr("LDR", LDR_PIN);

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
  fan.setSpeed(0);
  lamp.sw(0);
  // lamp.blip(3,500);
}

void updateCompleted()
{
  time_t now = timeClient.getEpochTime();
  serr.printf("HTTP update process complete at %s\n", ctime(&now));

  persistant[persistant.updateTime_n] = String(now);
  persistant.writeFile();
  // lamp.blip(5,500);
}

void loop2()
{
  lamp.pollSwitch();
  pollIR();
}

#ifndef ESP32
os_timer_t OSTimer;

void timedLoop8266(void *)
{
  loop2();
}
#else
TaskHandle_t loop2Task;
#endif

void loop2mgr(void *)
{
  int hwm = uxTaskGetStackHighWaterMark(loop2Task);
  while (true)
  {
    loop2();
    delay(10);
    int hwmnow = uxTaskGetStackHighWaterMark(loop2Task);
    if (hwmnow < hwm)
    {
      hwm = hwmnow;
      serr.printf("HWM = %d\n", hwm);
    }
  }
}

void setup()
{
  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Fancon Starting");
  indicator.setColour(RGBLed::RED);

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

  pinMode(IR_DETECTOR_PIN, INPUT_PULLUP);
  irrecv.enableIRIn();

  updater.onStart(updateStarted);
  updater.onEnd(updateCompleted);

#ifdef ESP32
  // TaskHandle_t loop2Task;
  xTaskCreate(
      loop2mgr,    /* Function to implement the task */
      "loop2Task", /* Name of the task */
      4000,        /* Stack size in words */
      NULL,        /* Task input parameter */
      0,           /* Priority of the task */
      &loop2Task); /* Task handle. */
#else
  os_timer_setfn(&OSTimer, timedLoop8266, NULL);
  os_timer_arm(&OSTimer, 10, true);
#endif

#ifdef ESP32
    //pinMode(LED_RED, OUTPUT);
    //pinMode(LED_GREEN, OUTPUT);
    //pinMode(LED_BLUE, OUTPUT);

    //digitalWrite(LED_RED, 1);
    //digitalWrite(LED_GREEN, 1);
    //digitalWrite(LED_BLUE, 1);

  //ESPhttpUpdate.onStart(update_started);
  //ESPhttpUpdate.onEnd(update_completed);
#else
  // ESPhttpUpdate.onStart(update_started);
  // ESPhttpUpdate.onEnd(update_completed);
#endif

  webServer.init();
  indicator.setColour({0,0,256});
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
      indicator.setColour({0,256,0});
    }
    wifiattemptcount = 0;

    if (mqttClient.connected())
      mqttClient.loop();
    else
      if (initMQTT())
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

#ifndef ESP32
    MDNS.update();
#endif

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
}
