#include <Arduino.h>
#include <vector>
#include <map>
#include <WiFi.h>
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
    virtual void mqttMsgRecd(const String &topic, const String &msg) {}
    const String getName() { return name; }
    virtual String getStatus() { return ""; }
    virtual void sendStatus();
    virtual void publish(const String topic, const String message, bool retain = false);

protected:
    MQTTController *pmqttctlr;

private:
    String topicPrefix;
    String name;
};

class MQTTController
{
public:
    // MQTTController(PubSubClient& c);
    MQTTController();
    virtual ~MQTTController();
    bool init();
    bool poll();

    void msgRecd(const String& fullTopic, const String& msg);
    static void rcvCallback(char* fullTopic, byte* payload, unsigned int length);

    bool subscribe(MQTTClientDev *, const MQTTTopic &);
    void publish(String& topic, String& msg, bool retained = false);
    void doSubscriptions();
    void addClientDev(MQTTClientDev& dev);
    void rmClientDev(MQTTClientDev& dev);

private:
    MQTTSubscriptionList subList;
    MQTTDevList devList;
    static MQTTController* thectlr;
    WiFiClient wifiClient;
    PubSubClient* client;
    String stdPrefix();
};