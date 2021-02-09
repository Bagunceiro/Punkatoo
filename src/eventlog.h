#pragma once

#include <Arduino.h>
#include "ptask.h"

const int logSize = 20;

class Event
{
public:
    Event(const char* = "");
    virtual ~Event();
    void dump();
private:
    time_t timestamp;
    char text[33];
};

class EventLog : public PTask
{
    public:
    EventLog(int queueSize);
    virtual ~EventLog();
    bool writeEvent(const char* t);
    Event readEvent();
    virtual bool operator()();
    void sayHello()
    {
        Serial.println("Hello World");
    }
    void printLog();
    private:
    QueueHandle_t queue;
    Event log[logSize];
    int head;
    int next;
    void addToLog(const Event&);
};