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

    struct Step
    {
        Colour colour;
        uint8_t repeat;
        uint16_t mark;
        uint16_t space;
    };

    IndicatorLed(const String& name, const uint8_t r, const uint8_t g, const uint8_t b);
    virtual ~IndicatorLed();
    const struct Colour getColour() const;
    void setColour(const struct Colour& c, const unsigned int timeout = 0);
    void off();

    static const Colour RED;
    static const Colour ORANGE;
    static const Colour YELLOW; 
    static const Colour GREEN;
    static const Colour CYAN;
    static const Colour BLUE;
    static const Colour MAGENTA;

    static const Colour WHITE;

    private:

    Colour colour;
    Colour blipColour;

    uint8_t redPin;
    uint8_t greenPin;
    uint8_t bluePin;

    uint8_t redChan;
    uint8_t greenChan;
    uint8_t blueChan;
};