#pragma once

#include <vector>

class SwitchedDev
{
  /*
   * A device controlled by a Switch object
   */
  public:
  SwitchedDev(const String& i) { id = i; }
  const String getid() { return id; }
  virtual void switchTo(const int state) = 0;
  private:
  String id;
};

class Switch
{
  /*
   * An input switch, physically SPST, on/off but the transition between
   * states is used to toggle the associated SwitchedDev object(s).
   * Any number of SwitchedDevs can be controlled by a single Switch.
   * 
   * Future enhancement option - add momentary switch capability.
   */
  public:

  Switch(const String& i, const int pin);
  void addDevice(SwitchedDev& d) { switched.push_back(&d);}
  bool poll();

  private:
  String id;
  int spin;
  int switchState;
  int debounce;
  unsigned long changeAt; // When the switch changed state
  std::vector<SwitchedDev*> switched;
};

class Switches : public P2Task
{
  /*
   * RTOS task that routines the switches attached to the system and acts upon them
   */
public:
  Switches(std::vector<Switch>* list) : P2Task("switches", 2000) { swlist = list; }
private:
  std::vector<Switch>* swlist;
  bool operator()();
};