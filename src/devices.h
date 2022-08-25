#pragma once

#include <vector>
#include <ArduinoJson.h>

#include "mqtt.h"
#include "lamp.h"
#include "fan.h"
#include "infrared.h"
#include "indicator.h"
#include "pir.h"
#include "eventlog.h"
#include "p2webserver.h"

using namespace std;

/**
 * @struct Devices
 * @brief List of real and pseudo devices in the system
 */
struct Devices
{
    MQTTController mqtt;
    IRController irctlr;
    EventLogger eventlogger;
    P2WebServer webServer;
    Switches switchTask;

    vector<Lamp> lamps;
    vector<Fan> fans;
    vector<IRLed> irleds;
    vector<IndicatorLed> indicators;
    vector<PIR> pirs;

    Devices()
    {
    }

    /**
     * @brief Build the device block using a configuration file
     * @param fname The name of the configuration file
     */
    bool build(const char *fname = "/etc/devices.json");
    /** @brief Turn motors off, lights out etc. */
    void toSecure();
    void start();
    /** @brief Most devices that need polling have their own task. But for those that don't */
    void poll();
    /**
     * @brief A lamp calls this to indicate change of state - used to trigger relevant PIR(s)
     * @param l The lamp making the call
     * @param val The new state of the lamp
     */
    void lampState(const Lamp *l, const int val);

private:
    /**
     * @brief parse the configuration JSON
     */
    void parse();
    void buildIRController(JsonObject obj);
    void buildIRLed(JsonArray list);
    void buildIndicator(JsonArray list);
    void buildLamp(JsonArray list);
    void buildSwitch(JsonArray list);
    void buildFan(JsonArray list);
    void buildPIR(JsonArray list);
};

extern struct Devices dev;