#include <WiFi.h>
#include <LITTLEFS.h>
#include "config.h"
#include "configurator.h"
#include "lamp.h"
#include "fan.h"
#include "indicator.h"

extern Fan fan;
extern Lamp lamp;
extern IndicatorLed indicator;

extern Configurator configurator;

Configurator::Configurator(const String& name) : IRControlled(name)
{
    startedAt = 0;
    running = false;
    startRequest = false;
    startCodeState = 0;
    stateChangedAt = 0;
}

Configurator::~Configurator()
{
}

void Configurator::irmsgRecd(uint32_t code)
{
    /*
    Configurator entry uses the fan speed up keys on the remote. but ony when the fan is off.
    The button must be pressed numberOfPresses times with a delay of no more than keyPressDelay
    */

    const int numberOfPresses = 5;
    const unsigned int keyPressDelay = 2000;

    unsigned long now = millis();

    if (running)
    {
        if (code == IRREMOTE_CONFIGURATOR_STOP)
        {

            stop();
        }
    }
    else if ((stateChangedAt == 0) || ((now - stateChangedAt) < keyPressDelay))
    {
        stateChangedAt = now;

        if (fan.getSpeed() == 0)
        {
            if (code == IRREMOTE_CONFIGURATOR_START)
            {
                startCodeState++;
                serr.printf("Configurator state = %d\n", startCodeState);
                indicator.setColour(startCodeState % 2 ? IndicatorLed::GREEN : IndicatorLed::RED);
                if (startCodeState >= numberOfPresses)
                {
                    //indicator.setColour(IndicatorLed::BLUE);
                    // running = true; // Here as well to avoid interupt bypassing it
                    stateChangedAt = 0;
                    startRequest = true; // done like this to avoid too much happening in the interrupt
                }
            }
        }
    }
    else
    {
        startCodeState = 0;
        stateChangedAt = 0;
    }
}

void Configurator::start()
{
    if (!running)
    {
        indicator.setColour(IndicatorLed::CYAN);

        String m = WiFi.macAddress();
        String ssid = persistant[persistant.controllername_n] + "_" + m.substring(9, 11) + m.substring(12, 14) + m.substring(15);
        const char *password = "configure";

        WiFi.softAP(ssid.c_str(), password); // Start the access point
        serr.printf("Access Point %s/%s started\n", ssid.c_str(), password);

        serr.printf("IP address: %s\n", WiFi.softAPIP().toString().c_str());

        // lamp.blip(10,250);
        startedAt = millis();
        running = true;
    }
}

void Configurator::stop()
{
    WiFi.softAPdisconnect(true);
    running = false;
    startCodeState = 0;
    stateChangedAt = 0;
    indicator.off();
    serr.println("SoftAP stopped");
}

void Configurator::poll()
{
    if (startRequest)
    {
        startRequest = false;
        start();
    }
    if (running)
    {
        unsigned long now = millis();
        if ((now - startedAt) > (15 * 60 * 1000))
        {
            serr.println("Configurator timeout");
            stop();
        }
    }
}
