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
    virtual void subscribeToMQTT() {}
    virtual void mqttMsgRecd(const String &topic, const String &msg) = 0;
    const String getName() { return name; }
    virtual String getStatus() = 0;
    virtual void sendStatus();

protected:
    MQTTController *mqttctlr;

private:
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

    bool subscribe(MQTTClientDev *, const MQTTTopic &);
    void publish(String& topic, String& msg, bool retained = true);
    void doSubscriptions();
    void addClientDev(MQTTClientDev& dev);

private:
    MQTTSubscriptionList subList;
    MQTTDevList devList;
    static MQTTController* thectlr;
    PubSubClient* client;
    String stdPrefix();
};