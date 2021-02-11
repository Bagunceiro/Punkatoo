#pragma once

#include "infrared.h"


class Configurator : public IRControlled
{
public:
    Configurator(const String& name);
    virtual ~Configurator();
    virtual void irmsgRecd(uint32_t code);
    void start();
    void stop();
    void poll();

private:
    unsigned long startedAt;
    bool running;
    bool startRequest;
    unsigned int startCodeState;
    unsigned long stateChangedAt;
};