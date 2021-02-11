#include "ldr.h"

LDR::LDR(String devName, uint8_t p)
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

uint16_t LDR::read()
{
    return analogRead(pin);
}

