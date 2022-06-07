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
    TRIGGERED
  };

  PIR(const char *name, int pin);
  ~PIR();

  void addLamp(Lamp &l)
  {
    _controlledLamps.push_back(&l);
    l.onAction(lampActivity, (void*)this);
  }

  void routine();

/*
  State getSignalState()
  {
    return _signalState;
  }
*/
  const String& getID() { return _id; }

  // Set the timeout (in seconds) before lights are killed.
  // This is set in the devices config, but can be overridden.
  // Set to zero to disable.
  void setTimeoutSecs(const int timeout) { _timeout = timeout * 1000; }
  
  void trigger()
  {
    _PIRState = TRIGGERED;
    _lastChange = millis();
  }

private:
  String _id;
  uint8_t _pin;
  State _signalState = UNDETECTED;
  State _PIRState = UNDETECTED;
  unsigned long _lastChange = 0;
  unsigned long _timeout = 0; // in milliseconds to facilitate comparison with millis()
  void lampActivity(Lamp* l, const uint8_t state);
  static void lampActivity(Lamp* l, const uint8_t state, void* pir);
  std::vector<Lamp *> _controlledLamps;
  long lastDetected = 0; // The last time the PIR was tripped
};