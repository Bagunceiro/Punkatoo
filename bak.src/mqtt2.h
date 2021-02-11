#include <Arduino.h>
#include <vector>
#include <map>
#include <PubSubClient.h>

#pragma once

typedef String MQTTTopic;
class MQTTController;
class MQTTClientDev;

typedef std::vector<MQTTClientDev *> MQTTDevList;
typedef std::map<MQTTTopic, MQTTDevList> MQTTSubscriptionList;

class MQTTClientDev
{
public:
    MQTTClientDev(const String &devName);
    virtual ~MQTTClientDev();
    void registerMQTT(MQTTController &c);
    virtual void mqttMsgRecd(const String &topic, const String &msg) = 0;
    const String getName() { return name; }

private:
    MQTTController *ctlr;
    String topicPrefix;
    String name;
};

class MQTTController
{
public:
    MQTTController(PubSubClient& c);
    virtual ~MQTTController();
    bool init();

    void msgRecd(const String& fullTopic, const String& msg);
    static void rcvCallback(char* fullTopic, byte* payload, unsigned int length);

    bool subscribe(MQTTClientDev *, MQTTTopic &);
    void publish(String& topic, String& msg);
    void doSubscriptions();

private:
    MQTTSubscriptionList subList;
    static MQTTController* thectlr;
    PubSubClient* client;
};