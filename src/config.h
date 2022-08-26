#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "confblk.h"
#include "ntpclient.h"
#include "indicator.h"
// #include "devices.h"

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
extern const String MQTT_TPC_SWITCHTO;   // "switchto"
extern const String MQTT_TPC_UPDATE;     // "update"
extern const String MQTT_TPC_SENDIRCODE; // "ir/sendcode"
extern const String MQTT_TPC_RECDIRCODE; // "recdcode"
extern const String MQTT_TPC_SYSTEM;     // "system"

extern const char *compTime;
extern const char *compDate;
extern const char *gitrevision;

extern char compDateTime[];

// GPIO pin definitions

extern const int WPS_PIN;

extern const unsigned int MQTT_CONNECT_ATTEMPT_INT; // Delay between attempts to reconnect MQTT (ms)
extern const unsigned int WIFI_CONNECT_ATTEMPT_INT;

const int TZ = -3;
extern NTPClient timeClient;

extern const char *controllername_n;
extern const char *mqtthost_n;
extern const char *mqttport_n;
extern const char *mqttuser_n;
extern const char *mqttpwd_n;
extern const char *mqttroot_n;
extern const char *mqtttopic_n;
extern const char *indicator_n;
extern const char *watchdog_n;

extern ConfBlk config;

extern unsigned long long startedAt;

void startup();

unsigned long long upFor();
const char* timeAndDate(char buff[], const int maxSize, const time_t when);

const char* nowTime();
const char* startTime();
const char* lastUpdateTime();
const char* upTime();

void report();