#include <ArduinoJson.h>
#include "weather.h"

#include "config.h"

BME::BME(int addr)
{
  _addr = addr;
}

BME::~BME()
{
}

void BME::setAddress(int addr)
{
  _addr = addr;
}

double setPrecision(double value, const unsigned int precision)
{
  int factor = pow(10, precision);
  return (round(value * factor) / factor);
}

/*
String BME::mqttGetStatus()
{
  time_t now = timeClient.getEpochTime();
  char tbuf[sizeof "YYYY-MM-DDTHH:MM:SS+ZZZZ"];
  strftime(tbuf, sizeof tbuf, "%FT%T", localtime(&now));

  StaticJsonDocument<512> doc;
  doc["timestamp"] = tbuf;
  if (ok)
  {
    doc["temperature"] = setPrecision(readTemperature(), 2);
    doc["pressure"]    = setPrecision(readPressure() / 100, 2);
    doc["humidity"]    = setPrecision(readHumidity(), 2);
  }
  String message;
  serializeJson(doc, message);
  return message;
}
*/

bool BME::start(TwoWire *theWire)
{
  _valid = begin(_addr, theWire);
  Serial.printf("BME::Start(%d): %d\n", _addr, _valid);
  return _valid;
}

void WeatherStation::start(TwoWire *theWire)
{
  _bme.start(theWire);
}

void WeatherStation::poll()
{
  if (_interval != 0)
  {
    unsigned long now = millis();
    if ((_latestPoll == 0) || ((now - _latestPoll) > (_interval)))
    {
      _latestPoll = now;
      String s;
      if (_bme)
      {
        double t;
        double h;
        double p;
        temperature(t);
        humidity(h);
        pressure(p);
        time_t now = timeClient.getEpochTime();
        char tbuf[sizeof "YYYY-MM-DDTHH:MM:SS+ZZZZ"];
        strftime(tbuf, sizeof tbuf, "%FT%T", localtime(&now));

        StaticJsonDocument<512> doc;
        doc["timestamp"] = tbuf;
        doc["temperature"] = t;
        doc["pressure"]    = p;
        doc["humidity"]    = h;
        String message;
        serializeJson(doc, message);
        mqttPublish("report", message.c_str(), true);
      }
    }
  }
}

bool WeatherStation::temperature(double &t)
{
  bool result = false;
  t = 0;

  if (_bme)
  {
    t = setPrecision(round(_bme.readTemperature() * 10) / 10,2);
    result = true;
  }
  return result;
}

bool WeatherStation::pressure(double &p)
{
  bool result = false;
  p = 0;

  if (_bme)
  {
    p = setPrecision(round(_bme.readPressure() * 10)/1000,2);
    result = true;
  }
  return result;
}

bool WeatherStation::humidity(double &h)
{
  bool result = false;
  h = 0;

  if (_bme)
  {
    h = setPrecision(round(_bme.readHumidity() * 10) / 10, 2);
    result = true;
  }
  return result;
}