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
#include "weather.h"
#include "pir.h"
#include "eventlog.h"
#include "p2webserver.h"

using namespace std;

struct Devices
{
    P2System        p2sys;
    MQTTController  mqtt;
    IRController*   irctlr;
    EventLogger     eventlogger;
    P2WebServer     webServer;
    Switches        switchTask;
    WeatherStation  weatherStn;

    vector<Lamp>         lamps;
    vector<Fan>          fans;
    vector<LDR>          ldrs;
    vector<IRLed>        irleds;
    vector<IndicatorLed> indicators;
    vector<PIR>          pirs;

    Devices()
    {
        irctlr = NULL;
    }

    // Build the device block using a configuration file
    bool build(const char* = "/etc/devices.json");
    // Turn motors off, lights out etc.
    void toSecure();
    void start();
    // Most devices that need polling have their own task. But for those that don't
    void poll();

private:
    void parse();
    void buildIRController(JsonObject obj);
    void buildIRLed(JsonArray list);
    void buildIndicator(JsonArray list);
    void buildLamp(JsonArray list);
    void buildSwitch(JsonArray list);
    void buildFan(JsonArray list);
    void buildLDR(JsonArray list);
    // void buildBME(JsonArray list);
    void buildPIR(JsonArray list);
    void buildWeatherStn(JsonObject obj);
};

extern struct Devices dev;