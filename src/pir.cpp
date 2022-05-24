#include "pir.h"

PIR::PIR(const char *name, int pin)
{
    _id = name;
    _pin = pin;
    pinMode(_pin, INPUT);
}
PIR::~PIR()
{
}

void PIR::routine()
{
    static bool alreadySwitchedOff = false;
    State st = (digitalRead(_pin) == 0 ? UNDETECTED : DETECTED);
    if (st != _state)
    {
        _state = st;
        _lastChange = millis();
        alreadySwitchedOff = false;
    }
    else
    {
        if (_timeout > 0)
        {
            if (!alreadySwitchedOff)
            {
                if (_state == UNDETECTED)
                {
                    unsigned long undetectedFor = millis() - _lastChange;
                    if (undetectedFor > (_timeout))
                    {
                        for (Lamp *l : _controlledLamps)
                        {
                            l->sw(0);
                        }
                        alreadySwitchedOff = true;
                    }
                }
            }
        }
    }
}