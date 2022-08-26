#pragma once

#include <vector>

#include "p2task.h"

/**
 * @class SwitchedDev
 * @brief A device controlled by a Switch object
 */
class SwitchedDev
{
public:
  /**
   * @param i ID for reporting purposes
   * @todo Would prefer the String to be a const char*
   */
  SwitchedDev(const String &i) { id = i; }
  const String getid() { return id; }
  /**
   * @brief Actuate the switch to realise the requirements held in parm and extra
   *
   * @param parm What this switch does
   */

  virtual int doSwitch(const char *parm) = 0;

private:
  /** @brief An ID for reporting purposes */
  String id;
};

/**
 * @class Switch
 * @brief An actuator. Switch defines what it does, sub classes defined how it is triggered.
 *
 * Any number of SwitchedDevs can be controlled by a single Switch.
 *
 */
class Switch
{
public:
  /**
   * @param i ID for reporting purposes
   * @todo Would prefer the String to be a const char*
   */
  Switch(const String &i);
  /**
   * @brief Add a device to the control of this Switch
   * @param d The device to add
   */
  void addDevice(SwitchedDev &d) { switched.push_back(&d); }
  /**
   * @brief poll the switch for activity
   */
  virtual void poll(){};
  /**
   * @brief Actuate based on a received infrared code
   * @param irc Infrared code
   * @todo Well... this is a bit odd
   */
  virtual void poll(const unsigned long irc) {}
  /**
   * @brief Set the switch's parameter - ie what does it do
   * @param p The parameter
   */
  void addParm(const char *p) { parm = p; }

protected:
  /**
   * @brief Actuate (and synchronise state of all controlled devices)
   */
  void pressed();
  /** @brief name for reporting purposes */
  String id;

private:
  /** @brief parameter idicating to the controlled device what this switch does */
  String parm;
  /** @brief list of controlled devices */
  std::vector<SwitchedDev *> switched;
};

/**
 * @class PhysSwitch
 * @brief A physical switch
 *
 * An input switch, physically SPST, on/off but the transition between
 * states is used to toggle the associated SwitchedDev object(s).
 * Future enhancement option - add momentary switch capability.
 */
class PhysSwitch : public Switch
{
  /**
   * An input switch, physically latching SPST, on/off but the transition between
   * states is used to toggle the associated SwitchedDev object(s).
   *
   * @todo Add momentary switch capability.
   */
public:
  /**
   * @param i Identity for the switch for reporting purposes
   * @param pin GPIO where the switch is connected
   */
  PhysSwitch(const String &i, const int pin);
  virtual void poll() override;

private:
  /** @brief GPIO pin */
  int spin;
  /** @brief 0 = closed, 1 = open */
  int switchState;
  /** @brief
   * Are we waiting for a switch to settle (0 = no)
   * @todo This ought to be a bool
   */
  int debounce;
  /** @brief When the switch changed state */
  unsigned long changeAt;
};

/**
 * @class IRSwitch
 * @brief An infrared code.
 *
 */
class IRSwitch : public Switch
{
  /**
   * @param i Identity for the switch for reporting purposes
   * @param ircode The code as received from the IR remote control
   */
public:
  IRSwitch(const String &i, const String &ircode);
  /**
   * @param irc the code received
   *
   * @todo rename. This isn't really a poll - it gets called when a code is received
   */
  virtual void poll(const unsigned long irc) override;

private:
  /** @brief The IR code to match against incoming codes */
  unsigned long code;
};

/**
 * @class Switches
 * @brief The task that monitors the switch objects
 */
class Switches : public P2Task
{
public:
  Switches() : P2Task("switches", 3000) {}
  /** @brief Handle an incoming infra-red code */
  void irMessage(const unsigned long code);
  /**
   * @brief Add a switch to the list of controlled devices
   * @param sw Pointer to the Switch to be added
   */
  void addSwitch(Switch *sw) { swlist.push_back(sw); }

private:
  /** @brief List of switches controlled by this process */
  std::vector<Switch *> swlist;
  /** Task main loop - polls all controlled Switches */
  bool operator()();
};