#pragma once

#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

class WiFiSerialClient : public WiFiClient
{
public:
    WiFiSerialClient() {}
    virtual ~WiFiSerialClient() {}
    WiFiSerialClient &operator=(const WiFiClient& c)
    {
        WiFiClient::operator=(c);
        return *this;
    }
    void begin(const String &a);
    void loop();

    virtual size_t write(uint8_t) override;
    virtual size_t write(const uint8_t *buf, size_t size) override;
};

extern WiFiSerialClient serr;
