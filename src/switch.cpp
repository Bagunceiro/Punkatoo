#include <Arduino.h>
#include "config.h"
#include "switch.h"

const int MAXDEBOUNCE = 5;
const int MIN_MILLIS = 10; // was 100

Switch::Switch(const String &i)
{
  id = i;
}

PhysSwitch::PhysSwitch(const String &i, const int pin) : Switch(i)
{
  spin = pin;
  pinMode(spin, INPUT_PULLUP);
  delay(50); // input needs settling time after mode setting to charge up capacitance?

  switchState = digitalRead(spin);
  debounce = 0;
  changeAt = 0;
}

IRSwitch::IRSwitch(const String &i, const String &c) : Switch(i)
{
  sscanf(c.c_str(), "%lx", &code);
}

void Switch::pressed()
{
  bool first = true;
  int newState = 0;
  for (SwitchedDev *d : switched)
  {
    if (first)
    {
      newState = d->doSwitch(parm.c_str());
      first = false;
    }
    else
    {
      // This one should switch it to "newState" so that all devices get synched
      d->doSwitch(parm.c_str(), true, newState);
    }
  }
}

void PhysSwitch::poll()
{
  int newState = digitalRead(spin);
  if (newState != switchState)
  {
    unsigned long now = millis();
    if (debounce == 0) // then this is the first sight of the change of state
    {
      changeAt = now; // so note when it was
    }
    debounce++;                        // and increment the count of sightings
    if ((now - changeAt) > MIN_MILLIS) // the state has been stable for MIN_MILLIS ms, switch it
    {
      Event e;

      char buffer[32];
      snprintf(buffer, sizeof(buffer) - 1, "Switch %s %s (%d)", id.c_str(), (newState == 1 ? "Open" : "Closed"), debounce);
      e.enqueue(buffer);
      pressed();
      switchState = newState;
      debounce = 0;
    }
  }
  else
  {
    debounce = 0; // to flag no change of state
  }
}

bool Switches::operator()()
{
  bool result = true;
  for (Switch *sw : swlist)
  {
    sw->poll();
  }

  return result;
}

void Switches::irMessage(const unsigned long code)
{
  for (auto sw : swlist)
  {
    sw->poll(code);
  }
}

void IRSwitch::poll(const unsigned long irc)
{
  if (irc == code)
  {
    pressed();
  }
}