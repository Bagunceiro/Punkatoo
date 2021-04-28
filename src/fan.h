#pragma once

#include "mqtt.h"
#include "infrared.h"
#include "spdt.h"

class Fan: public MQTTClientDev, public IRClientDev
{
    /*

    */
  public:
    Fan(const char* devName, const int dirRly1, const int dirRly2, const int speedRly1, const int speedRly2);
    virtual ~Fan();
    /*
      initialise the fan with GPIO pins for the relays
    */
    // void init(const int dirRly1, const int dirRly2, const int speedRly1, const int speedRly2);
    /*
      Set speed of motor.
      Speed is a number between -3 and +3 where 0 is off and sign = direction
    */
    void setSpeed(const int speed);
    /*
      Get current speed of motor.
      Speed is a number between -3 and +3 where 0 is off and sign = direction
    */
    int8_t getSpeed();
    /*
      Return the current speed of the fan as a string for MQTT status publication
    */
    virtual String mqttGetStatus();
    /*
        Converts speed value to positions of the speed switch.
    */
    int speedToPos(const int s);
    /*
      Convert position of the speed switch to speed value
    */
    int posToSpeed(const int p);
    /*
      Toggle motor on or off
    */
    void onoff();
    /*
      increment motor speed
    */
    void faster();
    /*
      decrement motor speed
    */
    void slower();
    /*
      reverse motor direction
    */
    bool reverse();
    /*
      Deal with incoming MQTT message. Virtual, derived from MQTTClientDev
    */
    virtual void mqttMsgRecd(const String &topic, const String &msg);
    /*
      Deal with incoming IR message. Virtual, derived from IRClientDev
    */
    virtual void irmsgRecd(const IRMessage code);

  private:
    /*
      Subscribe topics of interest with MQTT Controller. Called by parent class during registration
    */
    virtual void subscribeToMQTT();
    /*
      Subscribe topics of interest with IR Controller. Called by parent class during registration
    */
    virtual void subscribeToIR();
    SPDT dir; // Switch to set direction of motor (or off)
    SPDT spd; // Switch to set speed of motor
};
