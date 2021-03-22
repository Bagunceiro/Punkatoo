#include "ldr.h"

LDR::LDR(String devName, uint8_t p) : MQTTClientDev(devName)
{
    pin = p;
}

LDR::~LDR()
{

}

String LDR::mqttGetStatus()
{
    uint16_t val = read();
    return String(val);
}

uint16_t LDR::read()
{
    return analogRead(pin);
}

