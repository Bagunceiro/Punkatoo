#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>

#include "config.h"
#include "mqtt.h"

class BMESensor : public Adafruit_BME280, public MQTTClientDev
{
  public:
  BMESensor(const String& name);
  ~BMESensor();
  bool start(uint8_t addr, TwoWire *theWire);
  void msgRecd(const String& topic, const String& msg);
  virtual void mqttMsgRecd(const String& topic, const String& msg);
  // virtual void doSubscriptions();
  const bool running() const { return ok; }
  virtual String getStatus();
  private:
  bool ok;
};
