#include "rgbled.h"

const RGBLed::Colour RGBLed::RED     = {256,  0,  0}; 
const RGBLed::Colour RGBLed::ORANGE  = {255,165,  0}; 
const RGBLed::Colour RGBLed::YELLOW  = {256,256,  0};
const RGBLed::Colour RGBLed::GREEN   = {  0,256,  0};
const RGBLed::Colour RGBLed::CYAN    = {  0,256,256};
const RGBLed::Colour RGBLed::BLUE    = {  0,  0,256};
const RGBLed::Colour RGBLed::MAGENTA = {256,  0,256};

const RGBLed::Colour RGBLed::WHITE   = {256,256,256};

RGBLed::RGBLed(const String& name, const uint8_t r, const uint8_t g, const uint8_t b)
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

    off();
}

RGBLed::~RGBLed()
{
    off();
}

const RGBLed::Colour RGBLed::getColour() const
{
    return colour;
}

void RGBLed::setColour(const struct Colour& c, const unsigned int timeout)
{
    colour = c;
    ledcWrite(redChan, 256 - colour.red);
    ledcWrite(greenChan, 256 - colour.green);
    ledcWrite(blueChan, 256 - colour.blue);
}

void RGBLed::off()
{
  ledcWrite(redChan, 256);
  ledcWrite(greenChan, 256);
  ledcWrite(blueChan, 256);

  setAt = 0;
  timeToLive = 0;
  fade = 0;
}

void RGBLed::poll()
{
    unsigned long now = millis();
    
    if (timeToLive)
    {
        unsigned long livedFor = now - setAt;
        if (livedFor >= timeToLive)
        {
            fade = now;
            timeToLive = 0;
            setAt = 0;
            fadeRed = redChan / 10;
            fadeGreen = greenChan / 10;
            fadeGreen = greenChan / 10;
        }
    }
    if (fade != 0)
    {
        if ((now - fade) > 500)
        {
            fade = now;
            int16_t newRed   = redChan - fadeRed;
            int16_t newGreen = redChan - fadeRed;
            int16_t newBlue  = redChan - fadeRed;
            if ((newRed < 0) || (newGreen < 0) || (newBlue < 0)) off();
            else
            {
                setColour({uint8_t(newRed), uint8_t(newGreen), uint8_t(newBlue)});
            }
        }
    }
}