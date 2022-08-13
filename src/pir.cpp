#include "pir.h"
#include "eventlog.h"
#include "cli.h"

PIR::PIR(const char *name, int pin)
{
    _id = name;
    _pin = pin;
    pinMode(_pin, INPUT);
}
PIR::~PIR()
{
}

PIR &PIR::operator=(const PIR &rhs)
{
    _id = rhs._id;
    _pin = rhs._pin;
    _PIRState = rhs._PIRState;
    _timeout = rhs._timeout;
    _controlledLamps = rhs._controlledLamps;
    _lastTriggered = rhs._lastTriggered;

    return *this;
}

void PIR::trigger()
{
    _PIRState = TRIGGERED;
    _lastTriggered = millis();
}

void PIR::lampActivity(Lamp *l, const uint8_t state)
{
    // Event ev;
    // ev.enqueue((String) "PIR: Lamp activity (to " + state + ")");
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
    static unsigned long lastcalled = 0;
    unsigned long now = millis();
    if ((now - lastcalled) < 1000)
        return;

    lastcalled = now;

    State st = (digitalRead(_pin) == 0 ? UNDETECTED : TRIGGERED);
    // State st = UNDETECTED;

    if (st != _PIRState) // Change from last reading
    {
        Event ev;

        _PIRState = st;

        if (st == TRIGGERED)
        {
            trigger();
            ev.enqueue("PIR Triggered");
        }
        else
        {
            _PIRState = UNDETECTED;
            ev.enqueue("PIR Quiescent");
        }
    }


    if (_PIRState == TRIGGERED) // still triggered so refresh the timer.
    {
        trigger();
    }
    else
        // state is quiescent - check if it's time to switch the lights off
        if ((_lastTriggered != 0) && (_timeout > 0)) // ie, is the timer running and is timeout enabled
        {
            unsigned long undetectedFor = now - _lastTriggered; // how long has it been quiescent?

            if (undetectedFor > (_timeout)) // long enough
            {
                    Event ev1;
                    String sm("PIR Reset");
                    ev1.enqueue(sm.c_str());

                    _PIRState = UNDETECTED;
                    for (Lamp *l : _controlledLamps)
                    {
                        /*
                        Event e;
                        char buff[32];
                        sprintf(buff, "PIR lamp %lx off", l);
                        e.enqueue(buff);
                        */

                        l->sw(0);
                    }
                    _lastTriggered = 0;
            }
        }
}