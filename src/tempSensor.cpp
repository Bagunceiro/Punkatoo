#include <ArduinoJson.h>
#include "tempSensor.h"

TempSensor::TempSensor() : MqttControlled("bme")
{
  ok = false;
}

TempSensor::~TempSensor()
{
}

void TempSensor::msgRecd(const String &topic, const String &msg)
{
}

void TempSensor::mqttaction(const String &topic, const String &msg)
{
}

void TempSensor::doSubscriptions(PubSubClient &mqttClient)
{
}

double setPrecision(double value, const unsigned int precision)
{
  int factor = pow(10, precision);
  return (round(value * factor) / factor);
}

String TempSensor::getStatus()
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

bool TempSensor::start(uint8_t addr, TwoWire *theWire)
{
  bool started = begin(addr, theWire);
  ok = started;
  return ok;
}