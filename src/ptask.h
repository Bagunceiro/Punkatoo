#pragma once

#include <Arduino.h>

class PTask
{
public:
    PTask(const String& n, const int stack);
    virtual ~PTask();
    bool start(uint8_t priority);
    virtual bool operator()() = 0;
    static void loop(void*);
private:
    TaskHandle_t taskHandle;
    String name;
    int stackSize;
};