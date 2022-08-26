/**
 * @file wifiserial.h
 * @brief Serial output extended over WiFi
 * @todo Unpick this? It doesn't get used and the implementation is a bit odd.
 */
#pragma once

#include <Arduino.h>
#include <WiFi.h>

/**
 * @brief Extends Serial output over the WiFi if a client is
 * connected (on port 1686)

 */
class WiFiSerialServer : public WiFiServer
{
public:
    /**
     * @brief Start up the server to await connection requests
     * @param a Announcement sent to the client on connect
     * @param port The port to listen on (defaults 1686)
     */
    void init(const String &a, const uint16_t port = 1686);
    /**@brief listen for connections */
    void handle();

private:
    /** @brief Announcement sent to client on connection */
    String announcement;
};

/**
 * @brief A client to receive Serial output extended over WiFi
 *        by WiFiSerialServer. Only one is permitted.
 *
 * Write functions are performed using the WiFiClient's inherited
 * stream functions so println, fprintf etc are available.
 *
 * @todo Allow additional connections?
 */
class WiFiSerialClient : public WiFiClient
{
public:
    WiFiSerialClient() {}
    virtual ~WiFiSerialClient() {}

    /** @todo Is this necessary? */
    WiFiSerialClient &operator=(const WiFiClient &c)
    {
        WiFiClient::operator=(c);
        return *this;
    }
    /**
     * @brief Start the server. Avoids other classes having to be
     * server aware.
     * @todo This is a little odd.
     */
    void begin(const String &a);
    void loop();

    /**
     * @brief Intercept the write functions to write to the local serial port too
     */
    virtual size_t write(uint8_t) override;
    /**
     * @brief Intercept the write functions to write to the local serial port too
     */
    virtual size_t write(const uint8_t *buf, size_t size) override;
};

/**
 * @brief Use this in place of "Serial"
 */
extern WiFiSerialClient serr;
