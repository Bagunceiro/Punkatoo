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

private:
  String _id;
  uint8_t _pin;
  State _state = UNDETECTED;
  unsigned long _lastChange = 0;
  std::vector<Lamp *> _controlledLamps;
};
