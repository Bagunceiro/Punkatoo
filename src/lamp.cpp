#include <Arduino.h>
#include "config.h"
#include "lamp.h"
#include "indicator.h"
#include "eventlog.h"

Lamp::Lamp(const char *devName, const int relayPin) : MQTTClientDev(devName), SwitchedDev(devName)
{
  lpin = relayPin;
  pinMode(lpin, OUTPUT);
  sw(0);
}

Lamp::~Lamp() {}

void Lamp::sw(int toState)
{
  const int isOn = getStatus();
  if ((isOn && toState == 0) || (!isOn && (toState == 1)))
  {
    Event e;

    e.enqueue("Lamp " + String(mqttGetName()) + " " + (toState == 0 ? "off" : "on"));
    if (toState == 0)
    {
      digitalWrite(lpin, HIGH);
      for (LampAction act : callBacks)
      {
        act._cb(this, toState, act._data);
      }
    }
    else
    {
      digitalWrite(lpin, LOW);
      for (LampAction act : callBacks)
      {
        act._cb(this, toState, act._data);
      }
    }
    mqttSendStatus();
  }
}

int Lamp::toggle()
{
  // const int isOn = getStatus();
  const int newState = (getStatus() == 0 ? 1 : 0);
  // sw(isOn == 0 ? 1 : 0);
  sw(newState);
  return newState;
}

const int Lamp::getStatus() const
{
  int l = digitalRead(lpin);
  return (l == 0 ? 1 : 0);
}

String Lamp::mqttGetStatus()
{
  int l = digitalRead(lpin);
  return String((l == 0 ? 1 : 0));
}

const int Lamp::blip(const int t)
{
  toggle();
  delay(t);
  toggle();
  return (t);
}

void Lamp::blip(const int number, const int length)
{
  for (int i = number; i > 0; i--)
  {
    blip(length);
    if (i > 1)
      delay(length);
  }
}

void Lamp::mqttMsgRecd(const String &topic, const String &msg)
{
  if (topic.endsWith(MQTT_TPC_SWITCHTO))
  {
    sw(msg.toInt());
  }
  else if (topic.endsWith(MQTT_TPC_SWITCH))
  {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, msg);
    if (error)
    {
      serr.printf("Lamp cannot decode switch (%d)\n", error.code());
    }
    else
    {
      JsonObject root = doc.as<JsonObject>();
      for (JsonPair kv : root)
      {
        if (kv.key() == "tostate")
        {
          const char *val = (const char *)kv.value();
          serr.printf("lamp.switchto %s\n", val);
          if (strcmp(val, "on") == 0)
          {
            sw(1);
          }
          else if (strcmp(val, "off") == 0)
          {
            sw(0);
          }
          else if (strcmp(val, "toggle") == 0)
          {
            toggle();
          }
        }
      }
    }
  }
}

int Lamp::doSwitch(const char *parm, const bool more, const int extra)
{
  if (!more)
  {
    return toggle();
  }
  else
  {
    sw(extra);
    return extra;
  }
}

void Lamp::subscribeToMQTT()
{
  pmqttctlr->subscribe(this, mqttGetName() + MQTT_TPC_SWITCH);
  pmqttctlr->subscribe(this, mqttGetName() + MQTT_TPC_SWITCHTO);
  mqttSendStatus();
}
