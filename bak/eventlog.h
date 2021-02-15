#pragma once

#include <Arduino.h>
#include "ptask.h"

const int logSize = 20;

class Event
{
public:
    Event(const String& t);
    Event();
    Event(const Event &rhs);
    void construct(const String& t);

    virtual ~Event();
    void dump();
    uint32_t setSerial();
    const String asString();

private:
    struct TimeStamp
    {
        time_t secs;
        uint16_t msecs;
    } timestamp;
    uint32_t serialNo;
    char* text;
    // char text[33];
    static uint32_t nextSerialNo;
};

class EventLog : public PTask
{
public:
    EventLog(int queueSize);
    virtual ~EventLog();
    bool writeEvent(const String& t);
    Event readEvent();
    virtual bool operator()();
    void printLog();
    const String asString();

private:
    QueueHandle_t queue;
    Event log[logSize];
    int head;
    int next;
    void addToLog(const Event ev);
};