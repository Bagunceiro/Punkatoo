#pragma once

#include <Arduino.h>

class IndicatorLed
{
    public:
    struct Colour
    {
        uint16_t red;
        uint16_t blue;
        uint16_t green;
    };

    IndicatorLed(const String& name, const uint8_t r, const uint8_t g, const uint8_t b);
    virtual ~IndicatorLed();
    const struct Colour getColour() const;
    void setColour(const struct Colour& c, const bool ovr = false);
    void off();
    void poll();

    static const Colour RED;
    static const Colour ORANGE;
    static const Colour YELLOW; 
    static const Colour GREEN;
    static const Colour CYAN;
    static const Colour BLUE;
    static const Colour MAGENTA;

    static const Colour WHITE;
    static const Colour BLACK;

    private:

    Colour colour;
    Colour blipColour;

    uint8_t redPin;
    uint8_t greenPin;
    uint8_t bluePin;

    uint8_t redChan;
    uint8_t greenChan;
    uint8_t blueChan;

    unsigned long setAt;
    unsigned long timeout;
};

class StatusIndicator
{
public:
    StatusIndicator();
    virtual ~StatusIndicator();

    enum State
    {
        STATE_0 = 0,
        STATE_AWAKE,
        STATE_NETWORK,
        STATE_MQTT,
        STATE_WPS,
        STATE_UPDATE
    };

    void enterState(enum State s);
    void revertState() { st = prevst; }

    private:
    bool resetFlag;
    State st;
    State prevst;
};