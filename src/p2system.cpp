#include "p2system.h"
#include "config.h"

P2System::P2System() : MQTTClientDev("P2Sys"), IRClientDev("P2Sys")
{
    resetFlag = false;
    prevst = st = STATE_0;
}

P2System::~P2System()
{
}

void P2System::reset()
{
    resetFlag = true;
}

void P2System::routine()
{
    if (resetFlag)
    {
        serr.println("System going down");
        delay(1000);
        ESP.restart();
    }
}

void P2System::subscribeToMQTT()
{
    pmqttctlr->subscribe(this, MQTT_TPC_SYSTEM);
}

void P2System::mqttMsgRecd(const String &topic, const String &msg)
{
    if (topic == MQTT_TPC_SYSTEM)
    {
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, msg);
        if (error)
        {
            serr.printf("System MQTT Message error (%d)\n", error.code());
        }
        else
        {
            JsonObject root = doc.as<JsonObject>();
            for (JsonPair kv : root)
            {
                if (kv.key() == "command")
                {
                    const char *val = (const char *)kv.value();
                    if (strcmp(val, "stat") == 0)
                    {
                        mqttSendStatus();
                    }
                    else if (strcmp(val, "reset") == 0)
                    {
                        reset();
                    }
                }
            }
        }
    }
}

String P2System::mqttGetStatus()
{
    return ("{\"name\":\"" + config[controllername_n] + "\"}");
}

void P2System::enterState(enum State s)
{
    prevst = st;
    st = s;

    if (dev.indicators.size() > 0)
    {
        switch (st)
        {
        case STATE_0:
            dev.indicators[0].setColour(indicate_0);
            break;
        case STATE_AWAKE:
            dev.indicators[0].setColour(indicate_awake);
            break;
        case STATE_NETWORK:
            dev.indicators[0].setColour(indicate_network);
            break;
        case STATE_MQTT:
            dev.indicators[0].setColour(indicate_mqtt);
            break;
        case STATE_WPS:
            dev.indicators[0].setColour(indicate_wps);
            break;
        case STATE_CONFIGURATOR:
            dev.indicators[0].setColour(indicate_configurator);
            break;
        case STATE_UPDATE:
            dev.indicators[0].setColour(indicate_update);
            break;
        default:
            break;
        }
    }
}