#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "p2task.h"

class CLITask : public P2Task
{
    public:
    CLITask(const char* name);
    virtual ~CLITask();
    bool operator()() override;
    void init();

    private:
    WiFiServer cliServer;
    WiFiClient cliClient;
    String getCommand();

};