#pragma once

#include <Arduino.h>

/**
 * @brief RGB LED used as a status indicator
 */
class IndicatorLed
{
public:
    /** @brief A colour as its RGB components
     * @todo Should be uint8_t?
     */
    struct Colour
    {
        uint16_t red;
        uint16_t blue;
        uint16_t green;
    };

/**
 * @param name A name for reporting purposes
 * @param r GPIO pin for the red channel
 * @param g GPIO pin for the green channel
 * @param b GPIO pin for the blue channel
 */
    IndicatorLed(const String &name, const uint8_t r, const uint8_t g, const uint8_t b);
    virtual ~IndicatorLed();
    const struct Colour getColour() const;
    /**
     * @brief set the colour
     * @param c The colour required
     * @param ovr Override the "suppress indications" configuration parameter
     */
    void setColour(const struct Colour &c, const bool ovr = false);
    /** @brief Turn it off */
    void off();
    /** @brief Turn it off after timeout if one has been configured */
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

    /** @brief GPIO pin for the red channel */
    uint8_t redPin;
    /** @brief GPIO pin for the green channel */
    uint8_t greenPin;
    /** @brief GPIO pin for the blue channel */
    uint8_t bluePin;

    uint8_t redChan;
    uint8_t greenChan;
    uint8_t blueChan;

    unsigned long setAt;
    unsigned long timeout;
};

/**
 * @brief Broad brush state of the application to enable indication
 * @todo This is a bit weird
 */ 
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