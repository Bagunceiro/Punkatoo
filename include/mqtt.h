#ifndef MQTTCONTROLLED
#define MQTTCONTROLLED
#include <Arduino.h>
#include <PubSubClient.h>

class MqttControlled
{
  public:
    MqttControlled(String devName);
    virtual ~MqttControlled();
    void setPrefix(String tpcpfx);
    void setPrefix();
    String getPrefix();
    virtual char* getStatus(char* buff) = 0;
    void sendStatus();
    void msgRecd(String topic, String msg);
    virtual void mqttaction(String topic, String msg) = 0;
    virtual void doSubscriptions(PubSubClient& mqttClient);

    static void msgScanDevices(String fullTopic, String msg);
    static void messageReceived(int messageSize);
    static void setAllPrefixes();
    static void doAllSubscriptions(PubSubClient& mqttClient);
  private:
    String topicPrefix;
    String name;
    MqttControlled* next;
    static MqttControlled* list;
};

void initMQTT();


#endif
