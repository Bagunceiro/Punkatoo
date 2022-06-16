#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WiFiSerialServer : public WiFiServer
{
public:
    void init(const String &a, const uint16_t port = 1686);
    void handle();

private:
    String announcement;
};

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
