#include "devices.h"

bool Devices::build(const String &filename)
{
    bool result = true;

    Lamp *lamp = new Lamp("lamp");
    SwitchPinList sl;
    sl.push_back(LIGHT_SWITCH_PIN);
    lamp->init(sl, LIGHT_RELAY_PIN);
    lamp->sw(0);
    lamps.push_back(*lamp);

    Fan *fan = new Fan("fan");
    fan->init(DIR_RELAY1_PIN, DIR_RELAY2_PIN, SPD_RELAY1_PIN, SPD_RELAY2_PIN);
    fans.push_back(*fan);

    return result;
}

void Devices::toSecure()
{
    for (Lamp &lamp : lamps)
    {
        lamp.sw(0);
    }
    for (Fan &fan : fans)
    {
        fan.setSpeed(0);
    }
    for (IRLed led : irleds)
    {
        led.off();
    }
}

void Devices::start()
{
    toSecure();
    for (Fan &fan : fans)
    {
        fan.registerIR(irctlr);
        fan.registerMQTT(mqtt);
    }
    for (Lamp &lamp : lamps)
    {
        lamp.registerIR(irctlr);
        lamp.registerMQTT(mqtt);
        lamp.start(5);
    }
}