#include "indicator.h"
#include "config.h"

const IndicatorLed::Colour IndicatorLed::RED     = {256,  0,  0}; 
const IndicatorLed::Colour IndicatorLed::ORANGE  = {255,165,  0}; 
const IndicatorLed::Colour IndicatorLed::YELLOW  = {256,256,  0};
const IndicatorLed::Colour IndicatorLed::GREEN   = {  0,256,  0};
const IndicatorLed::Colour IndicatorLed::CYAN    = {  0,256,256};
const IndicatorLed::Colour IndicatorLed::BLUE    = {  0,  0,256};
const IndicatorLed::Colour IndicatorLed::MAGENTA = {256,  0,256};

const IndicatorLed::Colour IndicatorLed::WHITE   = {256,256,256};
const IndicatorLed::Colour IndicatorLed::BLACK   = {  0,  0,  0};

IndicatorLed::IndicatorLed(const String& name, const uint8_t r, const uint8_t g, const uint8_t b)
{
    redPin   = r;
    greenPin = g;
    bluePin  = b;

    redChan   = 1;
    greenChan = 2;
    blueChan  = 3;

    ledcAttachPin(redPin,   1);
    ledcAttachPin(greenPin, 2);
    ledcAttachPin(bluePin,  3);
    
    ledcSetup(redChan,   12000, 8); // 12 kHz PWM, 8-bit resolution
    ledcSetup(greenChan, 12000, 8);
    ledcSetup(blueChan,  12000, 8);

    override = false;
    off();
}

IndicatorLed::~IndicatorLed()
{
    off();
}

const IndicatorLed::Colour IndicatorLed::getColour() const
{
    return colour;
}

void IndicatorLed::setColour(const struct Colour& c, const bool ovr)
{
    override = ovr;
    if ((persistant[persistant.indicator_n] == "1") || override)
    {
        colour = c;
        ledcWrite(redChan, 256 - colour.red);
        ledcWrite(greenChan, 256 - colour.green);
        ledcWrite(blueChan, 256 - colour.blue);
    }
}

void IndicatorLed::off()
{
  ledcWrite(redChan, 256);
  ledcWrite(greenChan, 256);
  ledcWrite(blueChan, 256);
}