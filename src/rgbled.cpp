#include "rgbled.h"

const RGBLed::Colour RGBLed::RED     = {256,0,0}; 
const RGBLed::Colour RGBLed::YELLOW  = {256,256,0};
const RGBLed::Colour RGBLed::GREEN   = {0,256,0};
const RGBLed::Colour RGBLed::CYAN    = {0,256,256};
const RGBLed::Colour RGBLed::BLUE    = {0,0,256};
const RGBLed::Colour RGBLed::MAGENTA = {256,0,256};

const RGBLed::Colour RGBLed::WHITE   = {256,256,256};

RGBLed::RGBLed(const uint8_t r, const uint8_t g, const uint8_t b)
{
    redPin = r;
    greenPin = g;
    bluePin = b;

    redChan = 1;
    greenChan = 2;
    blueChan = 3;

    ledcAttachPin(redPin, 1);
    ledcAttachPin(greenPin, 2);
    ledcAttachPin(bluePin, 3);
    
    ledcSetup(redChan, 12000, 8); // 12 kHz PWM, 8-bit resolution
    ledcSetup(greenChan, 12000, 8);
    ledcSetup(blueChan, 12000, 8);

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

void RGBLed::setColour(const struct Colour& c)
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
}