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
        // Serial.printf("%d PIR cos -> %d\n", _lastChange, st);
        alreadySwitchedOff = false;
    }
    else
    {
        if (!alreadySwitchedOff)
        {
            if (_state == UNDETECTED)
            {
                unsigned long undetectedFor = millis() - _lastChange;
                if (undetectedFor > (30 * 60 * 1000))
                {
                    // 30 minutes - this will become a config entry
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