#pragma once

#include "mqtt.h"

class LDR
{
public:
    LDR(String devName, uint8_t p);
    virtual String getStatus();

    virtual ~LDR();
    uint16_t read();
private:
    uint8_t pin;
};