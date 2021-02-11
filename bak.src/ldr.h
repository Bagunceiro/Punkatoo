#pragma once

#include "mqtt.h"

class LDR : public MqttControlled
{
public:
    LDR(String devName, uint8_t p);
    virtual ~LDR();
    virtual String getStatus();
    virtual void mqttaction(const String& topic, const String& msg);
    virtual void doSubscriptions(PubSubClient& mqttclient);
    uint16_t read();
private:
    uint8_t pin;
};