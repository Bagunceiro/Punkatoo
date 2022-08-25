#pragma once

#include <vector>

#include "mqtt.h"
// #include "infrared.h"
#include "switch.h"

/**
 * @class Lamp
 * @brief A simple on/off circuit (probably a lamp) switched by one of the relays
 */
class Lamp : public MQTTClientDev, public SwitchedDev
{
public:
  /**
   * @param devName Name of the device for reporting and MQTT purposes
   * @param relayPin The GPIO for the appropriate relay
   */
  Lamp(const char *devName, const int relayPin);
  virtual ~Lamp();

  /**
   * @return The status of the circuit - 0 = off
   */
  const int getStatus() const;
  /**
   * @brief Switch the circuit on or off
   * @param toState 0 to switch off, 1 to switch on
   */
  void sw(int toState);
  /**
   * @brief Switch the circuit (off->on or on->off)
   * @return the new state of the circuit (1 = on, 0 = off)
   */
  int toggle();

  /**
   * @brief Toggle the circuit for a defined period and then toggle it back
   * @param t Length of the blip in milliseconds
   *
   * @note This function blocks
   */
  const int blip(const int t = 500);
  /**
   * @brief blip the circuit a number of times
   * @param number Number of times to blip
   * @param length length of each blip in milliseconds
   */
  void blip(const int number, const int length);

  /**
   * @brief get the status of the circuit in a format suitable for MQTT publishing
   * @return the status as a string "1" for on, "0" for off
   */
  virtual String mqttGetStatus() override;
  /**
   * @brief Handle incoming MQTT message
   * @param topic The MQTT topic
   * @param msg The text of the message
   */
  virtual void mqttMsgRecd(const String &topic, const String &msg) override;

  /**
   * @brief Handle a signal from a switch.
   *
   * Toggles the Lamp unless there is an "extra" parameter. In that case extra
   * becomes the new state
   *
   * @param parm Not used by Lamp
   * @param more Is there an "extra" parameter (true == yes)
   * @param extra
   */
  virtual int doSwitch(const char *parm, const bool more, const int extra);

private:
  /** @brief GPIO for the controlling relay (active low) */
  int lpin;
  /** @brief Subscribe to appropriate MQTT topics (and publish the current status) */ 
  virtual void subscribeToMQTT() override;
};
