#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>

#include "wifiserial.h"
#include "confblk.h"
#include "ntpclient.h"
#include "indicator.h"

enum AppState{
  STATE_0 = 0,
  STATE_AWAKE,
  STATE_NETWORK,
  STATE_MQTT,
  STATE_WPS,
  STATE_CONFIGURATOR,
  STATE_UPDATE
};

extern void enterState(const enum AppState);
extern void revertState();
extern enum AppState appState;

extern const IndicatorLed::Colour indicate_0;
extern const IndicatorLed::Colour indicate_awake;
extern const IndicatorLed::Colour indicate_network;
extern const IndicatorLed::Colour indicate_mqtt;
extern const IndicatorLed::Colour indicate_wps;
extern const IndicatorLed::Colour indicate_configurator;
extern const IndicatorLed::Colour indicate_update;

extern const String MQTT_TPC_STAT;       // "status"
extern const String MQTT_TPC_SPEED;      // "speed"
extern const String MQTT_TPC_SWITCH;     // "switch"
extern const String MQTT_TPC_UPDATE;     // "update"
extern const String MQTT_TPC_SENDIRCODE; // "ir/sendcode"

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

extern const int IRLED_PIN;

extern const unsigned int MQTT_CONNECT_ATTEMPT_INT; // Delay between attempts to reconnect MQTT (ms)
extern const unsigned int WIFI_CONNECT_ATTEMPT_INT;

const int TZ = -3;
extern NTPClient timeClient;
// extern EventLog evLog;

struct ConfigBlock : public ConfBlk
{
  static const char* controllername_n;
  static const char* mqtthost_n;
  static const char* mqttport_n;
  static const char* mqttuser_n;
  static const char* mqttpwd_n;
  static const char* mqttroot_n;
  static const char* mqtttopic_n;
  static const char* updateTime_n;
  static const char* indicator_n;
};

extern ConfigBlock persistant;
// extern Stream &serr;
extern WiFiSerialClient serr;

void   startup();
String upTime();
void   report();

#endif
