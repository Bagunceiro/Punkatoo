#pragma once

#include "mqtt.h"
#include "spdt.h"
#include "switch.h"

/**
 * @brief Ceiling fan with three speed control (three wire capacitor)
 */
class Fan : public MQTTClientDev, public SwitchedDev
{
public:
  Fan(const char *devName, const int dirRly1, const int dirRly2, const int speedRly1, const int speedRly2);
  virtual ~Fan();
  /**
   * @brief Set speed of motor.
   * @param speed A number between -3 and +3 where 0 is off and sign = direction
   */
  void setSpeed(const int speed);
  /**
   * @brief Get current speed of motor.
   * @param speed A number between -3 and +3 where 0 is off and sign = direction
   */
  int8_t getSpeed();
  /**
   * @brief Get the current speed of the fan as a string for MQTT status publication
   * @return speed as a string - "-3" to "+3"
   */
  virtual String mqttGetStatus();
  /**
   * @brief Converts speed value to positions of the speed switch.
   * @param s The required speed
   * @return How the speed switch should be set
   */
  int speedToPos(const int s);
  /**
   * @brief Convert position of the speed switch to speed value
   * @param p The position of the speed switch
   * @return The speed (0-3)
   */
  int posToSpeed(const int p);
  /**
   * @brief Toggle motor on or off
   */
  void onoff();
  /** @brief Increment motor speed */
  void faster();
  /** @brief Decrement motor speed */
  void slower();
  /** @brief reverse motor direction */
  bool reverse();
  /**
   * @brief Deal with an incoming MQTT message.
   * @param topic The MQTT topic
   * @param msg The text of the MQTT message
   */
  virtual void mqttMsgRecd(const String &topic, const String &msg);
protected:
  /**
   * @brief Operate the switches
   * @param parm Expects one of "pwr" (switch on/off), "rev" (reverse), "+" (increase speed), "-" (decrease speed), 
   */
  virtual int doSwitch(const char *parm) override;

private:
  /**
   * @brief Subscribe topics of interest with MQTT Controller. Called by parent class during registration
   */
  virtual void subscribeToMQTT();
  /** @brief Switch to set direction of motor (or off) */
  SPDT dir;
  /** @brief Switch to set speed of motor */
  SPDT spd;
};
