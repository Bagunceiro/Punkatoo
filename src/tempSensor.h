#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>

#include "config.h"
#include "mqtt.h"

class TempSensor : public Adafruit_BME280, public MqttControlled
{
  public:
  TempSensor();
  ~TempSensor();
  bool start(uint8_t addr, TwoWire *theWire);
  virtual String getStatus();
  void msgRecd(const String& topic, const String& msg);
  virtual void mqttaction(const String& topic, const String& msg);
  virtual void doSubscriptions(PubSubClient& mqttClient);
  const bool running() const { return ok; }
  private:
  bool ok;
};