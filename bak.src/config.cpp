#include <WiFi.h>
#include <LITTLEFS.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>

#include "wifiserial.h"
#include "config.h"

WiFiUDP udp;

NTPClient timeClient(udp, TZ * 60 * 60);

const char *ConfigBlock::controllername_n = "controllername";
const char *ConfigBlock::mqtthost_n       = "mqtthost";
const char *ConfigBlock::mqttport_n       = "mqttport";
const char *ConfigBlock::mqttuser_n       = "mqttuser";
const char *ConfigBlock::mqttpwd_n        = "mqttpwd";
const char *ConfigBlock::mqttroot_n       = "mqttroot";
const char *ConfigBlock::mqtttopic_n      = "mqtttopic";
const char *ConfigBlock::updateInterval_n = "updateInterval";
const char *ConfigBlock::updateServer_n   = "updateServer";
const char *ConfigBlock::updateTime_n     = "updateTime";

// GPIO pins

const int LED_RED          = 2;
const int LED_GREEN        = 12;
const int LED_BLUE         = 15;

const int SPD_RELAY2_PIN   = 33; // K1
const int SPD_RELAY1_PIN   = 25; // K2
const int DIR_RELAY1_PIN   = 26; // K3
const int DIR_RELAY2_PIN   = 27; // K4

const int LIGHT_RELAY_PIN  = 13; // K5
const int LIGHT_SWITCH_PIN = 19;
// const int LIGHT_SWITCH_PIN = 23;

const int IR_DETECTOR_PIN  = 34;
const int IR_TRANSMIT_PIN  = 4;
const int LDR_PIN          = 35;

const int WPS_PIN          = 0;


const unsigned int MQTT_CONNECT_ATTEMPT_PAUSE = 30000; // Delay between attempts to reconnect MQTT (ms)
const unsigned int WIFI_CONNECT_ATTEMPT_PAUSE = 15000;

ConfigBlock persistant;
time_t startTime = 0;

void startup()
{
  startTime = time(0);
}

String upTime()
{
  time_t now = time(0) - startTime;
  int days = now / (60 * 60 * 24);
  int hours = (now % (60 * 60 * 24)) / (60 * 60);
  int mins = (now % (60 * 60)) / 60;
  int secs = now % 60;
  char buffer[20];
  sprintf(buffer, "%d days, %02d:%02d:%02d", days, hours, mins, secs);
  return buffer;
}