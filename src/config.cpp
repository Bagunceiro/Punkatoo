#include <WiFi.h>
#include <LITTLEFS.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>

#include "wifiserial.h"
#include "config.h"

WiFiUDP udp;

NTPClient timeClient(udp, TZ * 60 * 60);

const char *controllername_n = "controllername";
const char *mqtthost_n = "mqtthost";
const char *mqttport_n = "mqttport";
const char *mqttuser_n = "mqttuser";
const char *mqttpwd_n = "mqttpwd";
const char *mqttroot_n = "mqttroot";
const char *mqtttopic_n = "mqtttopic";
const char *indicator_n = "indicator";
const char *watchdog_n = "watchdog";

const String MQTT_TPC_STAT = "status";
const String MQTT_TPC_SPEED = "fan/speed";
const String MQTT_TPC_SWITCH = "/switch";
const String MQTT_TPC_SWITCHTO = "/switchto";
const String MQTT_TPC_UPDATE = "update";
const String MQTT_TPC_SENDIRCODE = "ir/sendcode";
const String MQTT_TPC_RECDIRCODE = "recdcode";
const String MQTT_TPC_SYSTEM = "system";

// GPIO pins

const int WPS_PIN = 0;

const unsigned int MQTT_CONNECT_ATTEMPT_INT = (10 * 1000);     // Interval between attempts to reconnect MQTT (ms)
const unsigned int WIFI_CONNECT_ATTEMPT_INT = (5 * 60 * 1000); // Interval between attempts to reconnect WiFi (ms)

ConfBlk config;

const char *timeAndDate(char buff[], const int maxSize, const time_t when)
{
  struct tm tmstr;
  localtime_r(&when, &tmstr);
  strftime(buff, maxSize, "%d/%m/%y %H:%M:%S", &tmstr);
  return buff;
}

const char *startTime()
{
  static char buff[32];
  if (startedAt != 0)
  {
    time_t then = startedAt / 1000;
    timeAndDate(buff, sizeof(buff) - 1, then);
  }
  else
  {
    strcpy(buff, "");
  }
  return buff;
}

const char *nowTime()
{
  static char buff[32];
  if (startedAt != 0)
  {
    time_t now = timeClient.getEpochTime();
    timeAndDate(buff, sizeof(buff) - 1, now);
  }
  else
  {
    strcpy(buff, "");
  }
  return buff;
}

unsigned long long upFor()
{
  if (startedAt != 0)
    return timeClient.getEpochMillis() - startedAt;
  else
    return millis();
}

const char *upTime()
{
  static char buffer[24];

  unsigned long long period = upFor();

  int days = (period / (1000 * 60 * 60 * 24));
  int hours = (period % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
  int mins = (period % (1000 * 60 * 60)) / (1000 * 60);
  int secs = (period % (1000 * 60)) / 1000;
  // int msecs = (period % 1000);
  // sprintf(buffer, "%d days, %02d:%02d:%02d.%03d", days, hours, mins, secs, msecs);
  sprintf(buffer, "%d days, %02d:%02d:%02d", days, hours, mins, secs);
  return buffer;
}
