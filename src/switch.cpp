#include <Arduino.h>
#include "config.h"
#include "switch.h"

const int MAXDEBOUNCE = 5;
const int MIN_MILLIS = 100;

Switch::Switch(const String &i, const int pin)
{
  id = i;
  spin = pin;
  pinMode(spin, INPUT_PULLUP);
  delay(500); // input pin appears to need settling time after mode setting??
  switchState = digitalRead(spin);
  debounce = 0;
  changeAt = 0;
}

bool Switch::poll()
{
  bool result = false;
  int newState = digitalRead(spin);
  if (newState != switchState)
  {
    unsigned long now = millis();
    if (debounce == 0) // then this is the first sight of the change of state
    {
      changeAt = now; // so note when it was
    }
    debounce++; // and increment the count of sightings
    if ((now - changeAt) > MIN_MILLIS) // the state has been stable for MIN_MILLIS ms, switch it
    // if (debounce > MAXDEBOUNCE)
    {
      Event e;

      char buffer [24];
      snprintf(buffer, sizeof(buffer) -1, "Switch %s (%d)", (newState == 1 ? "Open" : "Closed"), debounce);
      e.enqueue(buffer);
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