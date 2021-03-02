#pragma once

#include "infrared.h"


class Configurator : public IRControlled
{
public:
    Configurator(const String& name = "configurator");
    virtual ~Configurator();
    virtual void subscribeToIR();
    virtual void irmsgRecd(const IRMessage m);
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