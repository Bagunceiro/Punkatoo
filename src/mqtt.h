#pragma once

#include <Arduino.h>
#include <vector>
#include <map>
#include <WiFi.h>
#include <PubSubClient.h>

typedef String MQTTTopic;
class MQTTController;

/*
  Parent class for devices using the MQTT services
*/
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
    bool connected();

protected:
    MQTTController *pmqttctlr;

private:
    String topicPrefix;
    String name;

};

/*
  This is a sort of local proxy for the MQTT broker. Client devices register with it 
  and it handles the external communications.
*/
class MQTTController
{
public:
    MQTTController();
    virtual ~MQTTController();
    bool init();
    bool poll(); // Called by the app's main loop - ToDo: make MQTTController its own task?

    void msgRecd(const String& fullTopic, const String& msg);
    static void rcvCallback(char* fullTopic, byte* payload, unsigned int length);

    bool subscribe(MQTTClientDev *, const MQTTTopic &);
    void publish(String& topic, String& msg, bool retained = false);
    void doSubscriptions();
    void addClientDev(MQTTClientDev& dev);
    void rmClientDev(MQTTClientDev& dev);
    bool connected();

private:
    typedef std::vector<MQTTClientDev *> MQTTDevList;
    typedef std::map<MQTTTopic, MQTTDevList> MQTTSubscriptionList;
    MQTTSubscriptionList subList;
    MQTTDevList devList;
    static MQTTController* thectlr;
    WiFiClient wifiClient;
    PubSubClient* client;
    String stdPrefix();
    bool connFlag; // set by poll to avoid using the thread unsafe PubSubClient::connected elsewhere
};