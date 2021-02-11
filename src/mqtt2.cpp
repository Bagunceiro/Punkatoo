#include "mqtt2.h"
#include "config.h"

MQTTController::MQTTController(PubSubClient& c)
{
    thectlr = this;
    client = &c;
}

MQTTController::~MQTTController()
{
    thectlr = NULL;
}

bool MQTTController::init()
{
    static unsigned long lastAttempt = 0;
    unsigned long now = millis();
    bool result = false;

    if ((persistant[persistant.mqtthost_n].length() == 0) || (persistant[persistant.mqttport_n].length()) == 0)
        result = true;
    else
    {
        if ((lastAttempt == 0) || ((now - lastAttempt) > MQTT_CONNECT_ATTEMPT_PAUSE))
        {
            serr.print("Connecting to MQTT ");
            serr.print(persistant[persistant.mqtthost_n]);
            serr.print(" port ");
            serr.println(persistant[persistant.mqttport_n]);

            lastAttempt = now;

            String clientID = String("ctlr_") + String(persistant[persistant.controllername_n]) + String("_") + String(millis() % 1000);

            client->setServer(persistant[persistant.mqtthost_n].c_str(), persistant[persistant.mqttport_n].toInt());
            client->setCallback(rcvCallback);

            if (client->connect(clientID.c_str(),
                                   persistant[persistant.mqttuser_n].c_str(),
                                   persistant[persistant.mqttpwd_n].c_str()))
            {
                serr.println("MQTT connected");
                doSubscriptions();
                for (MQTTClientDev *dev : devList)
                {
                    dev->sendStatus();
                }
                result = true;
            }
            else
            {
                serr.print("Failed: ");
                serr.println(client->state());
            }
        }
    }
    return result;
}

MQTTController *MQTTController::thectlr = NULL;

bool MQTTController::subscribe(MQTTClientDev *dev, const MQTTTopic &topic)
{
    auto record = subList.find(topic);
    if (record == subList.end())
    {
        MQTTDevList newVector;
        newVector.push_back(dev);
        subList.insert({topic, newVector});
    }
    else
    {
        record->second.push_back(dev);
    }
    return true;
}

String MQTTController::stdPrefix()
{
    return
        persistant[persistant.mqttroot_n] + "/" 
        + persistant[persistant.mqtttopic_n] + "/";
}

void MQTTController::publish(String &topic, String &msg, bool retained)
{
    if (client->connected())
    {
        String t = stdPrefix() + topic;
        client->publish(t.c_str(), msg.c_str(), retained);
    }
}


void MQTTController::doSubscriptions()
{
    // Iterate the subscription list and subscribe to all topics
    MQTTSubscriptionList::iterator it;

    for (it = subList.begin(); it != subList.end(); it++)
    {
        String t = stdPrefix() + it->first;
        client->subscribe(t.c_str());
    }
}

void MQTTController::msgRecd(const String &fulltopic, const String &msg)
{
    // remove the standard prefix (if it's there)
    String pfx = stdPrefix();
    String topic = fulltopic;

    if (fulltopic.startsWith(pfx))
    {
        topic = String(topic.substring(pfx.length()));
    }

    // Search the subscription list
    auto record = subList.find(topic);
    if (record != subList.end())
    // Send to subscribing devices
    {
        // For each subscription for this message
        for (MQTTClientDev *dev : record->second)
        {
            char buffer[20];
            snprintf(buffer, 20, "MQTT(%s/%s)->%s", topic.c_str(), msg.c_str(), dev->getName().c_str());
            evLog.writeEvent(buffer);
            dev->mqttMsgRecd(topic, msg);
        }
    }
}

void MQTTController::rcvCallback(char *fullTopic, byte *payload, unsigned int length)
{
    String msg;
    for (unsigned int i = 0; i < length; i++)
    {
        msg += (char)(payload[i]);
    }

    thectlr->msgRecd(fullTopic, msg);
}

MQTTClientDev::MQTTClientDev(const String& devname)
{
    name = devname;
    mqttctlr = NULL;
}

MQTTClientDev::~MQTTClientDev()
{
    if (mqttctlr != NULL)
    {
        mqttctlr->rmClientDev(*this);
    }
}

void MQTTClientDev::sendStatus()
{
    String topic = name + "/" + MQTT_TPC_STAT;
    String stat  = getStatus();
    mqttctlr->publish(topic, stat);
}

void MQTTClientDev::registerMQTT(MQTTController &c)
{
    mqttctlr = &c;
    c.addClientDev(*this);
    subscribeToMQTT();
}

void MQTTController::addClientDev(MQTTClientDev &dev)
{
    devList.push_back(&dev);
}

void MQTTController::rmClientDev(MQTTClientDev &dev)
{
    std::remove(devList.begin(), devList.end(), &dev);
}