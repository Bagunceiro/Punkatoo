#include <ArduinoJson.h>
#include "devices.h"
#include "eventlog.h"
#include "config.h"

QueueHandle_t Event::queue = xQueueCreate(20, sizeof(Event));
SemaphoreHandle_t Event::serialNoMutex = xSemaphoreCreateMutex();
uint16_t Event::nextSerial = 1;
// EventLogger Event::logger("event");

Event::Event()
{
    text = NULL;
    onQueue = false;
}

Event::~Event()
{
    // onQueue indicates that there is a memory block on the queue using this same
    // character buffer so don't free it (that will be done by the receiving, dequeued,
    // Event object

    if ((text != NULL) && (!onQueue))
    {
        free(text);
    }
}

/*
bool Event::startLogger(MQTTController &c)
{
    dev.eventlogger.registerMQTT(c);
    return dev.eventlogger.start(0);
}
*/

const uint16_t Event::setSerial()
{
    serial = 0;
    xSemaphoreTake(serialNoMutex, portMAX_DELAY);
    serial = nextSerial;
    if (++nextSerial >= 100)
        nextSerial = 0;
    xSemaphoreGive(serialNoMutex);

    if (startedAt != 0)
    {
        validts = true;
        timestamp = timeClient.getEpochMillis();
    }
    else
    {
        validts = false;
        timestamp = millis();
    }

    /*
    time_t now = time(0);
    ts.msecs = millis() % 1000;
    ts.secs = time(0);
    if (ts.secs != now) // second rolled over, we may need to revise the millisecond remainder
    {
        ts.msecs = millis() % 1000;
    }
    */
    return serial;
}

bool Event::enqueue(const String &msg)
{
    setSerial();
    text = (char *)malloc(msg.length() + 1);
    strcpy(text, msg.c_str());

    bool result = xQueueSend(queue, this, 0);
    if (result)
    {
        onQueue = true;
    }
    return result;
}

bool Event::dequeue()
{
    bool result = xQueueReceive(queue, this, 0);

    if (result)
    {
        onQueue = false;
    }
    return result;
}

bool Event::queued()
{
    bool result = xQueuePeek(queue, this, 0);
    return result;
}

EventLogger::EventLogger(const String name) : P2Task(name, 3000), MQTTClientDev(name)
{
}

bool EventLogger::operator()()
{
    bool result = false;

    Event ev;

    // Limit the transmission rate, and give NTP a chance to
    // get the RTC up to date if it's going to.
    delay(startedAt == 0 ? 3000 : 500);
    
    if (mqttConnected())
    {
        Event ev;
        if (ev.dequeue())
        {
            result = true;
            char buffer[24];

            if (!ev.validts) // Can we correct it now?
            {
                if (startedAt != 0) // Yes
                {
                    ev.timestamp += startedAt;
                    ev.validts = true;
                }
            }
            if (ev.validts)
            {
                time_t tssecs = ev.timestamp / 1000;
                struct tm tmstr;
                localtime_r(&tssecs, &tmstr);
                strftime(buffer, sizeof(buffer) - 1, "%Y-%m-%d %H:%M:%S", &tmstr);
                char msbuff[4];
                sprintf(msbuff, ".%03d", (int)ev.timestamp % 1000);
                strcat(buffer, msbuff);
            }
            else
            {
                sprintf(buffer, "%llu", ev.timestamp);
            }
            StaticJsonDocument<512> doc;
            doc["serial"] = ev.serial;
            JsonObject ts = doc.createNestedObject("ts");
            ts["valid"] = ev.validts;
            ts["timestamp"] = buffer;
            doc["text"] = ev.text;
            String s;
            serializeJson(doc, s);
            mqttPublish("log", s.c_str());
            delay(200);
        }
    }
    return result;
}
