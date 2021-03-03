#include "devices.h"
#include "config.h"

bool Devices::build(const String &filename)
{
    bool result = true;

    IndicatorLed* indicator = new IndicatorLed("indicator", LED_RED, LED_BLUE, LED_GREEN);
    indicators.push_back(*indicator);

    Lamp *lamp = new Lamp("lamp");
    SwitchPinList sl;
    sl.push_back(LIGHT_SWITCH_PIN);
    lamp->init(sl, LIGHT_RELAY_PIN);
    lamp->sw(0);
    lamps.push_back(*lamp);

    Fan *fan = new Fan("fan");
    fan->init(DIR_RELAY1_PIN, DIR_RELAY2_PIN, SPD_RELAY1_PIN, SPD_RELAY2_PIN);
    fans.push_back(*fan);

    IRLed *irled = new IRLed("ir", IRLED_PIN);
    irleds.push_back(*irled);

    BME *bme = new BME("bme");
    bmes.push_back(*bme);

    LDR *ldr = new LDR("LDR", LDR_PIN);
    ldrs.push_back(*ldr);

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
    for (IRLed &led : irleds)
    {
        led.off();
    }
}

void Devices::start()
{
    toSecure();

    Wire.begin();

    irctlr.start(4);
    eventlogger.registerMQTT(mqtt);
    eventlogger.start(0);

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
    for (IRLed &irled : irleds)
    {
        irled.registerMQTT(dev.mqtt);
    }
    for (BME &bme : bmes)
    {
        bme.registerMQTT(mqtt);
        if (!bme.start(0x76, &Wire))
        {
            serr.println("Could not find a valid BME280 sensor on dev 0x76");
        }
    }
    configurator.registerIR(irctlr);
    webServer.init();
}