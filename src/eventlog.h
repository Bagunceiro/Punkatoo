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
  // bool startLogger(MQTTController& c);

private:
  bool onQueue;

  const uint16_t setSerial();
  static QueueHandle_t queue;
  static uint16_t nextSerial;
//  static EventLogger logger;
  static SemaphoreHandle_t serialNoMutex;
};
