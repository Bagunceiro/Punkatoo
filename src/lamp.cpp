#include <Arduino.h>
#include "config.h"
#include "lamp.h"
#include "rgbled.h"
#include "eventlog.h"

extern RGBLed indicator;

const int MAXDEBOUNCE = 5; // Number of loops to allow light switch to settle

Lamp::Lamp(String devName) : MQTTClientDev(devName), IRControlled(devName), PTask(devName, 2500) {}

Lamp::~Lamp() {}

void Lamp::sw(int toState)
{
  Event e;

  e.enqueue("Lamp to " + String(toState));
  if (toState == 0)
  {
    digitalWrite(lpin, HIGH);
    #ifdef TESTING
    indicator.off();
    #endif
  }
  else
  {
    digitalWrite(lpin, LOW);
    #ifdef TESTING
    indicator.setColour(RGBLed::WHITE);
    #endif
  }
  sendStatus();
}

void Lamp::toggle()
{
  int isOn = getStatus().toInt();
  sw(isOn == 0 ? 1 : 0);
}

bool Lamp::operator()()
{
  for (SwBlk& si : swList)
  {
    int newState = digitalRead(si.spin);
    if (newState != si.switchState)
    {
      si.debounce++;
      if (si.debounce > MAXDEBOUNCE)
      {
        Event e;

        String msg("Lamp sw ");
        msg += (newState == 1 ? "Open" : "Closed");
        e.enqueue(msg.c_str());
        toggle();
        si.switchState = newState;
        si.debounce = 0;
      }
    }
  }
  delay(10);
  return true;
}

int Lamp::switchstate()
{
  int result = 42;
  for (SwBlk& si : swList)
  {
    result = digitalRead(si.spin);
  }
  return result;
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
      if (i > 1) delay(length);
    }
}

void Lamp::init(const SwitchList inpList, int out)
{
  for (int inp : inpList)
  {
    SwBlk si;
    si.spin = inp;
    pinMode(si.spin, INPUT_PULLUP);
    delay(500); // input pin appears to need settling time after mode setting??
    si.switchState = digitalRead(si.spin);
    si.debounce = 0;
    swList.push_back(si);
  }
  lpin = out;
  pinMode(lpin, OUTPUT);
  // sw(0);
}

void Lamp::mqttMsgRecd(const String& topic, const String& msg)
{
  if (topic == MQTT_TPC_SWITCH)
  {
    sw(msg.toInt());
  }
}

void Lamp::subscribeToMQTT()
{
  pmqttctlr->subscribe(this,  MQTT_TPC_SWITCH);
  sendStatus();
}


void Lamp::irmsgRecd(IRMessage msg)
{
  if (msg == IR_LAMP_TOGGLE) toggle();
}

void Lamp::subscribeToIR()
{
  subscribe(IR_LAMP_ON);
  subscribe(IR_LAMP_OFF);
  subscribe(IR_LAMP_TOGGLE);
}
