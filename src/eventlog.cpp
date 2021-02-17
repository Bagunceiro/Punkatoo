#include <ArduinoJson.h>
#include "eventlog.h"

QueueHandle_t Event::queue = xQueueCreate(20, sizeof(Event));
SemaphoreHandle_t Event::serialNoMutex = xSemaphoreCreateMutex();
uint16_t Event::nextSerial = 1;
EventLogger Event::logger("event");

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

bool Event::startLogger(MQTTController &c)
{
    logger.registerMQTT(c);
    return logger.start(0);
}

const uint16_t Event::setSerial()
{
    serial = 0;
    xSemaphoreTake(serialNoMutex, portMAX_DELAY);
    serial = nextSerial;
    if (++nextSerial >= 100)
        nextSerial = 0;
    xSemaphoreGive(serialNoMutex);

    time_t now = time(0);
    ts.msecs = millis() % 1000;
    ts.secs = time(0);
    if (ts.secs != now) // second rolled over, we may need to revise the millisecond remainder
    {
        ts.msecs = millis() % 1000;
    }
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

/*
bool Event::enqueue(const char *msg)
{
    setSerial();
    text = (char *)malloc(strlen(msg) + 1);
    strcpy(text, msg);

    bool result = xQueueSend(queue, this, 0);
    if (result)
    {
        onQueue = true;
    }
    return result;
}
*/

bool Event::dequeue()
{
    bool result = xQueueReceive(queue, this, 0);
    if (result)
    {
        onQueue = false;
    }
    return result;
}

EventLogger::EventLogger(const String name) : PTask(name, 3000), MQTTClientDev(name)
{
}

bool EventLogger::operator()()
{
    Event ev;
    bool result = false;

    if (ev.dequeue())
    {
        result = true;
        char buffer[24];
        sprintf(buffer, "%02d %ld.%03d ", ev.serial, ev.ts.secs, ev.ts.msecs);

        Serial.println(String(buffer) + String(ev.text));
        StaticJsonDocument<512> doc;
        doc["serial"] = ev.serial;
        JsonObject ts = doc.createNestedObject("ts");
        ts["secs"] = ev.ts.secs;
        ts["msecs"] = ev.ts.msecs;
        doc["text"] = ev.text;
        String s;
        serializeJson(doc, s);
        publish("log", s.c_str());
    }
    return result;
}
    /*
void setup()
{
  Serial.begin(9600);
  Event e;
  e.startLogger();
}

void loop()
{
  static int count = 0;

  char buffer[20];
  sprintf(buffer, "Message %d", ++count);

  Event e;
  e.enqueue(buffer);
  delay (200);
}
*/