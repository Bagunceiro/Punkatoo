#pragma once

#include <vector>
#include <ArduinoJson.h>

#include "p2system.h"
#include "mqtt.h"
#include "lamp.h"
#include "fan.h"
#include "ldr.h"
#include "infrared.h"
#include "indicator.h"
#include "bme.h"
#include "eventlog.h"
#include "p2webserver.h"
// #include "watchdog.h"

using namespace std;

struct Devices
{
    P2System        p2sys;
    MQTTController  mqtt;
    IRController*   irctlr;
    EventLogger     eventlogger;
    P2WebServer     webServer;
    Switches*       switchTask;
    // Watchdog        watchdog;

    vector<Lamp>         lamps;
    vector<Fan>          fans;
    SwitchList_t         switches;
    vector<LDR>          ldrs;
    vector<BME>          bmes;
    vector<IRLed>        irleds;
    vector<IndicatorLed> indicators;

    Devices()
    {
        irctlr = NULL;
        switchTask = NULL;
    }

    // Build the device block using a configuration file
    bool build(const char* = "/devices.json");
    // Turn motors off, lights out etc.
    void toSecure();
    void start();

private:
    void parse();
    void buildIRController(JsonObject obj);
    void buildIRLed(JsonArray list);
    void buildIndicator(JsonArray list);
    void buildLamp(JsonArray list);
    void buildSwitch(JsonArray list);
    void buildFan(JsonArray list);
    void buildLDR(JsonArray list);
    void buildBME(JsonArray list);
    // void buildWatchdog(JsonObject obj);
};

extern struct Devices dev;