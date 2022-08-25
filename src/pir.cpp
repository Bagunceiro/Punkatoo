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

void PIR::trigger(const bool report)
{
    _PIRState = TRIGGERED;
    _lastTriggered = millis();
    if (report)
    {
        // Only log if there are lamps to be switched off
        for (Lamp *l : _controlledLamps)
        {
            if (l->getStatus() != 0)
            {
                Event ev;
                ev.enqueue("PIR Triggered");
                break;
            }
        }
    }
}

void PIR::routine()
{
    static unsigned long lastcalled = 0;
    unsigned long now = millis();
    if ((now - lastcalled) < 1000)
        return;

    lastcalled = now;

    State st = (digitalRead(_pin) == 0 ? UNDETECTED : TRIGGERED);

    if (st != _PIRState) // Change from last reading
    {
        _PIRState = st;

        if (st == TRIGGERED)
        {
            trigger();
        }
        else
        {
            _PIRState = UNDETECTED;
        }
    }

    if (_PIRState == TRIGGERED) // still triggered so refresh the timer.
    {
        trigger();
    }
    else
    {
        // state is quiescent - check if it's time to switch the lights off
        if ((_lastTriggered != 0) && (_timeout > 0)) // ie, is the timer running and is timeout enabled
        {
            unsigned long undetectedFor = now - _lastTriggered; // how long has it been quiescent?

            if (undetectedFor > (_timeout)) // long enough
            {
                // Event ev1;
                // String sm("PIR Reset");
                // ev1.enqueue(sm.c_str());

                _PIRState = UNDETECTED;
                for (Lamp *l : _controlledLamps)
                {
                    const int buffsize = 32;
                    char buffer[buffsize];
                    snprintf(buffer, buffsize-1, "PIR: lamp %s off", l->mqttGetName());
                    Event {buffer};

                    l->sw(0);
                }
                _lastTriggered = 0;
            }
        }
    }
}