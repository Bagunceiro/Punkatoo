#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ConfBlk.h>
#include <WiFi.h>

#include "NTPClient.h"

extern const char* SSID;
extern const char* PSK;


#define MQTT_TPC_STAT   "status"
#define MQTT_TPC_SPEED  "speed"
#define MQTT_TPC_SWITCH "switch"
#define MQTT_TPC_UPDATE "update"

extern const char* appVersion;
extern const char* compDate;
extern const char* compTime;

// GPIO pin definitions

extern const int LED_RED;
extern const int LED_GREEN;
extern const int LED_BLUE;

extern const int DIR_RELAY1_PIN;
extern const int DIR_RELAY2_PIN;
extern const int SPD_RELAY1_PIN;
extern const int SPD_RELAY2_PIN;

extern const int LIGHT_RELAY_PIN;
extern const int LIGHT_SWITCH_PIN;

extern const int IR_DETECTOR_PIN;

extern const int LDR_PIN;

extern const int WPS_PIN;

extern const unsigned int MQTT_CONNECT_ATTEMPT_PAUSE; // Delay between attempts to reconnect MQTT (ms)
extern const unsigned int WIFI_CONNECT_ATTEMPT_PAUSE;

const int TZ = -3;
extern NTPClient timeClient;

struct ConfigBlock : public ConfBlk
{
  static const char* controllername_n;
  static const char* mqtthost_n;
  static const char* mqttport_n;
  static const char* mqttuser_n;
  static const char* mqttpwd_n;
  static const char* mqttroot_n;
  static const char* mqtttopic_n;
  static const char* updateInterval_n;
  static const char* updateServer_n;
  static const char* updateTime_n;
};

extern ConfigBlock persistant;
extern Stream &serr;

void   startup();
String upTime();
void   report();

#endif
