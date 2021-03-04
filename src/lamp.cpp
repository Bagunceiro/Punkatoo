#include <Arduino.h>
#include "config.h"
#include "lamp.h"
#include "indicator.h"
#include "eventlog.h"

Lamp::Lamp(String devName, const int relayPin) : MQTTClientDev(devName), IRControlled(devName), SwitchedDev(devName)
{
  lpin = relayPin;
  pinMode(lpin, OUTPUT);
}

Lamp::~Lamp() {}

void Lamp::sw(int toState)
{
  Event e;

  e.enqueue(String("Lamp ") + (toState == 0 ? "off" : "on"));
  if (toState == 0)
  {
    digitalWrite(lpin, HIGH);
  }
  else
  {
    digitalWrite(lpin, LOW);
  }
  sendStatus();
}

void Lamp::toggle()
{
  int isOn = getStatus().toInt();
  sw(isOn == 0 ? 1 : 0);
}

String Lamp::getStatus()
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
  if (topic == MQTT_TPC_SWITCH)
  {
    sw(msg.toInt());
  }
}

void Lamp::subscribeToMQTT()
{
  pmqttctlr->subscribe(this, MQTT_TPC_SWITCH);
  sendStatus();
}

void Lamp::irmsgRecd(IRMessage msg)
{
  if (msg == IR_LAMP_TOGGLE)
    toggle();
}

void Lamp::subscribeToIR()
{
  subscribe(IR_LAMP_ON);
  subscribe(IR_LAMP_OFF);
  subscribe(IR_LAMP_TOGGLE);
}
