#pragma once

#include "devices.h"

class P2State
{
public:
    enum State
    {
        STATE_0 = 0,
        STATE_AWAKE,
        STATE_NETWORK,
        STATE_MQTT,
        STATE_WPS,
        STATE_CONFIGURATOR,
        STATE_UPDATE
    };

    P2State() { prevst = st = STATE_0; }

    void revert() { st = prevst; }
    void enter(enum State s)
    {
        prevst = st;
        st = s;

        switch (st)
        {
        case STATE_0:
            dev.indicators[0].setColour(indicate_0);
            break;
        case STATE_AWAKE:
            dev.indicators[0].setColour(indicate_awake);
            break;
        case STATE_NETWORK:
            dev.indicators[0].setColour(indicate_network);
            break;
        case STATE_MQTT:
            dev.indicators[0].setColour(indicate_mqtt);
            break;
        case STATE_WPS:
            dev.indicators[0].setColour(indicate_wps);
            break;
        case STATE_CONFIGURATOR:
            dev.indicators[0].setColour(indicate_configurator);
            break;
        case STATE_UPDATE:
            dev.indicators[0].setColour(indicate_update);
            break;
        default:
            break;
        }
    }

private:
    State st;
    State prevst;
};

extern P2State p2state;