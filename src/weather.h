#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// #include <RTClib.h>

#include "mqtt.h"

class BME : public Adafruit_BME280
{
  /*
   * BME280 Temperature/Humidity/Pressure sensor
   */
  public:
  BME(int addr = 0x75);
  ~BME();

  operator bool() const { return _valid; }
  void setAddress(int addr);
  void setAddress(const String hexaddr);
  void setValid() { _valid = true; }
  bool start(TwoWire *theWire);
  private:
  bool _valid = false;
  int _addr;
};

class WeatherStation : public MQTTClientDev
{
  public:
  WeatherStation() : MQTTClientDev("weather") {} 
  void setInterval(const int i) { _interval = i; }
  void setValid() { _valid = true; }
  void enableBME(const int addr = 0)
  {
    if (addr) _bme.setAddress(addr);
    _bme.setValid();
  };
  void start(TwoWire *theWire);
  void poll();
  bool temperature(double& t);
  bool pressure(double& t);
  bool humidity(double& t);
  operator bool() const { return _valid; }

  private:
  BME _bme;
  bool _valid = false;

  unsigned long _interval = 0; // polling interval in milliseconds; 0 = disabled
  unsigned long _latestPoll = 0; // also in milliseconds
};
