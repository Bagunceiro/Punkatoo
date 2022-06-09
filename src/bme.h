#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// #include <RTClib.h>

#include "mqtt.h"

class BME : public Adafruit_BME280, public MQTTClientDev
{
  /*
   * BME280 Temperature/Humidity/Pressure sensor
   */
  public:
  BME(const char* name, int addr);
  ~BME();
  bool start(TwoWire *theWire);
  void msgRecd(const String& topic, const String& msg);
  virtual void mqttMsgRecd(const String& topic, const String& msg);
  const bool running() const { return ok; }
  virtual String mqttGetStatus();
  void routine();
  const String& getId() const { return _id; }
  private:
  String _id;
  bool ok;
  int _addr;
};

class WeatherStation
{
  public:
  void setBme(BME& bme) { _bme = &bme; }
  void setInterval(const int i) { _interval = i; }
  private:
  BME* _bme = NULL;
  int _interval = 15*60;
};
