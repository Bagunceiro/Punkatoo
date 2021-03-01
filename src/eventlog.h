#pragma once
#include <Arduino.h>
#include "ptask.h"
#include "mqtt.h"

class EventLogger : public PTask, public MQTTClientDev
{
    public:
    EventLogger(const String name = "eventLog");
    virtual bool operator()();
    const uint16_t getSerialNo();
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
  bool queued();
  bool startLogger(MQTTController& c);

private:
  bool onQueue;

  const uint16_t setSerial();
  static QueueHandle_t queue;
  static uint16_t nextSerial;
  static EventLogger logger;
  static SemaphoreHandle_t serialNoMutex;
};
