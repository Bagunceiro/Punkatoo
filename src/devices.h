#pragma once

#include <Arduino.h>
#include <vector>

#include "lamp.h"
#include "fan.h"
#include "indicator.h"
#include "ldr.h"
#include "tempSensor.h"
#include "configurator.h"

class Devices
{
public:
    Devices();
    IndicatorLed *indicator;
    IRController *irctlr;
    Lamp *lamp;
    Fan *fan;
    LDR *ldr;
    BMESensor *bme;
    MQTTController *mqttctlr;
    Configurator *configurator;
};