#pragma once

#include <vector>

/**
 * @brief A device controlled by a Switch object
 * 
 */
class SwitchedDev
{
public:
  SwitchedDev(const String &i) { id = i; }
  const String getid() { return id; }
  virtual int doSwitch(const char *parm, const bool more = false, const int extra = 0) = 0;

private:
  String id;
};

/**
 * @brief An actuator
 * This class defines what it does, sub classes defined how it is triggered
 * 
 */
class Switch
{
public:
  Switch(const String &i);
  void addDevice(SwitchedDev &d) { switched.push_back(&d); }
  virtual void poll() {};
  virtual void poll(const unsigned long irc) {}
  void pressed();
  void addParm(const char *p) { parm = p; }

protected:
  String id;

private:
  String parm;
  std::vector<SwitchedDev *> switched;
};



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

/**
 * @brief An infrared code.
 * 
 */
class IRSwitch : public Switch
{

public:
  IRSwitch(const String &i, const String &ircode);
  virtual void poll(const unsigned long irc) override;

private:
  unsigned long code;
};

/**
 * @brief The task that monitors the switch objects
 * 
 */
class Switches : public P2Task
{
public:
  Switches() : P2Task("switches", 3000) { }
  void irMessage(const unsigned long code);
  void addSwitch(Switch* sw) { swlist.push_back(sw); }

private:
  std::vector<Switch*> swlist;
  bool operator()();
};