#include <ArduinoJson.h>
#include "bme.h"

BMESensor::BMESensor(const String& name) : MQTTClientDev(name)
{
  ok = false;
}

BMESensor::~BMESensor()
{
}

void BMESensor::msgRecd(const String &topic, const String &msg)
{
}

void BMESensor::mqttMsgRecd(const String &topic, const String &msg)
{
}

double setPrecision(double value, const unsigned int precision)
{
  int factor = pow(10, precision);
  return (round(value * factor) / factor);
}

String BMESensor::getStatus()
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

bool BMESensor::start(uint8_t addr, TwoWire *theWire)
{
  bool started = begin(addr, theWire);
  ok = started;
  return ok;
}