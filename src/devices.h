#pragma once

#include <vector>
#include <ArduinoJson.h>

#include "p2system.h"
#include "mqtt.h"
#include "lamp.h"
#include "fan.h"
#include "infrared.h"
#include "indicator.h"
#include "pir.h"
#include "eventlog.h"
#include "p2webserver.h"

using namespace std;

struct Devices
{
    P2System        p2sys;
    MQTTController  mqtt;
    IRController    irctlr;
    EventLogger     eventlogger;
    P2WebServer     webServer;
    Switches        switchTask;

    vector<Lamp>         lamps;
    vector<Fan>          fans;
    vector<IRLed>        irleds;
    vector<IndicatorLed> indicators;
    vector<PIR>          pirs;

    Devices()
    {
    }

    // Build the device block using a configuration file
    bool build(const char* = "/etc/devices.json");
    // Turn motors off, lights out etc.
    void toSecure();
    void start();
    // Most devices that need polling have their own task. But for those that don't
    void poll();
    // A lamp calls this to indicate change of state - used to trigger relevant PIR(s)
    void lampState(const Lamp* l, const int val);

private:
    void parse();
    void buildIRController(JsonObject obj);
    void buildIRLed(JsonArray list);
    void buildIndicator(JsonArray list);
    void buildLamp(JsonArray list);
    void buildSwitch(JsonArray list);
    void buildFan(JsonArray list);
    void buildLDR(JsonArray list);
    void buildPIR(JsonArray list);
};

extern struct Devices dev;