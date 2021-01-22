#pragma once

#include <Arduino.h>

class RGBLed
{
    public:
    struct Colour
    {
        uint16_t red;
        uint16_t blue;
        uint16_t green;
    };

    RGBLed(const uint8_t r, const uint8_t g, const uint8_t b);
    virtual ~RGBLed();
    const struct Colour getColour() const;
    void setColour(const struct Colour& c);
    void off();

    static const Colour RED; 
    static const Colour GREEN; 
    static const Colour BLUE;
    static const Colour WHITE;

    private:

    Colour colour;

    uint8_t redPin;
    uint8_t greenPin;
    uint8_t bluePin;

    uint8_t redChan;
    uint8_t greenChan;
    uint8_t blueChan;
};