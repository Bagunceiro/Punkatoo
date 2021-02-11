#include <Arduino.h>
#include <PubSubClient.h>
#include "config.h"
#include "mqtt.h"

extern PubSubClient mqttClient;

MqttControlled* MqttControlled::list;

MqttControlled::MqttControlled(const String& devName) : name(devName) {
  serr.println((String)"Add " + devName);
  next = NULL;

  MqttControlled** ptr = &list;
  while (*ptr != NULL)
  {
    ptr = &((*ptr)->next);
  }
  *ptr = this;
}

void MqttControlled::msgScanDevices(const String& fullTopic, const String& msg)
{
  MqttControlled* ptr = list;
  while (ptr != NULL)
  {
    ptr->msgRecd(fullTopic, msg);
    ptr = ptr->next;
  }
}

// void MqttControlled::messageReceived(int messageSize) {
  
void messageReceived(char* fullTopic, byte* payload, unsigned int length)
{
  String msg;
  for (unsigned int i = 0; i < length; i++)
  {
    msg += (char) (payload[i]);
  }

  MqttControlled::msgScanDevices(fullTopic, msg);
}

void MqttControlled::setAllPrefixes() {
  MqttControlled* ptr = list;
  while (ptr != NULL)
  {
    ptr->setPrefix();
    ptr = ptr->next;
  }

}

void MqttControlled::doAllSubscriptions(PubSubClient& mqttClient)
{
  MqttControlled* ptr = list;
  while (ptr != NULL)
  {
    ptr->doSubscriptions(mqttClient);
    ptr = ptr->next;
  }
}

MqttControlled::~MqttControlled()
{
  MqttControlled**ptr = &list;
  while (*ptr != NULL)
  {
    if (*ptr == this)
    {
      (*ptr = next);
      break;
    }
    ptr = &((*ptr)->next);
  }
}


void MqttControlled::setPrefix(const String& tpcpfx)
{
  topicPrefix = tpcpfx;
}

void MqttControlled::doSubscriptions(PubSubClient& mqttClient)
{

}

void MqttControlled::setPrefix()
{
  setPrefix((String)persistant[persistant.mqttroot_n] + "/" + persistant[persistant.mqtttopic_n] + "/" + name + "/");
}

String MqttControlled::getPrefix()
{
  return topicPrefix;
}

void MqttControlled::sendStatus()
{
  if (mqttClient.connected())
  {
    char buffer[topicPrefix.length() + strlen(MQTT_TPC_STAT) + 1];
    sprintf(buffer, "%s%s", topicPrefix.c_str(), MQTT_TPC_STAT);
    String mbuff = getStatus();
    mqttClient.publish(buffer, mbuff.c_str(), true);
  }
}

void MqttControlled::msgRecd(const String& fullTopic, const String& msg)
{
  if (fullTopic.startsWith(topicPrefix))
  {
    String topic = fullTopic.substring(topicPrefix.length());
    mqttaction(topic, msg);
  }
}

bool initMQTT()
{
  static unsigned long lastAttempt = 0;
  unsigned long now = millis();
  bool result = false;

  if ((persistant[persistant.mqtthost_n].length() == 0) || (persistant[persistant.mqttport_n].length()) == 0)
   result = true;
  else
  {
  MqttControlled::setAllPrefixes();

  if ((lastAttempt == 0) || ((now - lastAttempt) > MQTT_CONNECT_ATTEMPT_PAUSE))
  {
    serr.print ("Connecting to MQTT ");
    serr.print(persistant[persistant.mqtthost_n]);
    serr.print(" port ");
    serr.println(persistant[persistant.mqttport_n]);

    lastAttempt = now;

    String clientID = String("ctlr_") + String(persistant[persistant.controllername_n]) + String("_") + String(millis() % 1000);

    mqttClient.setServer(persistant[persistant.mqtthost_n].c_str(), persistant[persistant.mqttport_n].toInt());
    mqttClient.setCallback(messageReceived);

    if (mqttClient.connect(clientID.c_str(), persistant[persistant.mqttuser_n].c_str(), persistant[persistant.mqttpwd_n].c_str())) {

      serr.println("MQTT connected");
      MqttControlled::doAllSubscriptions(mqttClient);
      result = true;
    }
    else
    {
      serr.print("Failed: ");
      serr.println(mqttClient.state());
    }
  }
  }
  return result;
}
