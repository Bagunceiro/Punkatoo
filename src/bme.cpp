#include <ArduinoJson.h>
#include "bme.h"
#include "config.h"

BME::BME(const char* name, int addr) : MQTTClientDev(name)
{
  ok = false;
  _id = name;
}

BME::~BME()
{
}

void BME::msgRecd(const String &topic, const String &msg)
{
}

void BME::mqttMsgRecd(const String &topic, const String &msg)
{
}

double setPrecision(double value, const unsigned int precision)
{
  int factor = pow(10, precision);
  return (round(value * factor) / factor);
}

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

bool BME::start(uint8_t addr, TwoWire *theWire)
{
  bool started = begin(addr, theWire);
  ok = started;
  return ok;
}

void BME::routine()
{
  static unsigned long then = 0;
  unsigned long now = millis();
  if ((then == 0) || ((now - then) > 15*60*1000))
  {
    String s = mqttGetStatus();
    mqttPublish("weather", s, true);
    then = now;
  }
}