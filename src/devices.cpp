#include "devices.h"

Devices::Devices()
{
    mqttctlr = new MQTTController;
    irctlr = new IRController("IRrcv");

    indicator = new RGBLed("indicator", LED_RED, LED_BLUE, LED_GREEN);

    lamp = new Lamp("light");
    fan = new Fan("fan");
    ldr = new LDR("LDR", LDR_PIN);
    bme = new BMESensor("bme");

    configurator = new Configurator("configurator");
}