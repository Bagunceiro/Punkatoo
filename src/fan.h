#pragma once

#include "mqtt.h"
#include "infrared.h"
#include "spdt.h"

class Fan: public MQTTClientDev, public IRControlled
{
    /*
       Speed is a number between -3 and +3 where 0 is off and sign = direction
    */
  public:
    Fan(String devName);
    virtual ~Fan();
    void init(const int d1, const int d2, const int s1, const int s2);
    void setSpeed(const int s);
    int8_t getSpeed();
    virtual String getStatus();
    /*
        Converts speed value to positions of the speed switch.
    */
    int speedToPos(const int s);
    int posToSpeed(const int p);
    void onoff();
    void faster();
    void slower();
    bool reverse();
    virtual void mqttMsgRecd(const String &topic, const String &msg);
    virtual void irmsgRecd(const IRMessage code);

  private:
    virtual void subscribeToMQTT();
    virtual void subscribeToIR();
    SPDT dir; // Switch to set direction of motor (or off)
    SPDT spd; // Switch to set speed of motor
};
