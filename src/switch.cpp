#include <Arduino.h>
#include "config.h"
#include "switch.h"

const int MIN_MILLIS = 10;

Switch::Switch(const char* i)
{
  id = i;
}

PhysSwitch::PhysSwitch(const char* i, const int pin) : Switch(i)
{
  spin = pin;
  pinMode(spin, INPUT_PULLUP);
  delay(50); // input seems to needs settling time after mode setting - to charge up capacitance?

  switchState = digitalRead(spin);
  debounce = 0;
  changeAt = 0;
}

IRSwitch::IRSwitch(const char* i, const char* c) : Switch(i)
{
  sscanf(c, "%lx", &code);
}

void Switch::pressed()
{
  for (SwitchedDev *d : switched)
  {
    d->doSwitch(parm.c_str());
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