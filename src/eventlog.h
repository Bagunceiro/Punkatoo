#pragma once
#include <Arduino.h>
#include "p2task.h"
#include "mqtt.h"

/**
 * @brief Event Logger for reporting events over MQTT
 *
 * Reads the event queue an passes events on to the MQTT server
 */
class EventLogger : public P2Task, public MQTTClientDev
{
public:
  /**
   * @param name Name for the task for reporting purposes
   */
  EventLogger(const String name = "event");
  /**
   * @brief Entry point to EventLogger code
   */
  virtual bool operator()();
};

/**
 * @brief An event to be logged over MQTT using the EventLogger
 *
 * Events are initially queued (using an RTOS queue) to be picked up by the event logger
 */
class Event
{
public:
  /**
   * @brief Create an empty Event - will need to be "enqueue()"d
   */
  Event();
  /**
   * @brief Convenience constructor - Builds, populates and sends in one.
   *        This means it can be used anonymously, eg: Event { "Log message" };
   * @param msg The text of the log message
   */
  Event(const char *msg);
  virtual ~Event();

  /** @brief Timestamp is valid (ie this has been stamped after an NTP update) */
  bool validts;
  /** When was the Event created */
  unsigned long long timestamp;
  /** @brief Serial number */
  uint16_t serial;
  /** @brief Text of the log message */
  char *text;
  /**
   * @brief Place on queue for EventLogger
   * @param msg The text to be sent
   */
  bool enqueue(const String &msg);
  /**
   * @brief Get first Event from queue
   *
   * This is called on an empty Event. The queued event is copied into it and then
   * discarded.
   * @return Event received
   */
  bool dequeue();
  /**
   * @brief Query queueing status
   * @return This event is on the queue
   */
  bool queued();

private:
  /**
   * @brief Queueing status - true = this Event is on the queue
   */
  bool onQueue;

  /**
   * @brief Set serial number and timestamp
   */
  const uint16_t setSerial();

  /** @brief The logging event queue */
  static QueueHandle_t queue;
  /** @brief The serial number for the next Event */
  static uint16_t nextSerial;
  /** @brief Semaphore to protect access to the queue */
  static SemaphoreHandle_t serialNoMutex;
};
