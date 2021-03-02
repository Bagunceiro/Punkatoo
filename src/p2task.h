#pragma once

#include <Arduino.h>

class P2Task
{
public:
    P2Task(const String& n, const int stack);
    virtual ~P2Task();
    bool start(uint8_t priority);
    virtual bool operator()() = 0;
    static void loop(void*);
private:
    TaskHandle_t taskHandle;
    String name;
    int stackSize;
};