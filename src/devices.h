#pragma once

#include <vector>

#include "mqtt.h"
#include "lamp.h"
#include "fan.h"
#include "ldr.h"
#include "infrared.h"
#include "indicator.h"
#include "bme.h"
#include "configurator.h"
#include "eventlog.h"
#include "updater.h"
#include "p2webserver.h"

using namespace std;

struct Devices
{
    MQTTController  mqtt;
    IRController    irctlr;
    Configurator    configurator;
    EventLogger     eventlogger;
    Updater         updater;
    P2WebServer     webServer;

    vector<Lamp>         lamps;
    vector<Fan>          fans;
    vector<Switch>       switches;
    vector<LDR>          ldrs;
    vector<BMESensor>    bmes;
    vector<IRLed>        irleds;
    vector<IndicatorLed> indicators;

    // Build the device block using a configuration file
    bool build(const String &filename = "devices.json");
    // Turn motors off, lights out etc.
    void toSecure();
    void start();
};

extern struct Devices dev;