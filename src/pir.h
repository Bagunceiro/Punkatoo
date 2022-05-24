#pragma once

#include <Arduino.h>
#include <vector>

#include "lamp.h"

class PIR
{
  /*
   * Passive Infrared detector
   */
public:
  enum State
  {
    UNDETECTED,
    DETECTED
  };

  PIR(const char *name, int pin);
  ~PIR();

  void addLamp(Lamp &l)
  {
    _controlledLamps.push_back(&l);
  }

  void routine();

  State getState()
  {
    return _state;
  }
  const String& getID() { return _id; }

  // Set the timeout (in seconds) before lights are killed.
  // This is set in the devices config, but can be overridden.
  // Set to zero to disable.
  void setTimeoutSecs(const int timeout) { _timeout = timeout * 1000; }

private:
  String _id;
  uint8_t _pin;
  State _state = UNDETECTED;
  unsigned long _lastChange = 0;
  unsigned long _timeout = 0; // in milliseconds to facilitate comparison with millis()
  std::vector<Lamp *> _controlledLamps;
};
