#pragma once

#include <Arduino.h>
#include <PubSubClient.h>

class MqttControlled
{
  public:
    MqttControlled(const String& devName);
    virtual ~MqttControlled();
    void setPrefix(const String& tpcpfx);
    void setPrefix();
    String getPrefix();
    virtual String getStatus() = 0;
    void sendStatus();
    void msgRecd(const String& topic, const String& msg);
    virtual void mqttaction(const String& topic, const String& msg) = 0;
    virtual void doSubscriptions(PubSubClient& mqttClient);

    static void msgScanDevices(const String& fullTopic, const String& msg);
    static void messageReceived(const int messageSize);
    static void setAllPrefixes();
    static void doAllSubscriptions(PubSubClient& mqttClient);
  private:
    String topicPrefix;
    String name;
    MqttControlled* next;
    static MqttControlled* list;
};

bool initMQTT();
