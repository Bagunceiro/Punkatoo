#include "config.h"
#include "fan.h"
#include "eventlog.h"

Fan::Fan(const char* devName, const int d1, const int d2, const int s1, const int s2) : MQTTClientDev(devName), SwitchedDev(devName)
{
  dir.setPins(d1, d2);
  spd.setPins(s1, s2);
  setSpeed(0);
}
Fan::~Fan() {}

void Fan::setSpeed(const int s)
{
  if (s >= -3 && s <= 3)
  {
    dev.indicators[0].setColour(s > 0 ? IndicatorLed::GREEN : (s < 0 ? IndicatorLed::BLUE : IndicatorLed::RED), true);
    Event e;
    String txt = "Fan to " + String(s);
    e.enqueue(txt.c_str());
    // serr.printf("Fan to %d\n", s);
    if (s == 0) // turn it off
    {
      dir.pos(0);
      spd.pos(0);
    }
    else
    {
      // Set the direction
      if (s > 0)
        dir.pos(1);
      else
        dir.pos(2);

      // Set the speed
      spd.pos(speedToPos(s));
    }

    mqttSendStatus();
    delay(500);
    dev.indicators[0].off();
  }
}

int8_t Fan::getSpeed()
{
  int8_t result = 0; // Assume it's off

  int dstat = dir.stat();

  if (dstat != 0)
  {
    // Then it is on - work out the speed
    int sstat = spd.stat();

    result = posToSpeed(sstat);

    // And correct for direction
    if (dstat == 2)
      result = -result;
  }
  return result;
}

String Fan::mqttGetStatus()
{
  return String(getSpeed());
}

int Fan::speedToPos(const int s)
{
  switch (abs(s))
  {
  case 1:
    return 1;
  case 2:
    return 2;
  case 3:
    return 3;
  default:
    return 0;
  }
}

int Fan::posToSpeed(const int p)
{
  switch (p)
  {
  case 1:
    return 1;
  case 2:
    return 2;
  case 3:
    return 3;
  default:
    return 0;
  }
}

void Fan::onoff()
{
  int s = getSpeed();

  if (s == 0)
    setSpeed(1);
  else
    setSpeed(0);
}

void Fan::faster()
{
  // if (dir.stat() != 0)
  // {
    switch (getSpeed())
    {
    case 0:
      setSpeed(1);
      break;
    case 1:
      setSpeed(2);
      break;
    case 2:
      setSpeed(3);
      break;
    case -1:
      setSpeed(-2);
      break;
    case -2:
      setSpeed(-3);
      break;
    }
  // }
}

void Fan::slower()
{
  if (dir.stat() != 0)
  {
    switch (getSpeed())
    {
    case 2:
      setSpeed(1);
      break;
    case 3:
      setSpeed(2);
      break;
    case -2:
      setSpeed(-1);
      break;
    case -3:
      setSpeed(-2);
      break;
    case 1:
    case -1:
      setSpeed(0);
      break;
    }
  }
}

bool Fan::reverse()
{
  if (dir.stat() != 0)
  {
    setSpeed(getSpeed() * -1);
    return true;
  }
  else
  {
    return false;
  }
}

void Fan::mqttMsgRecd(const String &topic, const String &msg)
{
  if (topic == MQTT_TPC_SPEED)
  {
    if (msg == "+")
    {
      faster();
    }
    else if (msg == "-")
    {
      slower();
    }
    else setSpeed(msg.toInt());
  }
}

void Fan::subscribeToMQTT()
{
  pmqttctlr->subscribe(this, MQTT_TPC_SPEED);
  mqttSendStatus();
}

int Fan::doSwitch(const char* parm, const bool more, const int extra)
{
  if (strcmp(parm, "pwr") == 0)
    onoff();
  else if (strcmp(parm,"rev") == 0)
    reverse();
  else if (strcmp(parm,"+") == 0)
    faster();
  else if (strcmp(parm,"-") == 0)
    slower();
  return getSpeed();
}

/*
void Fan::subscribeToIR()
{
  subscribe(IR_FAN_TOGGLE);
  subscribe(IR_FAN_REVERSE);
  subscribe(IR_FAN_FASTER);
  subscribe(IR_FAN_SLOWER);
}
*/
