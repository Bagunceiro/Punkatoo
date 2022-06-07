#include "pir.h"
#include "eventlog.h"

PIR::PIR(const char *name, int pin)
{
    _id = name;
    _pin = pin;
    pinMode(_pin, INPUT);
}
PIR::~PIR()
{
}

void PIR::lampActivity(Lamp *l, const uint8_t state)
{
    // treat light being switched on as someone being detected (even if done remotely)
    if (state != 0)
        trigger();
}

void PIR::lampActivity(Lamp *l, const uint8_t state, void *pir)
{
    ((PIR *)pir)->lampActivity(l, state);
}

void PIR::routine()
{
    State st = (digitalRead(_pin) == 0 ? UNDETECTED : TRIGGERED);
    if (st != _signalState)
    {
        _signalState = st;

/*
        Event ev1;
        String sm("PIR ");
        sm += (st == TRIGGERED ? "1" : "0");
        ev1.enqueue(sm.c_str());
*/

        if (st == TRIGGERED)
        {
            unsigned long now = millis();
            if ((lastDetected != 0) && (now - lastDetected) < (3 * 60 * 1000)) // twice noticed in the last 3 mins
            {
                if (_PIRState != TRIGGERED)
                {
                    _PIRState = TRIGGERED;

                    Event ev1;
                    String sm("PIR triggered");
                    ev1.enqueue(sm.c_str());
                }
                trigger();
            }
            lastDetected = now;
        }
    }
    else // No change in PIR state
    {
        if (st == TRIGGERED)
        {
            lastDetected = millis();
        }
        else if (_timeout > 0)
        {
            unsigned long undetectedFor = millis() - _lastChange;
            if (undetectedFor > (_timeout))
            {
                if (_PIRState == TRIGGERED)
                {
                    Event ev1;
                    String sm("PIR reset");
                    ev1.enqueue(sm.c_str());

                    _PIRState = UNDETECTED;
                    for (Lamp *l : _controlledLamps)
                    {
                        l->sw(0);
                    }
                }
            }
        }
    }
}