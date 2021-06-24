#pragma once

#include <vector>

class SwitchedDev
{
  /*
   * A device controlled by a Switch object
   */
public:
  SwitchedDev(const String &i) { id = i; }
  const String getid() { return id; }
  virtual void switched(const char *parm) = 0;

private:
  String id;
};

class Switch
{
  /*
  * An actuator
  * This class defines what it does, sub classes defined how it is triggered
  */
public:
  Switch(const String &i);
  void addDevice(SwitchedDev &d) { switched.push_back(&d); }
  virtual void poll() {}
  virtual void poll(const unsigned long irc) {}
  void pressed();
  void addParm(const char *p) { parm = p; }

protected:
  String id;

private:
  String parm;
  std::vector<SwitchedDev *> switched;
};

typedef std::vector<std::unique_ptr<Switch>> SwitchList_t;

class PhysSwitch : public Switch
{
  /*
   * An input switch, physically SPST, on/off but the transition between
   * states is used to toggle the associated SwitchedDev object(s).
   * Any number of SwitchedDevs can be controlled by a single Switch.
   * 
   * Future enhancement option - add momentary switch capability.
   */
public:
  PhysSwitch(const String &i, const int pin);
  virtual void poll() override;

private:
  int spin;
  int switchState;
  int debounce;
  unsigned long changeAt; // When the switch changed state
};

class IRSwitch : public Switch
{
  /*
   * An infrared code.
   */
public:
  IRSwitch(const String &i, const String &ircode);
  virtual void poll(const unsigned long irc) override;

private:
  unsigned long code;
};

class Switches : public P2Task
{
  /*
   * RTOS task that routines the switches attached to the system and acts upon them
   */
public:
  Switches(SwitchList_t *list) : P2Task("switches", 2000) { swlist = list; }
  void irMessage(const unsigned long code);

private:
  SwitchList_t *swlist;
  bool operator()();
};