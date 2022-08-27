#include "mqtt.h"
#include "config.h"
#include "eventlog.h"

extern StatusIndicator statusInd;

/** @brief Interval between attempts to reconnect MQTT (ms) */
const unsigned int MQTT_CONNECT_ATTEMPT_INT = (10 * 1000);

MQTTController::MQTTController()
{
    thectlr = this;
    client = new PubSubClient(wifiClient);
    connFlag = false;
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

    if ((config[mqtthost_n].length() == 0) || (config[mqttport_n].length()) == 0)
    {
    }
    else
    {
        if ((lastAttempt == 0) || ((now - lastAttempt) > MQTT_CONNECT_ATTEMPT_INT))
        {
            Serial.print("Connecting to MQTT ");
            Serial.print(config[mqtthost_n]);
            Serial.print(" port ");
            Serial.println(config[mqttport_n]);

            lastAttempt = now;

            String clientID = String("ctlr_") + String(config[controllername_n]) + String("_") + String(millis() % 1000);

            client->setServer(config[mqtthost_n].c_str(), config[mqttport_n].toInt());
            client->setCallback(rcvCallback);

            if (client->connect(clientID.c_str(),
                                config[mqttuser_n].c_str(),
                                config[mqttpwd_n].c_str()))
            {
                Serial.println("MQTT connected");
                poll();
                doSubscriptions();
                for (MQTTClientDev *dev : devList)
                {
                    dev->mqttSendStatus();
                    client->loop();
                }
                result = true;
            }
            else
            {
                Serial.print("MQTT Connect Failure: ");
                Serial.println(client->state());
            }
        }
    }
    return result;
}

MQTTController *MQTTController::thectlr = NULL;

void MQTTController::subscribe(MQTTClientDev *dev, const MQTTTopic &topic)
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
}

String MQTTController::stdPrefix()
{
    return config[mqttroot_n] + "/" + config[mqtttopic_n] + "/";
}

// void MQTTController::publish(const String &topic, const String &msg, const bool retained)
void MQTTController::publish(const char *topic, const char *msg, const bool retained)
{
    if (connected())
    {
        static SemaphoreHandle_t pubMutex = xSemaphoreCreateMutex();
        String t = stdPrefix() + topic;
        delay(5); // Needs further investigation. This delay or the debug output seems to stop
                  // things tripping over each other and causing loss of connection to the
                  // server. The debug line takes >100ms.
        // Serial.printf("publishing %s, %s, %d\n", t.c_str(), msg.c_str(), retained);
        xSemaphoreTake(pubMutex, portMAX_DELAY);
        client->publish(t.c_str(), msg, retained);
        xSemaphoreGive(pubMutex);
    }
}

void MQTTController::doSubscriptions()
{

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
            Event e;
            String txt = "MQTT " + topic + ":" + msg + " -> " + dev->mqttGetName();
            e.enqueue(txt.c_str());
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

MQTTClientDev::MQTTClientDev(const String &devname)
{
    name = devname;
    pmqttctlr = NULL;
}

MQTTClientDev::~MQTTClientDev()
{
    if (pmqttctlr != NULL)
    {
        pmqttctlr->rmClientDev(*this);
    }
}

void MQTTClientDev::mqttSendStatus()
{
    // Serial.printf("sendStatus for %s\n", name.c_str());
    String stat = mqttGetStatus();
    if (stat != "")
    {
        mqttPublish(MQTT_TPC_STAT, stat, true);
    }
}

void MQTTClientDev::mqttPublish(const String topic, const String message, bool retain)
{
    if (pmqttctlr != NULL)
    {
        // Serial.println("Publishing:");
        // Serial.println(name + "/" + topic);
        // Serial.println(message);
        pmqttctlr->publish((name + "/" + topic).c_str(), message.c_str(), retain);
    }
}

void MQTTClientDev::registerMQTT(MQTTController &c)
{
    pmqttctlr = &c;
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

bool MQTTController::poll()
{
    bool result = true;

    if (!client->loop())
    {
        if (connFlag)
        {
            Serial.println("Lost MQTT Connection");
        }
        if (init())
        {
            statusInd.enterState(StatusIndicator::STATE_MQTT);
        }
        else
        {
            statusInd.enterState(StatusIndicator::STATE_NETWORK);
            result = false;
        }
    }
    connFlag = result;
    return result;
}

/**
 * Are we connected to the MQTT broker, or at least were we when last we looked.
 * PubSubClient::connected does not appear to be thread safe so avoiding making the call here
 */
bool MQTTController::connected()
{
    return connFlag;
}

bool MQTTClientDev::mqttConnected()
{
    bool result = false;
    if (pmqttctlr != NULL)
    {
        result = pmqttctlr->connected();
    }
    return result;
}