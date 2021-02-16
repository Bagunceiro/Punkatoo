#pragma once
#include <Arduino.h>
#include "ptask.h"
#include "mqtt2.h"


class EventLogger : public PTask, public MQTTClientDev
{
    public:
    EventLogger(const String name = "eventLog");
    virtual bool operator()();
};

class Event
{
public:
  Event();
  virtual ~Event();

  struct Timestamp
  {
    time_t secs;
    uint16_t msecs;
  } ts;
  uint16_t serial;
  char *text;

  // bool enqueue(const char *msg);
  bool enqueue(const String& msg);
  bool dequeue();
  bool startLogger(MQTTController& c);

private:
  bool onQueue;

  const uint16_t setSerial();
  static QueueHandle_t queue;
  static uint16_t nextSerial;
  static SemaphoreHandle_t serialNoMutex;
  static EventLogger logger;
};
