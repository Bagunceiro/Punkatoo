#include "config.h"
#include "fan.h"

Fan::Fan(String devName) : MqttControlled(devName) {}
Fan::~Fan() {}

void Fan::init(const int d1, const int d2, const int s1, const int s2)
{
  dir.setPins(d1, d2);
  spd.setPins(s1, s2);
  setSpeed(0);
}

void Fan::setSpeed(const int s)
{
  if (s >= -3 && s <= 3)
  {
    Serial.printf("Fan to %d\n", s);
    if (s == 0) // turn it off
    {
      dir.pos(0);
      spd.pos(0);
    }
    else
    {
      // Set the direction
      if (s > 0) dir.pos(1);
      else dir.pos(2);

      // Set the speed
      spd.pos(speedToPos(s));
    }

    sendStatus();
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
    if (dstat == 2) result = -result;
  }
  return result;
}

String Fan::getStatus()
{
  int result = 0; // Assume it's off

  int dstat = dir.stat();

  if (dstat != 0)
  {
    // Then it is on - work out the speed
    int sstat = spd.stat();

    result = posToSpeed(sstat);

    // And correct for direction
    if (dstat == 2) result = -result;
  }
  return String(result);
}

int Fan::speedToPos(const int s)
{
  switch (abs(s)) {
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    default: return 0;
  }
}

int Fan::posToSpeed(const int p)
{
  switch (p) {
    case 3: return 3;
    case 1: return 1;
    case 2: return 2;
    default: return 0;
  }
}

void Fan::onoff()
{
  int s = getSpeed();

  if (s == 0) setSpeed(1);
  else setSpeed(0);
}

void Fan::faster()
{
  if (dir.stat() != 0)
  {
    switch (getSpeed())
    {
      case 1: setSpeed(2); break;
      case 2: setSpeed(3); break;
      case -1: setSpeed(-2); break;
      case -2: setSpeed(-3); break;
    }
  }
}

void Fan::slower()
{
  if (dir.stat() != 0)
  {
    switch (getSpeed())
    {
      case 2: setSpeed(1); break;
      case 3: setSpeed(2); break;
      case -2: setSpeed(-1); break;
      case -3: setSpeed(-2); break;
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

void Fan::mqttaction(const String& topic, const String& msg)
{
  if (topic == MQTT_TPC_SPEED)
  {
    setSpeed(msg.toInt());
  }
}

void Fan::doSubscriptions(PubSubClient& mqttClient)
{
  mqttClient.subscribe((getPrefix() + MQTT_TPC_SPEED).c_str());
  sendStatus();
}


void Fan::irmsgRecd(uint32_t code)
{

  if (code == IRREMOTE_FAN_ONOFF) onoff();
  else if (code == IRREMOTE_FAN_REVERSE) reverse();
  else if (code == IRREMOTE_FAN_FASTER) faster();
  else if (code == IRREMOTE_FAN_SLOWER) slower();

}
