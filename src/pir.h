#pragma once

#include <Arduino.h>
#include <vector>

#include "lamp.h"

/**
 * @brief Passive Infrared detector
 */
class PIR
{
public:
  enum State
  {
    UNDETECTED,
    TRIGGERED
  };

  PIR(const char *name, int pin);
  PIR(const PIR &rhs) { *this = rhs; }
  ~PIR();
  PIR &operator=(const PIR &rhs);

  /**
   * @brief Add to my list of controlled lamps
   * @param l The lamp in question
   */
  void addLamp(Lamp &l)
  {
    _controlledLamps.push_back(&l);
  }

  /**
   * @brief Poll the PIR.
   */
  void routine();

  /**
   * @brief Set the timeout (in seconds) before lights are killed.
   *
   * This is set in the devices config, but can be overridden.
   * Set to zero to disable.
   *
   * @param timeout The timeout value in seconds
   */
  void setTimeoutSecs(const int timeout) { _timeout = timeout * 1000; }

  /**
   * @brief Handle change of lamp state
   *
   * A lamp calls this (indirectly via Devices class) to indicate it has changed state.
   * PIR uses it to self trigger (and thus reset the timer)
   *
   * @param l The lamp in question
   * @param state The new state of the lamp (0 = off)
   */
  void lampState(const Lamp *l, const int state)
  {
    for (const Lamp *cl : _controlledLamps)
    {
      if (l == cl)
      {
        trigger();
        break;
      }
    }
  }

  const String &getID() const { return _id; }
  const State getState() const { return _PIRState; }
  const unsigned long getTimeout() const { return _timeout; }
  const unsigned long getLastTrigger() const { return _lastTriggered; }

private:
  /**
   * @brief Start or restart the PIR's timer
   * @param report - set to false to suppress the logging event
   */
  void trigger(const bool report = true);

  /** @brief A name to allow multiple PIRs(!?) to be distinguished */
  String _id;
  /** @brief The GPIO pin it's on */
  uint8_t _pin;
  /** @brief internal state */
  State _PIRState = UNDETECTED;
  /** @brief Timeout period (in milliseconds to facilitate comparison with millis()) */
  unsigned long _timeout = 0;
  /** @brief List of the lamps that are under control of this PIR */
  std::vector<Lamp *> _controlledLamps;
  /** @brief The last time the PIR was tripped */
  long _lastTriggered = 0;
};