#pragma once
#include <Arduino.h>
#include "p2task.h"
#include "mqtt.h"

class EventLogger : public P2Task, public MQTTClientDev
{
    public:
    EventLogger(const String name = "event");
    virtual bool operator()();
    const uint16_t getSerialNo();
};

class Event
{
public:
  Event();
  Event(const char* msg);
  virtual ~Event();

  bool validts;
  unsigned long long timestamp;
  uint16_t serial;
  char *text;

  bool enqueue(const String& msg);
  bool dequeue();
  bool queued();

private:
  bool onQueue;

  const uint16_t setSerial();
  static QueueHandle_t queue;
  static uint16_t nextSerial;
  static SemaphoreHandle_t serialNoMutex;
};
