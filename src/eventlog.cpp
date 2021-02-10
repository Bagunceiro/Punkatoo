#include <Arduino.h>

#include "eventlog.h"
#include "config.h"

uint32_t Event::nextSerialNo = 1;
SemaphoreHandle_t serialNoMutex = NULL;

Event::Event(const char* txt)
{


    timestamp = millis();
    strncpy(text, txt, 25);
    serialNo = 0;
}

Event::~Event()
{
}

uint32_t Event::setSerial()
{
    if(serialNoMutex == NULL )
    {
        serialNoMutex = xSemaphoreCreateMutex();
    }

    if (serialNoMutex != NULL)
    {
        xSemaphoreTake(serialNoMutex, portMAX_DELAY);
        serialNo = nextSerialNo++;
        xSemaphoreGive(serialNoMutex);
    }

    else serialNo = 0;
    return serialNo;
}

void Event::dump()
{
    char buffer[24];
    /*
    tm timep;
    localtime_r(&timestamp, &timep);
    strftime(buffer,20,"%d/%m/%y %H:%M:%S", &timep);
    */
    sprintf(buffer, "%05d %011ld", serialNo, timestamp);
    serr.print(buffer);
    serr.print("\t");
    serr.println(text);
}

EventLog::EventLog(int queueSize) : PTask("EventLog", 10000)
{
    Serial.println("Creating event log");
    queue = xQueueCreate(queueSize, sizeof(Event));
    head = 0;
    next = 0;
}

EventLog::~EventLog()
{
}

bool EventLog::writeEvent(const char* txt)
{

    Event e(txt);
    e.setSerial();
    return xQueueSend(queue, &e, 0);
    return true;
}

bool EventLog::operator()()
{
    Event e;
    
    if (xQueueReceive(queue, &e, 0))
    {
        // e.dump();
        addToLog(e);
    }
    // TODO: if log requested then provide it
    
    return true;
}

void EventLog::addToLog(const Event& e)
{
    log[next++] = e;
    if (next >= logSize) next = 0;
    if (next == head)
    {
        head++;
        if (head >= logSize) head = 0;
    }
}

void EventLog::printLog()
{
    serr.println("Log:");
    int e = head;
    while (e != next)
    {
        log[e].dump();
        e++;
        if (e >= logSize) e = 0;
    }
}