#include "ldr.h"

LDR::LDR(String devName, uint8_t p) : MqttControlled(devName)
{
    pin = p;
}

LDR::~LDR()
{

}

String LDR::getStatus()
{
    uint16_t val = read();
    return String(val);
}

void LDR::mqttaction(const String& topic, const String& msg)
{

}

void LDR::doSubscriptions(PubSubClient& mqttclient)
{

}

uint16_t LDR::read()
{
    return analogRead(pin);
}