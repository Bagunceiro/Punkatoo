#include "devices.h"

bool Devices::build(const String &filename)
{
    bool result = true;
    Fan* fan = new Fan("fan");
    fan->init(DIR_RELAY1_PIN, DIR_RELAY2_PIN, SPD_RELAY1_PIN, SPD_RELAY2_PIN);
    // fan->registerIR(dev.irctlr);
    // fan->registerMQTT(dev.mqtt);
    fans.push_back(*fan);
    return result;
}

void Devices::toSecure()
{
    for(Lamp& lamp: lamps)
    {
        lamp.sw(0);
    }
    for(Fan& fan: fans)
    {
        fan.setSpeed(0);
    }
    for(IRLed led: irleds)
    {
        led.off();
    }
}

void Devices::start()
{
    toSecure();
    for(Fan& fan: fans)
    {
        fan.registerIR(irctlr);
        fan.registerMQTT(mqtt);
    }
}