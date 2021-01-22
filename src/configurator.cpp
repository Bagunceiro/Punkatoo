#ifdef ESP32
#include <WiFi.h>
#include <LITTLEFS.h>
#else
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#endif
#include "config.h"
#include "configurator.h"
#include "lamp.h"
#include "fan.h"

extern Fan fan;
extern Lamp lamp;
Configurator configurator;

Configurator::Configurator()
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
            stop();
    }
    else if ((stateChangedAt == 0) || ((now - stateChangedAt) < keyPressDelay))
    {
        stateChangedAt = now;

        if (fan.getSpeed() == 0)
        {
            if (code == IRREMOTE_CONFIGURATOR_START)
            {
                startCodeState++;
                Serial.printf("Configurator state = %d\n", startCodeState);
                if (startCodeState >= numberOfPresses)
                {
                    // state = 0;
                    running = true; // Here as well to avoid interupt bypassing it
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
        String m = WiFi.macAddress();
        String ssid = persistant[persistant.controllername_n] + "_" + m.substring(9, 11) + m.substring(12, 14) + m.substring(15);
        const char *password = "configure";

        WiFi.softAP(ssid.c_str(), password); // Start the access point
        Serial.printf("Access Point %s/%s started\n", ssid.c_str(), password);

        Serial.printf("IP address: %s\n", WiFi.softAPIP().toString().c_str());

        lamp.blip(10,250);
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
    Serial.println("SoftAP stopped");
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
            Serial.println("Configurator timeout");
            stop();
        }
    }
}
