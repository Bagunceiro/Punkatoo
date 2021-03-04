#include <Arduino.h>
#include "config.h"
#include "switch.h"

const int MAXDEBOUNCE = 5;

Switch::Switch(const String &i, const int pin)
{
  id = i;
  spin = pin;
  pinMode(spin, INPUT_PULLUP);
  delay(500); // input pin appears to need settling time after mode setting??
  switchState = digitalRead(spin);
  debounce = 0;
}

bool Switch::poll()
{
  bool result = false;
  int newState = digitalRead(spin);
  if (newState != switchState)
  {
    debounce++;
    if (debounce > MAXDEBOUNCE)
    {
      Event e;

      String msg("Switch ");
      msg += (newState == 1 ? "Open" : "Closed");
      e.enqueue(msg.c_str());
      for (SwitchedDev *d : switched)
      {
        d->switchTo(newState);
      }
      switchState = newState;
      debounce = 0;
      result = true;
    }
  }
  return result;
}

bool Switches::operator()()
{
  bool result = true;

  for (Switch &sw : *swlist)
  {
    sw.poll();
  }

  return result;
}