#include <ArduinoJson.h>
#include "tempSensor.h"

TempSensor::TempSensor() : MqttControlled("bme")
{
//    setPrefix("bme");
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
  return(round(value * factor)/factor);
}

String TempSensor::getStatus()
{
  // Serial.printf("Sending %s\n", temp.c_str());
  //   mqttClient.publish("/home/temp", temp.c_str(), true);

  time_t now = timeClient.getEpochTime();
  StaticJsonDocument<512> doc;
  doc["timestamp"]   = ctime(&now);
  doc["temperature"] = setPrecision(readTemperature(), 2);
  doc["pressure"]    = setPrecision(readPressure()/100, 2);
  doc["humidity"]    = setPrecision(readHumidity(), 2);
  Serial.println("");
  String message;
  serializeJson(doc, message);
  return message;
  // Serial.println(message);
  // mqttClient.publish("home/weather", message.c_str(), message.length());
}