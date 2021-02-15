#include <Arduino.h>

#include "eventlog.h"
#include "config.h"

uint32_t Event::nextSerialNo = 1;
SemaphoreHandle_t serialNoMutex = NULL;

void Event::construct(const String& txt)
{
    time_t secs1 = time(0);
    uint16_t msecs = millis() % 1000;
    time_t secs = time(0);
    if (secs != secs1) {
        // Seconds just ticked over, get the millis again to ensure
        // that second is excluded
        msecs = millis()%1000;
    }
    timestamp.secs  = secs;
    timestamp.msecs = msecs;
    text = (char*)pvPortMalloc(txt.length() + 1);
    strcpy (text, txt.c_str());
    serialNo = 0;
}

Event::Event()
{
    construct("");
}

Event::Event(const Event &rhs)
{
    memcpy(this, &rhs, sizeof(*this));
    text = (char*)pvPortMalloc(strlen(rhs.text)  + 1);
    strcpy(text, rhs.text);
}

Event::Event(const String& txt)
{
    construct(txt);
}

Event::~Event()
{
    vPortFree(text);
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
        if (nextSerialNo >= 100) nextSerialNo = 0;
        xSemaphoreGive(serialNoMutex);
    }

    else serialNo = 0;
    return serialNo;
}

const String Event::asString()
{
    char buffer[24];
    sprintf(buffer, "%02d %08ld.%03d ", serialNo, timestamp.secs, timestamp.msecs);
    return (String(buffer) + text);
}

void Event::dump()
{
    serr.println(asString());
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

bool EventLog::writeEvent(const String& txt)
{
    Event e(txt);
    e.setSerial();
    Serial.println("writing event:");
    e.dump();
    return xQueueSend(queue, &e, 0);
}

bool EventLog::operator()()
{
    static Event ebuff;
    if (xQueueReceive(queue, &ebuff, 0))
    {
        // Event e = ebuff;
        // e.dump();
        addToLog(ebuff);
    }
    // TODO: if log requested then provide it
    
    return true;
}

void EventLog::addToLog(const Event e)
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

const String EventLog::asString()
{
    int e = head;
    String logstr;
    while (e != next)
    {
        logstr += log[e].asString() + "\n";
        e++;
        if (e >= logSize) e = 0;
    }
    return logstr;
}